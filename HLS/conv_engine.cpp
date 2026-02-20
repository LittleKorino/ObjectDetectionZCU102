/**
 * conv_engine_v3.cpp — "Verilog-in-C++" Cache & DMA Architecture
 *
 * ARCHITECTURE OVERVIEW
 * =====================
 * Three-stage DATAFLOW pipeline: Fetch → Execute → Write
 *
 * "Verilog-like" means:
 *  - Every DRAM access goes through an explicit DMA staging buffer
 *    (like an AXI read/write channel FIFO in RTL).
 *  - All packing/unpacking uses .range() bit-slicing
 *    (like Verilog's [hi:lo] wire selection).
 *  - Memory operations are phase-separated: READ → MODIFY → WRITE
 *    (like separate states in a Verilog FSM).
 *  - BRAM arrays are explicitly banked and bound
 *    (like instantiating separate BRAM primitives).
 *  - Address computation uses shifts/masks instead of div/mod
 *    (like address decoders in Verilog).
 *
 * KEY FIXES vs V2
 * ===============
 * 1. REMOVED big_input[1024] — caused 61B cycle serialization
 * 2. Input rows burst-read via DMA staging buffer → scatter to cache
 * 3. Weight block burst-read: entire 16 IC × K×K per OC in one DMA shot
 * 4. Output writes: phase-separated (edge-read → pack → burst-write)
 *    — eliminates conditional RMW that killed burst inference in V1/V2
 * 5. Clock 5ns (200 MHz) — fixes –2.05ns timing violation in V2
 *
 * EXPECTED PERFORMANCE
 * ====================
 *  Clock: 200 MHz, 256 MACs/cycle = 51.2 GMAC/s
 *  Tiny-YOLO: ~3.5 GMAC total → ~68ms compute → ~15 FPS
 *  (vs 0.3 FPS in V1 — 160-200× improvement)
 */
#include "conv_engine_v3.h"

/* =========================================================================
 * HELPER: Activation (inlined — becomes combinational logic)
 * Verilog analogy: a MUX tree selecting between relu/leaky/linear
 * ========================================================================= */
static data_t activate(data_t x, int use_leaky) {
    #pragma HLS INLINE
    if (use_leaky < 0) return x;              // detection layer: linear
    if (x >= (data_t)0) return x;             // positive: pass-through
    if (use_leaky > 0) {
        acc_t tmp = x;
        tmp = (tmp * (acc_t)13) >> 7;         // LeakyReLU ≈ 0.1
        return (data_t)tmp;
    }
    return (data_t)0;                         // ReLU: clamp to zero
}

/* =========================================================================
 * HELPER: Extract 16-bit element from 256-bit word
 * Verilog analogy: wire [15:0] elem = word[slot*16 +: 16];
 * ========================================================================= */
static inline data_t extract_elem(wide_t word, int slot) {
    #pragma HLS INLINE
    data_t val;
    val.range(15, 0) = word.range(slot * 16 + 15, slot * 16);
    return val;
}

/* =========================================================================
 * HELPER: Insert 16-bit element into 256-bit word
 * Verilog analogy: word[slot*16 +: 16] <= elem;
 * ========================================================================= */
static inline void insert_elem(wide_t& word, int slot, data_t val) {
    #pragma HLS INLINE
    word.range(slot * 16 + 15, slot * 16) = val.range(15, 0);
}

/* =========================================================================
 * STAGE 1: FETCH LAYER — IC-OUTER / OC-INNER
 *
 * KEY FIX: Input cache is filled ONCE per IC tile, then REUSED across
 * all OC tiles. This eliminates redundant DRAM reads that caused
 * 96% wasted cycles in the previous OC-outer design.
 *
 * Loop order: ROW → COL → IC → OC  (was ROW → COL → OC → IC)
 *
 *  ┌──────────┐     ┌───────────┐     ┌─────────────┐
 *  │  DRAM    │────→│ DMA line  │────→│ input_cache  │────→ stream
 *  │ (m_axi)  │     │ buf [4]   │     │ [16][35][35] │  (reused ×OC)
 *  └──────────┘     └───────────┘     └─────────────┘
 *
 * ========================================================================= */
