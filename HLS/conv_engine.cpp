/**
 * conv_engine.cpp
 * Optimized Hardware Accelerator for CNN Convolution Layer
 */
#include "conv_engine.h"

// ============================================================================
// HELPER: Activation Function
// ============================================================================
static data_t activate(data_t x, int use_leaky) {
    #pragma HLS INLINE
    if (use_leaky < 0) return x; 
    if (x >= 0) return x;
    else {
        if (use_leaky > 0) {
            acc_t tmp = x;
            tmp = (tmp * 13) >> 7; 
            return (data_t)tmp;
        } else {
            return 0;
        }
    }
}

// ============================================================================
// STAGE 1: FETCH LAYER
// - Reads Inputs (Burst Optimized)
// - Streams Weights (Once per Tile)
// - Streams Inputs (K-Major Order: Ky -> Kx -> i -> j)
// ============================================================================
void Fetch_Layer(
    wide_t* input_dram,
    wide_t* weights_dram,
    hls::stream<vec_t>& input_stream,
    hls::stream<vec_t>& weight_stream,
    int in_channels, int out_channels,
    int in_height, int in_width,
    int kernel_size, int stride, int padding,
    int out_height, int out_width
) {
    int tr_steps = (out_height + TILE_H - 1) / TILE_H;
    int tc_steps = (out_width  + TILE_W - 1) / TILE_W;
    int to_steps = (out_channels + TILE_OC - 1) / TILE_OC;
    int ti_steps = (in_channels  + TILE_IC - 1) / TILE_IC;

    // Input Cache (Dimensions for Max Stride)
    data_t input_cache[TILE_IC][CACHE_H][CACHE_W];
    #pragma HLS ARRAY_PARTITION variable=input_cache dim=1 complete

    // Weight Cache (Element-wise, partitioned on OC and IC for parallel access)
    data_t weight_cache[TILE_OC][TILE_IC][K_MAX][K_MAX];
    #pragma HLS ARRAY_PARTITION variable=weight_cache dim=1 complete
    #pragma HLS ARRAY_PARTITION variable=weight_cache dim=2 complete

    ROW_TILE: for (int tr = 0; tr < tr_steps; tr++) {
    #pragma HLS LOOP_TRIPCOUNT min=1 max=26 avg=8
        COL_TILE: for (int tc = 0; tc < tc_steps; tc++) {
        #pragma HLS LOOP_TRIPCOUNT min=1 max=26 avg=8
            OC_TILE: for (int to = 0; to < to_steps; to++) {
            #pragma HLS LOOP_TRIPCOUNT min=1 max=64 avg=16

                int r_start = tr * TILE_H;
                int c_start = tc * TILE_W;
                int r_end = (r_start + TILE_H > out_height) ? out_height : r_start + TILE_H;
                int c_end = (c_start + TILE_W > out_width)  ? out_width  : c_start + TILE_W;
                int curr_h = r_end - r_start;
                int curr_w = c_end - c_start;

                int tile_in_h = curr_h * stride + kernel_size - 1;
                int tile_in_w = curr_w * stride + kernel_size - 1;
                int h_base = r_start * stride - padding;
                int w_base = c_start * stride - padding;

                IC_TILE: for (int ti = 0; ti < ti_steps; ti++) {
                    #pragma HLS LOOP_TRIPCOUNT min=1 max=64 avg=16
                    int ic_base = ti * TILE_IC;
                    int oc_base = to * TILE_OC;

                    // 1. Load Input Cache
                    LOAD_IC: for (int ic = 0; ic < TILE_IC; ic++) {
                        bool ic_valid = (ic_base + ic < in_channels);
                        for (int i = 0; i < tile_in_h; i++) {
                            #pragma HLS LOOP_TRIPCOUNT min=3 max=34 avg=18
                            int r_idx = h_base + i;
                            bool row_valid = ic_valid && (r_idx >= 0) && (r_idx < in_height);
                            int row_offset = ((ic_base + ic) * in_height + r_idx) * in_width;
                            
                            // Burst read: compute valid column range
                            int c_start_valid = (w_base < 0) ? -w_base : 0;
                            int c_end_valid = (w_base + tile_in_w > in_width) ? (in_width - w_base) : tile_in_w;
                            
                            // Clear cache row first
                            CLEAR_COL: for (int j = 0; j < tile_in_w; j++) {
                                #pragma HLS LOOP_TRIPCOUNT min=3 max=34 avg=18
                                #pragma HLS UNROLL
                                input_cache[ic][i][j] = 0;
                            }
                            
                            // Burst read only valid range (unconditional within range)
                            if (row_valid && c_start_valid < c_end_valid) {
                                int burst_start = row_offset + w_base + c_start_valid;
                                int burst_len = c_end_valid - c_start_valid;
                                
                                LOAD_COL: for (int j = 0; j < burst_len; j++) {
                                    #pragma HLS LOOP_TRIPCOUNT min=1 max=34 avg=16
                                    #pragma HLS PIPELINE II=1
                                    int idx = burst_start + j;
                                    wide_t packet = input_dram[idx / 16];
                                    data_t val;
                                    val.range(15,0) = packet.range((idx % 16)*16+15, (idx % 16)*16);
                                    input_cache[ic][i][c_start_valid + j] = val;
                                }
                            }
                        }
                    }

                    // 2. Load Weights (Burst optimized - unconditional read, then mask)
                    // Pre-clear weight cache
                    CLEAR_WT_OC: for (int oc = 0; oc < TILE_OC; oc++) {
                        #pragma HLS UNROLL
                        for (int ic = 0; ic < TILE_IC; ic++) {
                            #pragma HLS UNROLL
                            for (int ky = 0; ky < K_MAX; ky++) {
                                for (int kx = 0; kx < K_MAX; kx++) {
                                    weight_cache[oc][ic][ky][kx] = 0;
                                }
                            }
                        }
                    }
                    
                    // Compute valid OC/IC ranges
                    int oc_valid = (oc_base + TILE_OC > out_channels) ? (out_channels - oc_base) : TILE_OC;
                    int ic_valid = (ic_base + TILE_IC > in_channels) ? (in_channels - ic_base) : TILE_IC;
                    
                    LOAD_WT_OC: for (int oc = 0; oc < oc_valid; oc++) {
                    #pragma HLS LOOP_TRIPCOUNT min=1 max=16 avg=16
                        LOAD_WT_IC: for (int ic = 0; ic < ic_valid; ic++) {
                        #pragma HLS LOOP_TRIPCOUNT min=1 max=16 avg=16
                            for (int ky = 0; ky < kernel_size; ky++) {
                            #pragma HLS LOOP_TRIPCOUNT min=1 max=3 avg=3
                                for (int kx = 0; kx < kernel_size; kx++) {
                                    #pragma HLS LOOP_TRIPCOUNT min=1 max=3 avg=3
                                    #pragma HLS PIPELINE II=1
                                    int w_idx = ((((oc_base + oc) * in_channels) + (ic_base + ic)) * kernel_size + ky) * kernel_size + kx;
                                    wide_t w_pkg = weights_dram[w_idx / 16];
                                    weight_cache[oc][ic][ky][kx].range(15,0) = w_pkg.range((w_idx % 16)*16+15, (w_idx % 16)*16);
                                }
                            }
                        }
                    }

                    // 3. Stream Weights (Pack IC into vec_t, Order: OC -> Ky -> Kx)
                    // Done ONCE per tile
                    STREAM_WT: for (int oc = 0; oc < TILE_OC; oc++) {
                         for (int ky = 0; ky < kernel_size; ky++) {
                         #pragma HLS LOOP_TRIPCOUNT min=1 max=3 avg=3
                             for (int kx = 0; kx < kernel_size; kx++) {
                                 #pragma HLS LOOP_TRIPCOUNT min=1 max=3 avg=3
                                 #pragma HLS PIPELINE II=1
                                 vec_t w_vec;
                                 for (int ic = 0; ic < TILE_IC; ic++) {
                                     #pragma HLS UNROLL
                                     w_vec.range(ic*16+15, ic*16) = weight_cache[oc][ic][ky][kx].range(15,0);
                                 }
                                 weight_stream.write(w_vec);
                             }
                         }
                    }

                    // 4. Stream Inputs (K-Major Order)
                    // Outer Loops: Kernel Position (Ky, Kx)
                    // Inner Loops: Spatial Position (i, j)
                    // This aligns with K-Major execution to prevent RAW hazards
                    STREAM_IN_K: for (int ky = 0; ky < kernel_size; ky++) {
                    #pragma HLS LOOP_TRIPCOUNT min=1 max=3 avg=3
                        for (int kx = 0; kx < kernel_size; kx++) {
                        #pragma HLS LOOP_TRIPCOUNT min=1 max=3 avg=3
                            
                            STREAM_IN_SPATIAL: for (int i = 0; i < curr_h; i++) {
                            #pragma HLS LOOP_TRIPCOUNT min=1 max=16 avg=16
                                for (int j = 0; j < curr_w; j++) {
                                    #pragma HLS LOOP_TRIPCOUNT min=1 max=16 avg=16
                                    #pragma HLS PIPELINE II=1
                                    vec_t in_vec;
                                    for (int ic = 0; ic < TILE_IC; ic++) {
                                        #pragma HLS UNROLL
                                        int row = i * stride + ky;
                                        int col = j * stride + kx;
                                        in_vec.range(ic*16+15, ic*16) = input_cache[ic][row][col].range(15,0);
                                    }
                                    input_stream.write(in_vec);
                                }
                            }
                        }
                    }

                } 
            }
        }
    }
}

