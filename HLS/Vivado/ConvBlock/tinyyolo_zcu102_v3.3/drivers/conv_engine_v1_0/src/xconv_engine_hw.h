// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2024.2 (64-bit)
// Tool Version Limit: 2024.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
// control
// 0x00 : Control signals
//        bit 0  - ap_start (Read/Write/COH)
//        bit 1  - ap_done (Read/COR)
//        bit 2  - ap_idle (Read)
//        bit 3  - ap_ready (Read/COR)
//        bit 7  - auto_restart (Read/Write)
//        bit 9  - interrupt (Read)
//        others - reserved
// 0x04 : Global Interrupt Enable Register
//        bit 0  - Global Interrupt Enable (Read/Write)
//        others - reserved
// 0x08 : IP Interrupt Enable Register (Read/Write)
//        bit 0 - enable ap_done interrupt (Read/Write)
//        bit 1 - enable ap_ready interrupt (Read/Write)
//        others - reserved
// 0x0c : IP Interrupt Status Register (Read/TOW)
//        bit 0 - ap_done (Read/TOW)
//        bit 1 - ap_ready (Read/TOW)
//        others - reserved
// 0x10 : Data signal of input_dram
//        bit 31~0 - input_dram[31:0] (Read/Write)
// 0x14 : Data signal of input_dram
//        bit 31~0 - input_dram[63:32] (Read/Write)
// 0x18 : reserved
// 0x1c : Data signal of output_dram
//        bit 31~0 - output_dram[31:0] (Read/Write)
// 0x20 : Data signal of output_dram
//        bit 31~0 - output_dram[63:32] (Read/Write)
// 0x24 : reserved
// 0x28 : Data signal of weights_dram
//        bit 31~0 - weights_dram[31:0] (Read/Write)
// 0x2c : Data signal of weights_dram
//        bit 31~0 - weights_dram[63:32] (Read/Write)
// 0x30 : reserved
// 0x34 : Data signal of bn_params_dram
//        bit 31~0 - bn_params_dram[31:0] (Read/Write)
// 0x38 : Data signal of bn_params_dram
//        bit 31~0 - bn_params_dram[63:32] (Read/Write)
// 0x3c : reserved
// 0x40 : Data signal of in_channels
//        bit 31~0 - in_channels[31:0] (Read/Write)
// 0x44 : reserved
// 0x48 : Data signal of out_channels
//        bit 31~0 - out_channels[31:0] (Read/Write)
// 0x4c : reserved
// 0x50 : Data signal of in_height
//        bit 31~0 - in_height[31:0] (Read/Write)
// 0x54 : reserved
// 0x58 : Data signal of in_width
//        bit 31~0 - in_width[31:0] (Read/Write)
// 0x5c : reserved
// 0x60 : Data signal of kernel_size
//        bit 31~0 - kernel_size[31:0] (Read/Write)
// 0x64 : reserved
// 0x68 : Data signal of stride
//        bit 31~0 - stride[31:0] (Read/Write)
// 0x6c : reserved
// 0x70 : Data signal of padding
//        bit 31~0 - padding[31:0] (Read/Write)
// 0x74 : reserved
// 0x78 : Data signal of use_pool
//        bit 31~0 - use_pool[31:0] (Read/Write)
// 0x7c : reserved
// 0x80 : Data signal of pool_stride
//        bit 31~0 - pool_stride[31:0] (Read/Write)
// 0x84 : reserved
// 0x88 : Data signal of use_leaky
//        bit 31~0 - use_leaky[31:0] (Read/Write)
// 0x8c : reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XCONV_ENGINE_CONTROL_ADDR_AP_CTRL             0x00
#define XCONV_ENGINE_CONTROL_ADDR_GIE                 0x04
#define XCONV_ENGINE_CONTROL_ADDR_IER                 0x08
#define XCONV_ENGINE_CONTROL_ADDR_ISR                 0x0c
#define XCONV_ENGINE_CONTROL_ADDR_INPUT_DRAM_DATA     0x10
#define XCONV_ENGINE_CONTROL_BITS_INPUT_DRAM_DATA     64
#define XCONV_ENGINE_CONTROL_ADDR_OUTPUT_DRAM_DATA    0x1c
#define XCONV_ENGINE_CONTROL_BITS_OUTPUT_DRAM_DATA    64
#define XCONV_ENGINE_CONTROL_ADDR_WEIGHTS_DRAM_DATA   0x28
#define XCONV_ENGINE_CONTROL_BITS_WEIGHTS_DRAM_DATA   64
#define XCONV_ENGINE_CONTROL_ADDR_BN_PARAMS_DRAM_DATA 0x34
#define XCONV_ENGINE_CONTROL_BITS_BN_PARAMS_DRAM_DATA 64
#define XCONV_ENGINE_CONTROL_ADDR_IN_CHANNELS_DATA    0x40
#define XCONV_ENGINE_CONTROL_BITS_IN_CHANNELS_DATA    32
#define XCONV_ENGINE_CONTROL_ADDR_OUT_CHANNELS_DATA   0x48
#define XCONV_ENGINE_CONTROL_BITS_OUT_CHANNELS_DATA   32
#define XCONV_ENGINE_CONTROL_ADDR_IN_HEIGHT_DATA      0x50
#define XCONV_ENGINE_CONTROL_BITS_IN_HEIGHT_DATA      32
#define XCONV_ENGINE_CONTROL_ADDR_IN_WIDTH_DATA       0x58
#define XCONV_ENGINE_CONTROL_BITS_IN_WIDTH_DATA       32
#define XCONV_ENGINE_CONTROL_ADDR_KERNEL_SIZE_DATA    0x60
#define XCONV_ENGINE_CONTROL_BITS_KERNEL_SIZE_DATA    32
#define XCONV_ENGINE_CONTROL_ADDR_STRIDE_DATA         0x68
#define XCONV_ENGINE_CONTROL_BITS_STRIDE_DATA         32
#define XCONV_ENGINE_CONTROL_ADDR_PADDING_DATA        0x70
#define XCONV_ENGINE_CONTROL_BITS_PADDING_DATA        32
#define XCONV_ENGINE_CONTROL_ADDR_USE_POOL_DATA       0x78
#define XCONV_ENGINE_CONTROL_BITS_USE_POOL_DATA       32
#define XCONV_ENGINE_CONTROL_ADDR_POOL_STRIDE_DATA    0x80
#define XCONV_ENGINE_CONTROL_BITS_POOL_STRIDE_DATA    32
#define XCONV_ENGINE_CONTROL_ADDR_USE_LEAKY_DATA      0x88
#define XCONV_ENGINE_CONTROL_BITS_USE_LEAKY_DATA      32