void Fetch_Layer(
    wide_t* input_dram,
    wide_t* weights_dram,
    hls::stream<vec_t>& input_stream,
    hls::stream<vec_t>& weight_stream,
    int in_channels, int out_channels,
    int in_height,   int in_width,
    int kernel_size,  int stride, int padding,
    int out_height,   int out_width
) {
    int tr_steps = (out_height + TILE_H - 1) / TILE_H;
    int tc_steps = (out_width  + TILE_W - 1) / TILE_W;
    int to_steps = (out_channels + TILE_OC - 1) / TILE_OC;
    int ti_steps = (in_channels  + TILE_IC - 1) / TILE_IC;

    /* ---- On-chip BRAM caches ---- */
    data_t input_cache[TILE_IC][CACHE_H][CACHE_W];
    #pragma HLS ARRAY_PARTITION variable=input_cache dim=1 complete
    #pragma HLS BIND_STORAGE variable=input_cache type=ram_2p impl=bram

    data_t weight_cache[TILE_OC][TILE_IC][K_MAX][K_MAX];
    #pragma HLS ARRAY_PARTITION variable=weight_cache dim=1 complete
    #pragma HLS ARRAY_PARTITION variable=weight_cache dim=2 complete

    /* ---- DMA staging buffers ---- */
    wide_t dma_line[DMA_LINE_WORDS];
    #pragma HLS ARRAY_PARTITION variable=dma_line complete

    wide_t dma_wt[DMA_WT_WORDS];
    #pragma HLS ARRAY_PARTITION variable=dma_wt complete

    /* ==== MAIN TILE LOOPS — IC-OUTER, OC-INNER ==== */
    ROW_TILE: for (int tr = 0; tr < tr_steps; tr++) {
    #pragma HLS LOOP_TRIPCOUNT min=1 max=26 avg=8
        COL_TILE: for (int tc = 0; tc < tc_steps; tc++) {
        #pragma HLS LOOP_TRIPCOUNT min=1 max=26 avg=8

            int r_start = tr * TILE_H;
            int c_start = tc * TILE_W;
            int curr_h  = (r_start + TILE_H > out_height) ? out_height - r_start : TILE_H;
            int curr_w  = (c_start + TILE_W > out_width)  ? out_width  - c_start : TILE_W;
            int tile_in_h = curr_h * stride + kernel_size - 1;
            int tile_in_w = curr_w * stride + kernel_size - 1;
            int h_base = r_start * stride - padding;
            int w_base = c_start * stride - padding;

            IC_TILE: for (int ti = 0; ti < ti_steps; ti++) {
            #pragma HLS LOOP_TRIPCOUNT min=1 max=64 avg=16
                int ic_base = ti * TILE_IC;
                int ic_valid = (ic_base + TILE_IC > in_channels)
                             ? (in_channels - ic_base) : TILE_IC;

                /* ============================================================
                 * PHASE A: DMA burst-read input tile → input_cache
                 * Happens ONCE per IC tile — shared across ALL OC tiles.
                 * This is the key fix: was previously inside OC loop.
                 * ============================================================ */
                FILL_IC: for (int ic = 0; ic < TILE_IC; ic++) {
                    int abs_ic = ic_base + ic;
                    bool ic_valid_flag = (abs_ic < in_channels);

                    FILL_ROW: for (int i = 0; i < tile_in_h; i++) {
                    #pragma HLS LOOP_TRIPCOUNT min=1 max=35 avg=18
                        int r_idx = h_base + i;
                        bool row_ok = ic_valid_flag
                                   && (r_idx >= 0) && (r_idx < in_height);

                        FILL_CLEAR: for (int j = 0; j < tile_in_w; j++) {
                        #pragma HLS LOOP_TRIPCOUNT min=1 max=35 avg=18
                            #pragma HLS PIPELINE II=1
                            input_cache[ic][i][j] = (data_t)0;
                        }

                        if (row_ok) {
                            int c_lo = (w_base < 0) ? -w_base : 0;
                            int c_hi = (w_base + tile_in_w > in_width)
                                     ? (in_width - w_base) : tile_in_w;

                            if (c_lo < c_hi) {
                                int row_base  = (abs_ic * in_height + r_idx) * in_width;
                                int elem_lo   = row_base + w_base + c_lo;
                                int elem_hi   = row_base + w_base + c_hi - 1;
                                int first_word = elem_lo >> 4;
                                int last_word  = elem_hi >> 4;
                                int n_words    = last_word - first_word + 1;

                                DMA_IN_BURST: for (int w = 0; w < n_words; w++) {
                                #pragma HLS LOOP_TRIPCOUNT min=1 max=4 avg=2
                                    #pragma HLS PIPELINE II=1
                                    dma_line[w] = input_dram[first_word + w];
                                }

                                SCATTER_IN: for (int j = c_lo; j < c_hi; j++) {
                                #pragma HLS LOOP_TRIPCOUNT min=1 max=35 avg=16
                                    #pragma HLS PIPELINE II=1
                                    int abs_idx = row_base + w_base + j;
                                    int wi = (abs_idx >> 4) - first_word;
                                    int si =  abs_idx & 0xF;
                                    input_cache[ic][i][j] = extract_elem(dma_line[wi], si);
                                }
                            }
                        }
                    }
                }

                /* ============================================================
                 * PHASE D: Stream inputs → Execute (ONCE per IC tile)
                 * Previously inside OC loop — moved here for 2-64× fewer
                 * stream writes.  Execute caches locally and reuses ×OC.
                 * ============================================================ */
                STREAM_IN_KY: for (int ky = 0; ky < kernel_size; ky++) {
                #pragma HLS LOOP_TRIPCOUNT min=1 max=3 avg=3
                    STREAM_IN_KX: for (int kx = 0; kx < kernel_size; kx++) {
                    #pragma HLS LOOP_TRIPCOUNT min=1 max=3 avg=3
                        STREAM_IN_I: for (int i = 0; i < curr_h; i++) {
                        #pragma HLS LOOP_TRIPCOUNT min=1 max=16 avg=16
                            STREAM_IN_J: for (int j = 0; j < curr_w; j++) {
                            #pragma HLS LOOP_TRIPCOUNT min=1 max=16 avg=16
                                #pragma HLS PIPELINE II=1
                                vec_t in_vec;
                                PACK_IN_IC: for (int ic = 0; ic < TILE_IC; ic++) {
                                    #pragma HLS UNROLL
                                    int row = i * stride + ky;
                                    int col = j * stride + kx;
                                    in_vec.range(ic*16+15, ic*16) =
                                        input_cache[ic][row][col].range(15, 0);
                                }
                                input_stream.write(in_vec);
                            }
                        }
                    }
                }

                /* ============================================================
                 * Iterate OC tiles — only WEIGHTS streamed per OC tile.
                 * CLEAR_WT removed: stale weights × zero-padded input = 0,
                 * and invalid OC outputs are discarded by Write_Layer.
                 * sdiv/srem replaced with nested ky/kx loops.
                 * ============================================================ */
                OC_TILE: for (int to = 0; to < to_steps; to++) {
                #pragma HLS LOOP_TRIPCOUNT min=1 max=64 avg=16
                    int oc_base = to * TILE_OC;
                    int oc_valid = (oc_base + TILE_OC > out_channels)
                                 ? (out_channels - oc_base) : TILE_OC;

                    /* ---- Load weights for this (OC, IC) pair ---- */
                    LD_WT_OC: for (int oc = 0; oc < oc_valid; oc++) {
                    #pragma HLS LOOP_TRIPCOUNT min=1 max=16 avg=16
                        int oc_abs = oc_base + oc;
                        int block_start = (oc_abs * in_channels + ic_base) * kernel_size * kernel_size;
                        int block_elems = ic_valid * kernel_size * kernel_size;
                        int fw = block_start >> 4;
                        int lw = (block_start + block_elems - 1) >> 4;
                        int nw = lw - fw + 1;

                        DMA_WT_BURST: for (int w = 0; w < nw; w++) {
                        #pragma HLS LOOP_TRIPCOUNT min=1 max=12 avg=9
                            #pragma HLS PIPELINE II=1
                            dma_wt[w] = weights_dram[fw + w];
                        }

                        UNPACK_WT_IC: for (int ic = 0; ic < ic_valid; ic++) {
                        #pragma HLS LOOP_TRIPCOUNT min=1 max=16 avg=16
                            UNPACK_WT_KY: for (int ky = 0; ky < kernel_size; ky++) {
                            #pragma HLS LOOP_TRIPCOUNT min=1 max=3 avg=3
                                UNPACK_WT_KX: for (int kx = 0; kx < kernel_size; kx++) {
                                #pragma HLS LOOP_TRIPCOUNT min=1 max=3 avg=3
                                    #pragma HLS PIPELINE II=1
                                    int flat = block_start
                                             + ic * kernel_size * kernel_size
                                             + ky * kernel_size + kx;
                                    int wi = (flat >> 4) - fw;
                                    int si =  flat & 0xF;
                                    weight_cache[oc][ic][ky][kx]
                                        = extract_elem(dma_wt[wi], si);
                                }
                            }
                        }
                    }

                    /* ---- Stream weights → Execute ---- */
                    STREAM_WT_OC: for (int oc = 0; oc < TILE_OC; oc++) {
                        STREAM_WT_KY: for (int ky = 0; ky < kernel_size; ky++) {
                        #pragma HLS LOOP_TRIPCOUNT min=1 max=3 avg=3
                            STREAM_WT_KX: for (int kx = 0; kx < kernel_size; kx++) {
                            #pragma HLS LOOP_TRIPCOUNT min=1 max=3 avg=3
                                #pragma HLS PIPELINE II=1
                                vec_t w_vec;
                                PACK_WT_IC: for (int ic = 0; ic < TILE_IC; ic++) {
                                    #pragma HLS UNROLL
                                    w_vec.range(ic*16+15, ic*16) =
                                        weight_cache[oc][ic][ky][kx].range(15, 0);
                                }
                                weight_stream.write(w_vec);
                            }
                        }
                    }

                } /* OC_TILE */
            } /* IC_TILE */
        }
    }
}

