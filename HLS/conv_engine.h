/**
 * conv_engine.h
 * Shared definitions for CNN Accelerator
 */
#ifndef CONV_ENGINE_H
#define CONV_ENGINE_H

#include <hls_stream.h>
#include <ap_fixed.h>
#include <ap_int.h>

// ============================================================================
// CONFIGURATION
// ============================================================================
#define TILE_H 16
#define TILE_W 16
#define TILE_OC 16
#define TILE_IC 16
#define K_MAX 3

// ============================================================================
// TYPES
// ============================================================================
// Data: 16-bit Fixed Point Q8.8
typedef ap_fixed<16, 8, AP_RND, AP_SAT> data_t;

// Accumulator: 32-bit Fixed Point Q16.16 (Prevent overflow)
typedef ap_fixed<32, 16, AP_RND, AP_SAT> acc_t;

// DRAM Interface: 256-bit Words
typedef ap_int<256> wide_t;

// Vector Stream: 256-bit Unsigned (Holds 16 x 16-bit elements)
typedef ap_uint<256> vec_t;

// Cache Sizing (Support Stride up to 2)
#define MAX_STRIDE 2
#define CACHE_H (TILE_H * MAX_STRIDE + K_MAX - 1)
#define CACHE_W (TILE_W * MAX_STRIDE + K_MAX - 1)

// ============================================================================
// PROTOTYPES
// ============================================================================
extern "C" void conv_engine(
    wide_t* input_dram,
    wide_t* output_dram,
    wide_t* weights_dram,
    data_t* bn_params_dram,
    int in_channels, int out_channels,
    int in_height, int in_width,
    int kernel_size, int stride, int padding,
    int use_pool, int pool_stride,
    int use_leaky
);

#endif