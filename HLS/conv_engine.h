/**
 * conv_engine_v3.h — "Verilog-in-C++" Cache & DMA Architecture
 *
 * === Philosophy ===
 * Treat HLS C++ like a hardware description language.
 * Every buffer is an explicit BRAM instantiation.
 * Every DRAM access goes through a DMA staging buffer.
 * No implicit type conversions — all bit manipulation via .range().
 *
 * === Key Changes from V2 ===
 *  1. REMOVED big_input[1024] — caused 61B-cycle serialization
 *  2. Explicit DMA staging buffers for burst read/write (Verilog-style)
 *  3. Phase-separated output writes: read-edge → pack → burst-write
 *  4. Clock target 5ns (200 MHz) to fix timing violations
 *  5. Wider weight burst: read 16 IC × K×K per OC in one shot
 *  6. TILE_OC/TILE_IC = 16: 256 MACs/cycle (51.2 GMAC/s)
 *  7. vec_t 256-bit for 16-element stream packing
 */
#ifndef CONV_ENGINE_H
#define CONV_ENGINE_H

#include <hls_stream.h>
#include <ap_fixed.h>
#include <ap_int.h>

/* =========================================================================
 * TILE CONFIGURATION
 * ========================================================================= */
#define TILE_H  16
#define TILE_W  16
#define TILE_OC 16
#define TILE_IC 16
#define K_MAX   3

/* =========================================================================
 * DATA TYPES — fixed bit widths, no implicit conversion
 * Verilog analogy: these are wire/reg bit vectors
 * ========================================================================= */
typedef ap_fixed<16, 8, AP_RND, AP_SAT>  data_t;   /* 16-bit activation/weight  */
typedef ap_fixed<32, 16, AP_RND, AP_SAT> acc_t;    /* 32-bit MAC accumulator    */
typedef ap_int<128>   wide_t;                        /* 128-bit DRAM word (m_axi) */
typedef ap_uint<256>  vec_t;                         /* 256-bit stream word (16×16b) */

/* =========================================================================
 * DERIVED CONSTANTS — computed at compile time (like Verilog parameters)
 * ========================================================================= */
#define ELEMS_PER_WORD  8                            /* 128 / 16 bits             */
#define WORD_SHIFT      3                            /* log2(ELEMS_PER_WORD)       */
#define WORD_MASK       (ELEMS_PER_WORD - 1)
#define MAX_STRIDE      2
#define CACHE_H  (TILE_H * MAX_STRIDE + K_MAX - 1)  /* 35                        */
#define CACHE_W  (TILE_W * MAX_STRIDE + K_MAX - 1)  /* 35                        */

/* Max OC tile steps across all layers (1024/16 = 64 for conv7) */
#define MAX_OC_STEPS    64

/* =========================================================================
 * DMA STAGING BUFFER SIZES (Verilog-like: explicit FIFO/buffer sizing)
 *
 * These are the hardware buffers that sit between DRAM (AXI) and
 * the on-chip BRAM caches. In Verilog you'd instantiate these as
 * separate AXI read/write channel FIFOs.
 * ========================================================================= */

/* Input line DMA: burst-reads one full row of the input tile.
 * An unaligned 35-element row can span up to 6 128-bit words.               */
#define DMA_LINE_WORDS  6

/* Weight block DMA: burst-reads all weights for one OC channel × all IC
 * 16 IC × K_MAX² = 16×9 = 144 elements = 18 aligned words.
 * An unaligned block can span up to 19 128-bit words.                       */
#define DMA_WT_WORDS    19

/* Output row DMA: a TILE_W=16 row can span up to 3 unaligned words.         */
#define DMA_OUT_WORDS   3

/* =========================================================================
 * TOP-LEVEL PROTOTYPE
 * ========================================================================= */
extern "C" void conv_engine(
    wide_t* input_dram,
    wide_t* output_dram,
    wide_t* weights_dram,
    data_t* bn_params_dram,
    int in_channels, int out_channels,
    int in_height,   int in_width,
    int kernel_size,  int stride, int padding,
    int use_pool,     int pool_stride,
    int use_leaky
);

#endif /* CONV_ENGINE_V3_H */