/* =========================================================================
 * STAGE 2: EXECUTE LAYER — IC-OUTER / OC-INNER with PSUM BUFFER
 *
 * Loop order: ROW → COL → IC → OC  (matches Fetch_Layer)
 *
 * Because IC is the outer loop, partial sums must persist across IC tiles.
 * psum_buf[MAX_OC_STEPS][TILE_OC][TILE_H][TILE_W] in BRAM stores
 * intermediate accumulations.  On the first IC tile, acc_buf is cleared.
 * On intermediate IC tiles, acc_buf is loaded from / saved to psum_buf.
 * On the last IC tile, BN + Activate + Stream Out is performed.
 *
 * 256-MAC tree: 16 OC × 16 IC DSP-mapped multiplies per cycle at II=1.
 * ========================================================================= */
void Execute_Layer(
    hls::stream<vec_t>& input_stream,
    hls::stream<vec_t>& weight_stream,
    hls::stream<vec_t>& output_stream,
    data_t* bn_params,
    int in_channels, int out_channels,
    int out_height,   int out_width,
    int kernel_size,  int use_leaky
) {
    int tr_steps = (out_height + TILE_H - 1) / TILE_H;
    int tc_steps = (out_width  + TILE_W - 1) / TILE_W;
    int to_steps = (out_channels + TILE_OC - 1) / TILE_OC;
    int ti_steps = (in_channels  + TILE_IC - 1) / TILE_IC;

    /* ---- Accumulator register file (16 OC × 16 H × 16 W) ---- */
    acc_t acc_buf[TILE_OC][TILE_H][TILE_W];
    #pragma HLS ARRAY_PARTITION variable=acc_buf dim=1 complete
    #pragma HLS ARRAY_PARTITION variable=acc_buf dim=3 complete

    /* ---- Weight register file (256 × K×K) ---- */
    data_t wt_buf[TILE_OC][TILE_IC][K_MAX][K_MAX];
    #pragma HLS ARRAY_PARTITION variable=wt_buf dim=1 complete
    #pragma HLS ARRAY_PARTITION variable=wt_buf dim=2 complete

    /* ---- Local input cache (filled once per IC tile, reused ×OC) ----
     * Eliminates redundant input_stream reads.  ~36 BRAM18K. */
    vec_t input_lcl[K_MAX][K_MAX][TILE_H][TILE_W];
    #pragma HLS BIND_STORAGE variable=input_lcl type=ram_2p impl=bram

    /* ---- BN parameter registers ---- */
    data_t scale_buf[TILE_OC];
    data_t bias_buf[TILE_OC];
    #pragma HLS ARRAY_PARTITION variable=scale_buf complete
    #pragma HLS ARRAY_PARTITION variable=bias_buf complete

    /* ---- Partial sum buffer for IC-outer accumulation ----
     * Stores intermediate partial sums across IC tiles.
     * Partitioned on dim=4 (W) for 16-way parallel load/save at II=1.
     * With TILE_OC=16 and MAX_OC_STEPS=64: 64×16×16×16 = 262K entries.
     * ~512 BRAM18K (~28% of ZCU102's 1824). */
    acc_t psum_buf[MAX_OC_STEPS][TILE_OC][TILE_H][TILE_W];
    #pragma HLS ARRAY_PARTITION variable=psum_buf dim=4 complete
    #pragma HLS BIND_STORAGE variable=psum_buf type=ram_2p impl=bram

    EXEC_ROW: for (int tr = 0; tr < tr_steps; tr++) {
    #pragma HLS LOOP_TRIPCOUNT min=1 max=26 avg=8
        EXEC_COL: for (int tc = 0; tc < tc_steps; tc++) {
        #pragma HLS LOOP_TRIPCOUNT min=1 max=26 avg=8

            int r_start = tr * TILE_H;
            int c_start = tc * TILE_W;
            int curr_h = (r_start + TILE_H > out_height) ? out_height - r_start : TILE_H;
            int curr_w = (c_start + TILE_W > out_width)  ? out_width  - c_start : TILE_W;

            EXEC_IC: for (int ti = 0; ti < ti_steps; ti++) {
            #pragma HLS LOOP_TRIPCOUNT min=1 max=64 avg=16
                bool is_first_ic = (ti == 0);
                bool is_last_ic  = (ti == ti_steps - 1);

                /* -- Read input stream → local cache (ONCE per IC tile) -- */
                LOAD_IN_KY: for (int ky = 0; ky < kernel_size; ky++) {
                #pragma HLS LOOP_TRIPCOUNT min=1 max=3 avg=3
                    LOAD_IN_KX: for (int kx = 0; kx < kernel_size; kx++) {
                    #pragma HLS LOOP_TRIPCOUNT min=1 max=3 avg=3
                        LOAD_IN_I: for (int i = 0; i < curr_h; i++) {
                        #pragma HLS LOOP_TRIPCOUNT min=1 max=16 avg=16
                            LOAD_IN_J: for (int j = 0; j < curr_w; j++) {
                            #pragma HLS LOOP_TRIPCOUNT min=1 max=16 avg=16
                                #pragma HLS PIPELINE II=1
                                input_lcl[ky][kx][i][j] = input_stream.read();
                            }
                        }
                    }
                }

                EXEC_OC: for (int to = 0; to < to_steps; to++) {
                #pragma HLS LOOP_TRIPCOUNT min=1 max=64 avg=16

                    /* -- Load BN params (only on last IC tile) -- */
                    if (is_last_ic) {
                        LOAD_BN: for (int idx = 0; idx < TILE_OC * 2; idx++) {
                            #pragma HLS PIPELINE II=1
                            #pragma HLS LOOP_TRIPCOUNT min=32 max=32 avg=32
                            data_t val = bn_params[to * TILE_OC * 2 + idx];
                            if (idx & 1)
                                bias_buf[idx >> 1] = val;
                            else
                                scale_buf[idx >> 1] = val;
                        }
                    }

                    /* -- Initialize accumulator -- */
                    if (is_first_ic) {
                        /* Clear on first IC tile */
                        CLEAR_ACC_H: for (int i = 0; i < curr_h; i++) {
                        #pragma HLS LOOP_TRIPCOUNT min=1 max=16 avg=16
                            CLEAR_ACC_W: for (int j = 0; j < curr_w; j++) {
                            #pragma HLS LOOP_TRIPCOUNT min=1 max=16 avg=16
                                #pragma HLS PIPELINE II=1
                                CLEAR_ACC_OC: for (int oc = 0; oc < TILE_OC; oc++) {
                                    #pragma HLS UNROLL
                                    acc_buf[oc][i][j] = 0;
                                }
                            }
                        }
                    } else {
                        /* Load partial sums from psum_buf (256 cycles) */
                        LOAD_PSUM: for (int idx = 0; idx < TILE_OC * TILE_H; idx++) {
                        #pragma HLS LOOP_TRIPCOUNT min=256 max=256 avg=256
                            #pragma HLS PIPELINE II=1
                            int oc = idx >> 4;   /* idx / 16 */
                            int i  = idx & 0xF;  /* idx % 16 */
                            LOAD_PSUM_W: for (int j = 0; j < TILE_W; j++) {
                                #pragma HLS UNROLL
                                acc_buf[oc][i][j] = psum_buf[to][oc][i][j];
                            }
                        }
                    }

                    /* -- Read weight stream → register file -- */
                    READ_WT_OC: for (int oc = 0; oc < TILE_OC; oc++) {
                        READ_WT_KY: for (int ky = 0; ky < kernel_size; ky++) {
                        #pragma HLS LOOP_TRIPCOUNT min=1 max=3 avg=3
                            READ_WT_KX: for (int kx = 0; kx < kernel_size; kx++) {
                            #pragma HLS LOOP_TRIPCOUNT min=1 max=3 avg=3
                                #pragma HLS PIPELINE II=1
                                vec_t w_pkg = weight_stream.read();
                                RD_WT_UNROLL: for (int ic = 0; ic < TILE_IC; ic++) {
                                    #pragma HLS UNROLL
                                    wt_buf[oc][ic][ky][kx].range(15, 0) =
                                        w_pkg.range(ic*16+15, ic*16);
                                }
                            }
                        }
                    }

                    /* -- 256-MAC compute (DSP-mapped) -- */
                    COMPUTE_KY: for (int ky = 0; ky < kernel_size; ky++) {
                    #pragma HLS LOOP_TRIPCOUNT min=1 max=3 avg=3
                        COMPUTE_KX: for (int kx = 0; kx < kernel_size; kx++) {
                        #pragma HLS LOOP_TRIPCOUNT min=1 max=3 avg=3
                            #pragma HLS LOOP_FLATTEN off

                            COMPUTE_I: for (int i = 0; i < curr_h; i++) {
                            #pragma HLS LOOP_TRIPCOUNT min=1 max=16 avg=16
                                COMPUTE_J: for (int j = 0; j < curr_w; j++) {
                                #pragma HLS LOOP_TRIPCOUNT min=1 max=16 avg=16
                                    #pragma HLS PIPELINE II=1

                                    vec_t in_pkg = input_lcl[ky][kx][i][j];

                                    /* 16 OC × 16 IC = 256 MACs per cycle */
                                    MAC_OC: for (int oc = 0; oc < TILE_OC; oc++) {
                                        #pragma HLS UNROLL
                                        acc_t dot = 0;
                                        MAC_IC: for (int ic = 0; ic < TILE_IC; ic++) {
                                            #pragma HLS UNROLL
                                            data_t w_val = wt_buf[oc][ic][ky][kx];
                                            data_t in_val;
                                            in_val.range(15, 0) =
                                                in_pkg.range(ic*16+15, ic*16);
                                            acc_t prod = (acc_t)(w_val * in_val);
                                            #pragma HLS BIND_OP variable=prod op=mul impl=dsp
                                            dot += prod;
                                        }
                                        acc_buf[oc][i][j] += dot;
                                    }
                                }
                            }
                        }
                    }

                    /* -- Post-process OR save partial sums -- */
                    if (is_last_ic) {
                        /* Last IC tile: BN + Activate + Stream Out */
                        STREAM_OUT_H: for (int i = 0; i < curr_h; i++) {
                        #pragma HLS LOOP_TRIPCOUNT min=1 max=16 avg=16
                            STREAM_OUT_W: for (int j = 0; j < curr_w; j++) {
                            #pragma HLS LOOP_TRIPCOUNT min=1 max=16 avg=16
                                #pragma HLS PIPELINE II=1
                                vec_t out_pkg;
                                OUT_PACK_OC: for (int oc = 0; oc < TILE_OC; oc++) {
                                    #pragma HLS UNROLL
                                    acc_t bn_mul = acc_buf[oc][i][j] * scale_buf[oc];
                                    #pragma HLS BIND_OP variable=bn_mul op=mul impl=dsp
                                    data_t res = activate(
                                        (data_t)(bn_mul + bias_buf[oc]),
                                        use_leaky);
                                    out_pkg.range(oc*16+15, oc*16) = res.range(15, 0);
                                }
                                output_stream.write(out_pkg);
                            }
                        }
                    } else {
                        /* Save partial sums to psum_buf (256 cycles) */
                        SAVE_PSUM: for (int idx = 0; idx < TILE_OC * TILE_H; idx++) {
                        #pragma HLS LOOP_TRIPCOUNT min=256 max=256 avg=256
                            #pragma HLS PIPELINE II=1
                            int oc = idx >> 4;   /* idx / 16 */
                            int i  = idx & 0xF;  /* idx % 16 */
                            SAVE_PSUM_W: for (int j = 0; j < TILE_W; j++) {
                                #pragma HLS UNROLL
                                psum_buf[to][oc][i][j] = acc_buf[oc][i][j];
                            }
                        }
                    }

                } /* EXEC_OC */
            } /* EXEC_IC */
        }
    }
}

