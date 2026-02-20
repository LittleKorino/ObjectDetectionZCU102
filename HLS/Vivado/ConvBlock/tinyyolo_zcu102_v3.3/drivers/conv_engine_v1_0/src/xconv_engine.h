// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2024.2 (64-bit)
// Tool Version Limit: 2024.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
#ifndef XCONV_ENGINE_H
#define XCONV_ENGINE_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#ifndef __linux__
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_io.h"
#else
#include <stdint.h>
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stddef.h>
#endif
#include "xconv_engine_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
#else
typedef struct {
#ifdef SDT
    char *Name;
#else
    u16 DeviceId;
#endif
    u64 Control_BaseAddress;
} XConv_engine_Config;
#endif

typedef struct {
    u64 Control_BaseAddress;
    u32 IsReady;
} XConv_engine;

typedef u32 word_type;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XConv_engine_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XConv_engine_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XConv_engine_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XConv_engine_ReadReg(BaseAddress, RegOffset) \
    *(volatile u32*)((BaseAddress) + (RegOffset))

#define Xil_AssertVoid(expr)    assert(expr)
#define Xil_AssertNonvoid(expr) assert(expr)

#define XST_SUCCESS             0
#define XST_DEVICE_NOT_FOUND    2
#define XST_OPEN_DEVICE_FAILED  3
#define XIL_COMPONENT_IS_READY  1
#endif

/************************** Function Prototypes *****************************/
#ifndef __linux__
#ifdef SDT
int XConv_engine_Initialize(XConv_engine *InstancePtr, UINTPTR BaseAddress);
XConv_engine_Config* XConv_engine_LookupConfig(UINTPTR BaseAddress);
#else
int XConv_engine_Initialize(XConv_engine *InstancePtr, u16 DeviceId);
XConv_engine_Config* XConv_engine_LookupConfig(u16 DeviceId);
#endif
int XConv_engine_CfgInitialize(XConv_engine *InstancePtr, XConv_engine_Config *ConfigPtr);
#else
int XConv_engine_Initialize(XConv_engine *InstancePtr, const char* InstanceName);
int XConv_engine_Release(XConv_engine *InstancePtr);
#endif

void XConv_engine_Start(XConv_engine *InstancePtr);
u32 XConv_engine_IsDone(XConv_engine *InstancePtr);
u32 XConv_engine_IsIdle(XConv_engine *InstancePtr);
u32 XConv_engine_IsReady(XConv_engine *InstancePtr);
void XConv_engine_EnableAutoRestart(XConv_engine *InstancePtr);
void XConv_engine_DisableAutoRestart(XConv_engine *InstancePtr);

void XConv_engine_Set_input_dram(XConv_engine *InstancePtr, u64 Data);
u64 XConv_engine_Get_input_dram(XConv_engine *InstancePtr);
void XConv_engine_Set_output_dram(XConv_engine *InstancePtr, u64 Data);
u64 XConv_engine_Get_output_dram(XConv_engine *InstancePtr);
void XConv_engine_Set_weights_dram(XConv_engine *InstancePtr, u64 Data);
u64 XConv_engine_Get_weights_dram(XConv_engine *InstancePtr);
void XConv_engine_Set_bn_params_dram(XConv_engine *InstancePtr, u64 Data);
u64 XConv_engine_Get_bn_params_dram(XConv_engine *InstancePtr);
void XConv_engine_Set_in_channels(XConv_engine *InstancePtr, u32 Data);
u32 XConv_engine_Get_in_channels(XConv_engine *InstancePtr);
void XConv_engine_Set_out_channels(XConv_engine *InstancePtr, u32 Data);
u32 XConv_engine_Get_out_channels(XConv_engine *InstancePtr);
void XConv_engine_Set_in_height(XConv_engine *InstancePtr, u32 Data);
u32 XConv_engine_Get_in_height(XConv_engine *InstancePtr);
void XConv_engine_Set_in_width(XConv_engine *InstancePtr, u32 Data);
u32 XConv_engine_Get_in_width(XConv_engine *InstancePtr);
void XConv_engine_Set_kernel_size(XConv_engine *InstancePtr, u32 Data);
u32 XConv_engine_Get_kernel_size(XConv_engine *InstancePtr);
void XConv_engine_Set_stride(XConv_engine *InstancePtr, u32 Data);
u32 XConv_engine_Get_stride(XConv_engine *InstancePtr);
void XConv_engine_Set_padding(XConv_engine *InstancePtr, u32 Data);
u32 XConv_engine_Get_padding(XConv_engine *InstancePtr);
void XConv_engine_Set_use_pool(XConv_engine *InstancePtr, u32 Data);
u32 XConv_engine_Get_use_pool(XConv_engine *InstancePtr);
void XConv_engine_Set_pool_stride(XConv_engine *InstancePtr, u32 Data);
u32 XConv_engine_Get_pool_stride(XConv_engine *InstancePtr);
void XConv_engine_Set_use_leaky(XConv_engine *InstancePtr, u32 Data);
u32 XConv_engine_Get_use_leaky(XConv_engine *InstancePtr);

void XConv_engine_InterruptGlobalEnable(XConv_engine *InstancePtr);
void XConv_engine_InterruptGlobalDisable(XConv_engine *InstancePtr);
void XConv_engine_InterruptEnable(XConv_engine *InstancePtr, u32 Mask);
void XConv_engine_InterruptDisable(XConv_engine *InstancePtr, u32 Mask);
void XConv_engine_InterruptClear(XConv_engine *InstancePtr, u32 Mask);
u32 XConv_engine_InterruptGetEnabled(XConv_engine *InstancePtr);
u32 XConv_engine_InterruptGetStatus(XConv_engine *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