// ============================================================================
// STAGE 2: EXECUTE LAYER
// - Buffers Weights Unpacked
// - Buffers BN Params
// - Computes in K-Major Order (Ky, Kx outer -> i, j inner) to allow II=1
// ============================================================================
void Execute_Layer(
    hls::stream<vec_t>& input_stream,
    hls::stream<vec_t>& weight_stream,
    hls::stream<vec_t>& output_stream,
    data_t* bn_params,
    int in_channels, int out_channels,
    int out_height, int out_width,
    int kernel_size, int use_leaky
) {
    int tr_steps = (out_height + TILE_H - 1) / TILE_H;
    int tc_steps = (out_width + TILE_W - 1) / TILE_W;
    int to_steps = (out_channels + TILE_OC - 1) / TILE_OC;
    int ti_steps = (in_channels + TILE_IC - 1) / TILE_IC;

    // Accumulator Buffer
    // Partitioned: dim=1 complete (OC unroll), dim=3 complete (W unroll for port freedom)
    // Creates 16x16=256 small [TILE_H]-deep arrays -> LUTRAM, native 1R+1W per array
    acc_t acc_buf[TILE_OC][TILE_H][TILE_W];
    #pragma HLS ARRAY_PARTITION variable=acc_buf dim=1 complete
    #pragma HLS ARRAY_PARTITION variable=acc_buf dim=3 complete

    // Local Weight Buffer (Unpacked: TILE_OC x TILE_IC x K x K)
    data_t wt_buf[TILE_OC][TILE_IC][K_MAX][K_MAX];
    #pragma HLS ARRAY_PARTITION variable=wt_buf dim=1 complete
    #pragma HLS ARRAY_PARTITION variable=wt_buf dim=2 complete

    // Local BN Buffers
    data_t scale_buf[TILE_OC];
    data_t bias_buf[TILE_OC];
    #pragma HLS ARRAY_PARTITION variable=scale_buf complete
    #pragma HLS ARRAY_PARTITION variable=bias_buf complete

    EXEC_ROW: for (int tr = 0; tr < tr_steps; tr++) {
    #pragma HLS LOOP_TRIPCOUNT min=1 max=26 avg=8
        EXEC_COL: for (int tc = 0; tc < tc_steps; tc++) {
        #pragma HLS LOOP_TRIPCOUNT min=1 max=26 avg=8
            EXEC_OC: for (int to = 0; to < to_steps; to++) {
            #pragma HLS LOOP_TRIPCOUNT min=1 max=64 avg=16
                
                int r_start = tr * TILE_H;
                int c_start = tc * TILE_W;
                int curr_h = (r_start + TILE_H > out_height) ? out_height - r_start : TILE_H;
                int curr_w = (c_start + TILE_W > out_width) ? out_width - c_start : TILE_W;

                // 1. Load BN Params (Single read per cycle for II=1)
                LOAD_BN: for (int idx = 0; idx < TILE_OC * 2; idx++) {
                    #pragma HLS PIPELINE II=1
                    data_t val = bn_params[to * TILE_OC * 2 + idx];
                    if (idx & 1)
                        bias_buf[idx >> 1] = val;
                    else
                        scale_buf[idx >> 1] = val;
                }

                // 2. Clear Accumulators
                CLEAR_ACC_H: for (int i = 0; i < curr_h; i++) {
                #pragma HLS LOOP_TRIPCOUNT min=1 max=16 avg=16
                    CLEAR_ACC_W: for (int j = 0; j < curr_w; j++) {
                        #pragma HLS LOOP_TRIPCOUNT min=1 max=16 avg=16
                        #pragma HLS PIPELINE II=1
                        for (int oc = 0; oc < TILE_OC; oc++) acc_buf[oc][i][j] = 0;
                    }
                }

                // 3. Compute Loop
                EXEC_IC: for (int ti = 0; ti < ti_steps; ti++) {
                    #pragma HLS LOOP_TRIPCOUNT min=1 max=64 avg=16
                    
                    // A. Read Weights (Once per Tile)
                    // Unpack immediately into fully partitioned array
                    READ_WT: for (int oc = 0; oc < TILE_OC; oc++) {
                        for (int ky = 0; ky < kernel_size; ky++) {
                        #pragma HLS LOOP_TRIPCOUNT min=1 max=3 avg=3
                            for (int kx = 0; kx < kernel_size; kx++) {
                                #pragma HLS LOOP_TRIPCOUNT min=1 max=3 avg=3
                                #pragma HLS PIPELINE II=1
                                vec_t w_pkg = weight_stream.read();
                                for (int ic = 0; ic < TILE_IC; ic++) {
                                    #pragma HLS UNROLL
                                    wt_buf[oc][ic][ky][kx].range(15,0) = w_pkg.range(ic*16+15, ic*16);
                                }
                            }
                        }
                    }

                    // B. Compute (K-Major Order)
                    // Outer loop over Kernel to maximize reuse distance for Acc Buf
                    COMPUTE_K: for (int ky = 0; ky < kernel_size; ky++) {
                    #pragma HLS LOOP_TRIPCOUNT min=1 max=3 avg=3
                        for (int kx = 0; kx < kernel_size; kx++) {
                        #pragma HLS LOOP_TRIPCOUNT min=1 max=3 avg=3
                            #pragma HLS LOOP_FLATTEN off
                            // Inner Spatial Loop - Pipelined II=1
                            COMPUTE_SPATIAL: for (int i = 0; i < curr_h; i++) {
                            #pragma HLS LOOP_TRIPCOUNT min=1 max=16 avg=16
                                for (int j = 0; j < curr_w; j++) {
                                    #pragma HLS LOOP_TRIPCOUNT min=1 max=16 avg=16
                                    #pragma HLS PIPELINE II=1
                                    
                                    // 1. Read Input Vector
                                    vec_t in_pkg = input_stream.read();
                                    
                                    // 2. MAC Array (Fully Unrolled)
                                    for (int oc = 0; oc < TILE_OC; oc++) {
                                        #pragma HLS UNROLL
                                        acc_t dot_prod = 0;
                                        for (int ic = 0; ic < TILE_IC; ic++) {
                                            #pragma HLS UNROLL
                                            data_t w_val = wt_buf[oc][ic][ky][kx];
                                            data_t in_val;
                                            in_val.range(15,0) = in_pkg.range(ic*16+15, ic*16);
                                            dot_prod += w_val * in_val;
                                        }
                                        // 3. Update Accumulator
                                        // No Hazard: (i,j) changes every cycle. 
                                        // Next write to same (i,j) is in next K-loop iteration (hundreds of cycles later)
                                        acc_buf[oc][i][j] += dot_prod;
                                    }
                                }
                            }
                        }
                    }
                }

                // 4. Post-Process & Stream Out
                STREAM_OUT_H: for (int i = 0; i < curr_h; i++) {
                #pragma HLS LOOP_TRIPCOUNT min=1 max=16 avg=16
                    STREAM_OUT_W: for (int j = 0; j < curr_w; j++) {
                        #pragma HLS LOOP_TRIPCOUNT min=1 max=16 avg=16
                        #pragma HLS PIPELINE II=1
                        vec_t out_pkg;
                        for (int oc = 0; oc < TILE_OC; oc++) {
                            #pragma HLS UNROLL
                            // Apply BN + Activation
                            data_t res = activate(acc_buf[oc][i][j] * scale_buf[oc] + bias_buf[oc], use_leaky);
                            out_pkg.range(oc*16+15, oc*16) = res.range(15,0);
                        }
                        output_stream.write(out_pkg);
                    }
                }
            }
        }
    }
}