/* =========================================================================
 * STAGE 3: WRITE LAYER — DMA BURST WRITE ENGINE
 *
 * Key innovation vs V1/V2: phase-separated output writes.
 *
 * V1/V2 approach (BROKEN for bursts):
 *   read_DRAM → modify word → write_DRAM  (interleaved, conditional)
 *   HLS cannot infer burst because reads/writes are interleaved.
 *
 * V3 approach (Verilog FSM-style):
 *   Phase 1: READ stream → tile_buf (on-chip)
 *   Phase 2: For each output row of each OC channel:
 *     2a. READ edge words from DRAM → dma_out[] (1-2 scalar reads)
 *     2b. PACK tile data into dma_out[] (pure register ops, no DRAM)
 *     2c. BURST WRITE dma_out[] → DRAM (clean sequential burst)
 *
 * This separation guarantees burst inference for the write phase.
 *
 *  ┌────────────┐     ┌───────────┐     ┌──────────┐
 *  │ tile_buf   │────→│ dma_out[] │────→│  DRAM    │
 *  │ [16][16][16│     │ staging   │     │ (m_axi)  │
 *  └────────────┘     └───────────┘     └──────────┘
 *  BRAM (on-chip)     Pack buffer       AXI burst write
 *
 * ========================================================================= */