// ============================================================================
// STAGE 3: WRITE LAYER (Optimized for II=1 with word-aligned burst writes)
// ============================================================================
void Write_Layer(
    wide_t* output_dram,
    hls::stream<vec_t>& output_stream,
    int out_channels, int out_height, int out_width,
    int use_pool, int pool_stride
) {
    int tr_steps = (out_height + TILE_H - 1) / TILE_H;
    int tc_steps = (out_width  + TILE_W - 1) / TILE_W;
    int to_steps = (out_channels + TILE_OC - 1) / TILE_OC;

    // Tile buffer: partition dim=1 (OC) for parallel unpack, dim=3 (W) for row packing
    data_t tile_buf[TILE_OC][TILE_H][TILE_W];
    #pragma HLS ARRAY_PARTITION variable=tile_buf dim=1 complete
    #pragma HLS ARRAY_PARTITION variable=tile_buf dim=3 complete

    WB_ROW: for (int tr = 0; tr < tr_steps; tr++) {
    #pragma HLS LOOP_TRIPCOUNT min=1 max=26 avg=8
        WB_COL: for (int tc = 0; tc < tc_steps; tc++) {
        #pragma HLS LOOP_TRIPCOUNT min=1 max=26 avg=8
            WB_OC: for (int to = 0; to < to_steps; to++) {
            #pragma HLS LOOP_TRIPCOUNT min=1 max=64 avg=16

                int r_start = tr * TILE_H;
                int c_start = tc * TILE_W;
                int curr_h = (r_start + TILE_H > out_height) ? out_height - r_start : TILE_H;
                int curr_w = (c_start + TILE_W > out_width)  ? out_width  - c_start : TILE_W;
                int oc_limit = (to * TILE_OC + TILE_OC > out_channels) ? (out_channels - to * TILE_OC) : TILE_OC;

                // ============================================================
                // PHASE 1: Read Stream into tile_buf (fixed bounds for HLS)
                // ============================================================
                READ_STREAM_H: for (int i = 0; i < TILE_H; i++) {
                    READ_STREAM_W: for (int j = 0; j < TILE_W; j++) {
                        #pragma HLS PIPELINE II=1
                        if (i < curr_h && j < curr_w) {
                            vec_t out_pkg = output_stream.read();
                            READ_STREAM_OC: for (int oc = 0; oc < TILE_OC; oc++) {
                                #pragma HLS UNROLL
                                tile_buf[oc][i][j].range(15,0) = out_pkg.range(oc*16+15, oc*16);
                            }
                        }
                    }
                }

                // ============================================================
                // PHASE 2 & 3: Element-accurate write to DRAM
                // Handles non-16-aligned output widths correctly.
                // Each tile row maps to at most 2 DRAM words.
                // RMW (read-modify-write) is used when a row starts
                // mid-word (flat_index % 16 != 0) to preserve data
                // already written by adjacent rows/tiles.
                // ============================================================
                if (use_pool && pool_stride >= 2) {
                    int final_h = out_height / 2;
                    int final_w = out_width  / 2;
                    int ph = curr_h / 2;
                    int pw = curr_w / 2;

                    POOL_WR_OC: for (int oc = 0; oc < TILE_OC; oc++) {
                        POOL_WR_H: for (int i = 0; i < TILE_H/2; i++) {
                        #pragma HLS LOOP_TRIPCOUNT min=1 max=8 avg=8
                            if (oc < oc_limit && i < ph) {
                                int global_oc = to * TILE_OC + oc;
                                int out_r = (r_start / 2) + i;
                                int out_c_base = c_start / 2;
                                int base = (global_oc * final_h + out_r) * final_w + out_c_base;
                                int start_slot = base & 15;
                                int first_word_addr = base >> 4;
                                int first_count = 16 - start_slot;
                                if (first_count > pw) first_count = pw;
                                int second_count = pw - first_count;

                                // --- First DRAM word (RMW unless full 16-slot write) ---
                                wide_t word1 = (first_count < 16) ? output_dram[first_word_addr] : (wide_t)0;
                                POOL_PACK_W1: for (int j = 0; j < TILE_W/2; j++) {
                                    #pragma HLS PIPELINE II=1
                                    if (j < first_count) {
                                        data_t v0 = tile_buf[oc][i*2][j*2];
                                        data_t v1 = tile_buf[oc][i*2+1][j*2];
                                        data_t v2 = tile_buf[oc][i*2][j*2+1];
                                        data_t v3 = tile_buf[oc][i*2+1][j*2+1];
                                        data_t mx01 = (v0 > v1) ? v0 : v1;
                                        data_t mx23 = (v2 > v3) ? v2 : v3;
                                        data_t pool_val = (mx01 > mx23) ? mx01 : mx23;
                                        int slot = start_slot + j;
                                        word1.range(slot*16+15, slot*16) = pool_val.range(15, 0);
                                    }
                                }
                                output_dram[first_word_addr] = word1;

                                // --- Second DRAM word (if elements spill over) ---
                                if (second_count > 0) {
                                    wide_t word2 = (second_count < 16) ? output_dram[first_word_addr + 1] : (wide_t)0;
                                    POOL_PACK_W2: for (int j = 0; j < TILE_W/2; j++) {
                                        #pragma HLS PIPELINE II=1
                                        if (j < second_count) {
                                            int jj = first_count + j;
                                            data_t v0 = tile_buf[oc][i*2][jj*2];
                                            data_t v1 = tile_buf[oc][i*2+1][jj*2];
                                            data_t v2 = tile_buf[oc][i*2][jj*2+1];
                                            data_t v3 = tile_buf[oc][i*2+1][jj*2+1];
                                            data_t mx01 = (v0 > v1) ? v0 : v1;
                                            data_t mx23 = (v2 > v3) ? v2 : v3;
                                            data_t pool_val = (mx01 > mx23) ? mx01 : mx23;
                                            word2.range(j*16+15, j*16) = pool_val.range(15, 0);
                                        }
                                    }
                                    output_dram[first_word_addr + 1] = word2;
                                }
                            }
                        }
                    }
                } else {
                    // Direct write: element-accurate packing into DRAM words
                    DIRECT_WR_OC: for (int oc = 0; oc < TILE_OC; oc++) {
                        DIRECT_WR_H: for (int i = 0; i < TILE_H; i++) {
                        #pragma HLS LOOP_TRIPCOUNT min=1 max=16 avg=16
                            if (oc < oc_limit && i < curr_h) {
                                int global_oc = to * TILE_OC + oc;
                                int base = (global_oc * out_height + r_start + i) * out_width + c_start;
                                int start_slot = base & 15;
                                int first_word_addr = base >> 4;
                                int first_count = 16 - start_slot;
                                if (first_count > curr_w) first_count = curr_w;
                                int second_count = curr_w - first_count;

                                // --- First DRAM word (RMW unless full 16-slot write) ---
                                wide_t word1 = (first_count < 16) ? output_dram[first_word_addr] : (wide_t)0;
                                DIRECT_PACK_W1: for (int j = 0; j < TILE_W; j++) {
                                    #pragma HLS PIPELINE II=1
                                    if (j < first_count) {
                                        int slot = start_slot + j;
                                        word1.range(slot*16+15, slot*16) = tile_buf[oc][i][j].range(15, 0);
                                    }
                                }
                                output_dram[first_word_addr] = word1;

                                // --- Second DRAM word (if elements spill over) ---
                                if (second_count > 0) {
                                    wide_t word2 = (second_count < 16) ? output_dram[first_word_addr + 1] : (wide_t)0;
                                    DIRECT_PACK_W2: for (int j = 0; j < TILE_W; j++) {
                                        #pragma HLS PIPELINE II=1
                                        if (j < second_count) {
                                            word2.range(j*16+15, j*16) = tile_buf[oc][i][first_count + j].range(15, 0);
                                        }
                                    }
                                    output_dram[first_word_addr + 1] = word2;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// ============================================================================
// DATAFLOW CORE (canonical form: only declarations + function calls)
// ============================================================================
static void conv_dataflow(
    wide_t* input_dram,
    wide_t* output_dram,
    wide_t* weights_dram,
    data_t* bn_params_dram,
    int in_channels, int out_channels,
    int in_height, int in_width,
    int kernel_size, int stride, int padding,
    int use_pool, int pool_stride, int use_leaky,
    int out_height, int out_width
) {
    #pragma HLS DATAFLOW

    hls::stream<vec_t> input_stream("input_stream");
    hls::stream<vec_t> weight_stream("weight_stream");
    hls::stream<vec_t> output_stream("output_stream");

    #pragma HLS STREAM variable=input_stream depth=1024
    #pragma HLS STREAM variable=weight_stream depth=1024
    #pragma HLS STREAM variable=output_stream depth=1024

    Fetch_Layer(input_dram, weights_dram, input_stream, weight_stream,
                in_channels, out_channels, in_height, in_width,
                kernel_size, stride, padding, out_height, out_width);

    Execute_Layer(input_stream, weight_stream, output_stream, bn_params_dram,
                  in_channels, out_channels, out_height, out_width,
                  kernel_size, use_leaky);

    Write_Layer(output_dram, output_stream, out_channels, out_height, out_width,
                use_pool, pool_stride);
}

// ============================================================================
// TOP LEVEL
// ============================================================================
extern "C" void conv_engine(
    wide_t* input_dram,
    wide_t* output_dram,
    wide_t* weights_dram,
    data_t* bn_params_dram,
    int in_channels,
    int out_channels,
    int in_height,
    int in_width,
    int kernel_size,
    int stride,
    int padding,
    int use_pool,
    int pool_stride,
    int use_leaky
) {
    #pragma HLS INTERFACE m_axi port=input_dram bundle=gmem0 depth=1000000
    #pragma HLS INTERFACE m_axi port=output_dram bundle=gmem1 depth=1000000
    #pragma HLS INTERFACE m_axi port=weights_dram bundle=gmem2 depth=5000000
    #pragma HLS INTERFACE m_axi port=bn_params_dram bundle=gmem3 depth=4096

    #pragma HLS INTERFACE s_axilite port=input_dram bundle=control
    #pragma HLS INTERFACE s_axilite port=output_dram bundle=control
    #pragma HLS INTERFACE s_axilite port=weights_dram bundle=control
    #pragma HLS INTERFACE s_axilite port=bn_params_dram bundle=control

    #pragma HLS INTERFACE s_axilite port=in_channels bundle=control
    #pragma HLS INTERFACE s_axilite port=out_channels bundle=control
    #pragma HLS INTERFACE s_axilite port=in_height bundle=control
    #pragma HLS INTERFACE s_axilite port=in_width bundle=control
    #pragma HLS INTERFACE s_axilite port=kernel_size bundle=control
    #pragma HLS INTERFACE s_axilite port=stride bundle=control
    #pragma HLS INTERFACE s_axilite port=padding bundle=control
    #pragma HLS INTERFACE s_axilite port=use_pool bundle=control
    #pragma HLS INTERFACE s_axilite port=pool_stride bundle=control
    #pragma HLS INTERFACE s_axilite port=use_leaky bundle=control
    #pragma HLS INTERFACE s_axilite port=return bundle=control

    // Safety guard — compute outside the dataflow region
    if (kernel_size > K_MAX) return;

    // Pre-compute output dimensions outside the dataflow region
    int out_height = (in_height + 2 * padding - kernel_size) / stride + 1;
    int out_width  = (in_width  + 2 * padding - kernel_size) / stride + 1;

    // Single function call → clean dataflow inside
    conv_dataflow(input_dram, output_dram, weights_dram, bn_params_dram,
                  in_channels, out_channels, in_height, in_width,
                  kernel_size, stride, padding,
                  use_pool, pool_stride, use_leaky,
                  out_height, out_width);
}