void Write_Layer(
    wide_t* output_dram,
    hls::stream<vec_t>& output_stream,
    int out_channels, int out_height, int out_width,
    int use_pool,     int pool_stride
) {
    int tr_steps = (out_height + TILE_H - 1) / TILE_H;
    int tc_steps = (out_width  + TILE_W - 1) / TILE_W;
    int to_steps = (out_channels + TILE_OC - 1) / TILE_OC;

    /* ---- Tile buffer BRAM ---- */
    data_t tile_buf[TILE_OC][TILE_H][TILE_W];
    #pragma HLS ARRAY_PARTITION variable=tile_buf dim=1 complete
    #pragma HLS ARRAY_PARTITION variable=tile_buf dim=3 complete

    /* ---- DMA write staging buffer (Verilog: AXI write channel FIFO) ---- */
    wide_t dma_out[DMA_OUT_WORDS];

    /* ---- Pooling scratch buffer ---- */
    data_t pool_row[TILE_W / 2];
    #pragma HLS ARRAY_PARTITION variable=pool_row complete

    /* Compute final DRAM dimensions */
    int final_h, final_w;
    if (use_pool && pool_stride >= 2) {
        final_h = out_height / pool_stride;
        final_w = out_width  / pool_stride;
    } else {
        final_h = out_height;
        final_w = out_width;
    }

    WB_ROW: for (int tr = 0; tr < tr_steps; tr++) {
    #pragma HLS LOOP_TRIPCOUNT min=1 max=26 avg=8
        WB_COL: for (int tc = 0; tc < tc_steps; tc++) {
        #pragma HLS LOOP_TRIPCOUNT min=1 max=26 avg=8
            WB_OC: for (int to = 0; to < to_steps; to++) {
            #pragma HLS LOOP_TRIPCOUNT min=1 max=64 avg=16

                int r_start = tr * TILE_H;
                int c_start = tc * TILE_W;
                int curr_h  = (r_start + TILE_H > out_height) ? out_height - r_start : TILE_H;
                int curr_w  = (c_start + TILE_W > out_width)  ? out_width  - c_start : TILE_W;
                int oc_limit = (to * TILE_OC + TILE_OC > out_channels)
                             ? (out_channels - to * TILE_OC) : TILE_OC;

                /* ====== Phase 1: Read output stream → tile_buf ======
                 * Verilog: deserialize stream into BRAM */
                RD_STREAM_H: for (int i = 0; i < curr_h; i++) {
                #pragma HLS LOOP_TRIPCOUNT min=1 max=16 avg=16
                    RD_STREAM_W: for (int j = 0; j < curr_w; j++) {
                    #pragma HLS LOOP_TRIPCOUNT min=1 max=16 avg=16
                        #pragma HLS PIPELINE II=1
                        vec_t out_pkg = output_stream.read();
                        UNPACK_OC: for (int oc = 0; oc < TILE_OC; oc++) {
                            #pragma HLS UNROLL
                            tile_buf[oc][i][j].range(15, 0) =
                                out_pkg.range(oc*16+15, oc*16);
                        }
                    }
                }

                /* ====== Phase 2: DMA write to DRAM ======
                 * For each OC channel, for each row, pack + burst-write. */

                if (use_pool && pool_stride >= 2) {
                    /* ---- Pooled write path ---- */
                    int ph = curr_h / pool_stride;
                    int pw = curr_w / pool_stride;

                    POOL_WR_OC: for (int oc = 0; oc < TILE_OC; oc++) {
                        if (oc >= oc_limit) continue;
                        int global_oc = to * TILE_OC + oc;

                        POOL_WR_ROW: for (int pi = 0; pi < ph; pi++) {
                        #pragma HLS LOOP_TRIPCOUNT min=1 max=8 avg=4

                            /* -- Step 2a: Compute pooled values into scratch -- */
                            POOL_CALC: for (int pj = 0; pj < pw; pj++) {
                            #pragma HLS LOOP_TRIPCOUNT min=1 max=8 avg=4
                                #pragma HLS PIPELINE II=1
                                data_t v0 = tile_buf[oc][pi*2  ][pj*2  ];
                                data_t v1 = tile_buf[oc][pi*2+1][pj*2  ];
                                data_t v2 = tile_buf[oc][pi*2  ][pj*2+1];
                                data_t v3 = tile_buf[oc][pi*2+1][pj*2+1];
                                data_t mx01 = (v0 > v1) ? v0 : v1;
                                data_t mx23 = (v2 > v3) ? v2 : v3;
                                pool_row[pj] = (mx01 > mx23) ? mx01 : mx23;
                            }

                            /* -- Step 2b: Address decode for this output row -- */
                            int out_r = (r_start / pool_stride) + pi;
                            int out_c = c_start / pool_stride;
                            int base_idx = (global_oc * final_h + out_r) * final_w + out_c;
                            int first_word = base_idx >> 4;
                            int start_slot = base_idx & 0xF;
                            int end_idx    = base_idx + pw - 1;
                            int last_word  = end_idx >> 4;
                            int n_words    = last_word - first_word + 1;

                            /* -- Step 2c: Read edge words (Verilog: AXI read) --
                             * Only read the first/last word if they're partial.
                             * This is the ONLY place we read from output DRAM. */
                            EDGE_RD_POOL: for (int w = 0; w < n_words; w++) {
                            #pragma HLS LOOP_TRIPCOUNT min=1 max=2 avg=1
                                #pragma HLS PIPELINE II=1
                                if ((w == 0 && start_slot != 0) ||
                                    (w == n_words - 1 && (end_idx & 0xF) != 15)) {
                                    dma_out[w] = output_dram[first_word + w];
                                } else {
                                    dma_out[w] = (wide_t)0;
                                }
                            }

                            /* -- Step 2d: Pack pooled values into staging buffer --
                             * Pure on-chip: pool_row → dma_out words
                             * Verilog: MUX + bit-insert logic */
                            PACK_POOL: for (int pj = 0; pj < pw; pj++) {
                            #pragma HLS LOOP_TRIPCOUNT min=1 max=8 avg=4
                                #pragma HLS PIPELINE II=1
                                int flat = base_idx + pj;
                                int wi   = (flat >> 4) - first_word;
                                int si   =  flat & 0xF;
                                insert_elem(dma_out[wi], si, pool_row[pj]);
                            }

                            /* -- Step 2e: Burst write staging → DRAM --
                             * Clean sequential loop. No conditionals.
                             * Verilog: AXI AWLEN=n_words-1, AWBURST=INCR */
                            BURST_WR_POOL: for (int w = 0; w < n_words; w++) {
                            #pragma HLS LOOP_TRIPCOUNT min=1 max=2 avg=1
                                #pragma HLS PIPELINE II=1
                                output_dram[first_word + w] = dma_out[w];
                            }
                        }
                    }

                } else {
                    /* ---- Direct write path (no pooling) ---- */
                    DIRECT_WR_OC: for (int oc = 0; oc < TILE_OC; oc++) {
                        if (oc >= oc_limit) continue;
                        int global_oc = to * TILE_OC + oc;

                        DIRECT_WR_ROW: for (int i = 0; i < curr_h; i++) {
                        #pragma HLS LOOP_TRIPCOUNT min=1 max=16 avg=16

                            /* -- Step 2b: Address decode for this output row -- */
                            int base_idx   = (global_oc * out_height + r_start + i) * out_width + c_start;
                            int first_word = base_idx >> 4;
                            int start_slot = base_idx & 0xF;
                            int end_idx    = base_idx + curr_w - 1;
                            int last_word  = end_idx >> 4;
                            int n_words    = last_word - first_word + 1;

                            /* -- Step 2c: Read edge words (Verilog: AXI read) --
                             * Only partial words need read-modify-write.
                             * Full words are overwritten completely. */
                            EDGE_RD_DIRECT: for (int w = 0; w < n_words; w++) {
                            #pragma HLS LOOP_TRIPCOUNT min=1 max=2 avg=1
                                #pragma HLS PIPELINE II=1
                                if ((w == 0 && start_slot != 0) ||
                                    (w == n_words - 1 && (end_idx & 0xF) != 15)) {
                                    dma_out[w] = output_dram[first_word + w];
                                } else {
                                    dma_out[w] = (wide_t)0;
                                }
                            }

                            /* -- Step 2d: Pack tile data into staging buffer --
                             * Pure on-chip: tile_buf → dma_out words
                             * Verilog: data path packing logic */
                            PACK_DIRECT: for (int j = 0; j < curr_w; j++) {
                            #pragma HLS LOOP_TRIPCOUNT min=1 max=16 avg=16
                                #pragma HLS PIPELINE II=1
                                int flat = base_idx + j;
                                int wi   = (flat >> 4) - first_word;
                                int si   =  flat & 0xF;
                                insert_elem(dma_out[wi], si, tile_buf[oc][i][j]);
                            }

                            /* -- Step 2e: Burst write staging → DRAM --
                             * Clean sequential loop. No conditionals.
                             * Verilog: AXI AWLEN=n_words-1, AWBURST=INCR, WVALID toggling */
                            BURST_WR_DIRECT: for (int w = 0; w < n_words; w++) {
                            #pragma HLS LOOP_TRIPCOUNT min=1 max=2 avg=1
                                #pragma HLS PIPELINE II=1
                                output_dram[first_word + w] = dma_out[w];
                            }
                        }
                    }
                }

            } /* WB_OC */
        }
    }
}

/* =========================================================================
 * DATAFLOW CORE
 *
 * Verilog analogy: top-level structural connection of three pipeline stages
 * connected by FIFO channels.
 *
 *  Fetch ──[input_stream]──→ Execute ──[output_stream]──→ Write
 *       └──[weight_stream]──┘
 * ========================================================================= */
static void conv_dataflow(
    wide_t* input_dram,
    wide_t* output_dram,
    wide_t* weights_dram,
    data_t* bn_params_dram,
    int in_channels, int out_channels,
    int in_height,   int in_width,
    int kernel_size,  int stride, int padding,
    int use_pool,     int pool_stride, int use_leaky,
    int out_height,   int out_width
) {
    #pragma HLS DATAFLOW

    /* ---- Stream FIFOs (Verilog: hls::stream = FIFO primitive) ---- */
    hls::stream<vec_t> input_stream("input_stream");
    hls::stream<vec_t> weight_stream("weight_stream");
    hls::stream<vec_t> output_stream("output_stream");

    #pragma HLS STREAM variable=input_stream  depth=8192
    #pragma HLS STREAM variable=weight_stream depth=4096
    #pragma HLS STREAM variable=output_stream depth=4096

    Fetch_Layer(input_dram, weights_dram, input_stream, weight_stream,
                in_channels, out_channels, in_height, in_width,
                kernel_size, stride, padding, out_height, out_width);

    Execute_Layer(input_stream, weight_stream, output_stream, bn_params_dram,
                  in_channels, out_channels, out_height, out_width,
                  kernel_size, use_leaky);

    Write_Layer(output_dram, output_stream, out_channels, out_height, out_width,
                use_pool, pool_stride);
}

/* =========================================================================
 * TOP LEVEL — AXI Interface Binding
 *
 * Verilog analogy: top-level port map & AXI protocol wrapper
 *
 * Port mapping:
 *   gmem0 (256-bit, READ)  ← input_dram   (activations)
 *   gmem1 (256-bit, R/W)   ← output_dram  (output feature map)
 *   gmem2 (256-bit, READ)  ← weights_dram (convolution weights)
 *   gmem3 (16-bit,  READ)  ← bn_params    (batch-norm scale/bias)
 *   s_axi_control           ← all scalar parameters
 * ========================================================================= */
extern "C" void conv_engine(
    wide_t* input_dram,
    wide_t* output_dram,
    wide_t* weights_dram,
    data_t* bn_params_dram,
    int in_channels,  int out_channels,
    int in_height,    int in_width,
    int kernel_size,  int stride, int padding,
    int use_pool,     int pool_stride,
    int use_leaky
) {
    /* ---- AXI master ports (Verilog: AXI4 master interfaces) ----
     * max_read/write_burst_length enables the DMA staging approach.
     * Burst length 64 × 256 bits = 2048 bytes per AXI transaction. */
    #pragma HLS INTERFACE m_axi port=input_dram    bundle=gmem0 depth=1000000 \
        max_read_burst_length=64
    #pragma HLS INTERFACE m_axi port=output_dram   bundle=gmem1 depth=1000000 \
        max_read_burst_length=64 max_write_burst_length=64
    #pragma HLS INTERFACE m_axi port=weights_dram  bundle=gmem2 depth=5000000 \
        max_read_burst_length=64
    #pragma HLS INTERFACE m_axi port=bn_params_dram bundle=gmem3 depth=4096

    /* ---- AXI-Lite slave control (Verilog: s_axi_control register bank) ---- */
    #pragma HLS INTERFACE s_axilite port=input_dram     bundle=control
    #pragma HLS INTERFACE s_axilite port=output_dram    bundle=control
    #pragma HLS INTERFACE s_axilite port=weights_dram   bundle=control
    #pragma HLS INTERFACE s_axilite port=bn_params_dram bundle=control

    #pragma HLS INTERFACE s_axilite port=in_channels   bundle=control
    #pragma HLS INTERFACE s_axilite port=out_channels  bundle=control
    #pragma HLS INTERFACE s_axilite port=in_height     bundle=control
    #pragma HLS INTERFACE s_axilite port=in_width      bundle=control
    #pragma HLS INTERFACE s_axilite port=kernel_size   bundle=control
    #pragma HLS INTERFACE s_axilite port=stride        bundle=control
    #pragma HLS INTERFACE s_axilite port=padding       bundle=control
    #pragma HLS INTERFACE s_axilite port=use_pool      bundle=control
    #pragma HLS INTERFACE s_axilite port=pool_stride   bundle=control
    #pragma HLS INTERFACE s_axilite port=use_leaky     bundle=control
    #pragma HLS INTERFACE s_axilite port=return        bundle=control

    if (kernel_size > K_MAX) return;

    /* ---- Compute output dimensions (Verilog: combinational logic) ---- */
    int out_height = (in_height + 2 * padding - kernel_size) / stride + 1;
    int out_width  = (in_width  + 2 * padding - kernel_size) / stride + 1;

    conv_dataflow(input_dram, output_dram, weights_dram, bn_params_dram,
                  in_channels, out_channels, in_height, in_width,
                  kernel_size, stride, padding,
                  use_pool, pool_stride, use_leaky,
                  out_height, out_width);
}
