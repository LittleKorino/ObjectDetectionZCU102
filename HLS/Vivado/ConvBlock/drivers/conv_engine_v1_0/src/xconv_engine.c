// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2024.2 (64-bit)
// Tool Version Limit: 2024.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
/***************************** Include Files *********************************/
#include "xconv_engine.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XConv_engine_CfgInitialize(XConv_engine *InstancePtr, XConv_engine_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Control_BaseAddress = ConfigPtr->Control_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XConv_engine_Start(XConv_engine *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XConv_engine_ReadReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_AP_CTRL) & 0x80;
    XConv_engine_WriteReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XConv_engine_IsDone(XConv_engine *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XConv_engine_ReadReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XConv_engine_IsIdle(XConv_engine *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XConv_engine_ReadReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XConv_engine_IsReady(XConv_engine *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XConv_engine_ReadReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XConv_engine_EnableAutoRestart(XConv_engine *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XConv_engine_WriteReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_AP_CTRL, 0x80);
}

void XConv_engine_DisableAutoRestart(XConv_engine *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XConv_engine_WriteReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_AP_CTRL, 0);
}

void XConv_engine_Set_input_r(XConv_engine *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XConv_engine_WriteReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_INPUT_R_DATA, (u32)(Data));
    XConv_engine_WriteReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_INPUT_R_DATA + 4, (u32)(Data >> 32));
}

u64 XConv_engine_Get_input_r(XConv_engine *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XConv_engine_ReadReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_INPUT_R_DATA);
    Data += (u64)XConv_engine_ReadReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_INPUT_R_DATA + 4) << 32;
    return Data;
}

void XConv_engine_Set_output_r(XConv_engine *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XConv_engine_WriteReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_OUTPUT_R_DATA, (u32)(Data));
    XConv_engine_WriteReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_OUTPUT_R_DATA + 4, (u32)(Data >> 32));
}

u64 XConv_engine_Get_output_r(XConv_engine *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XConv_engine_ReadReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_OUTPUT_R_DATA);
    Data += (u64)XConv_engine_ReadReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_OUTPUT_R_DATA + 4) << 32;
    return Data;
}

void XConv_engine_Set_weights(XConv_engine *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XConv_engine_WriteReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_WEIGHTS_DATA, (u32)(Data));
    XConv_engine_WriteReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_WEIGHTS_DATA + 4, (u32)(Data >> 32));
}

u64 XConv_engine_Get_weights(XConv_engine *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XConv_engine_ReadReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_WEIGHTS_DATA);
    Data += (u64)XConv_engine_ReadReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_WEIGHTS_DATA + 4) << 32;
    return Data;
}

void XConv_engine_Set_bn_params(XConv_engine *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XConv_engine_WriteReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_BN_PARAMS_DATA, (u32)(Data));
    XConv_engine_WriteReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_BN_PARAMS_DATA + 4, (u32)(Data >> 32));
}

u64 XConv_engine_Get_bn_params(XConv_engine *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XConv_engine_ReadReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_BN_PARAMS_DATA);
    Data += (u64)XConv_engine_ReadReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_BN_PARAMS_DATA + 4) << 32;
    return Data;
}

void XConv_engine_Set_in_channels(XConv_engine *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XConv_engine_WriteReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_IN_CHANNELS_DATA, Data);
}

u32 XConv_engine_Get_in_channels(XConv_engine *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XConv_engine_ReadReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_IN_CHANNELS_DATA);
    return Data;
}

void XConv_engine_Set_out_channels(XConv_engine *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XConv_engine_WriteReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_OUT_CHANNELS_DATA, Data);
}

u32 XConv_engine_Get_out_channels(XConv_engine *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XConv_engine_ReadReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_OUT_CHANNELS_DATA);
    return Data;
}

void XConv_engine_Set_in_height(XConv_engine *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XConv_engine_WriteReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_IN_HEIGHT_DATA, Data);
}

u32 XConv_engine_Get_in_height(XConv_engine *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XConv_engine_ReadReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_IN_HEIGHT_DATA);
    return Data;
}

void XConv_engine_Set_in_width(XConv_engine *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XConv_engine_WriteReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_IN_WIDTH_DATA, Data);
}

u32 XConv_engine_Get_in_width(XConv_engine *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XConv_engine_ReadReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_IN_WIDTH_DATA);
    return Data;
}

void XConv_engine_Set_kernel_size(XConv_engine *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XConv_engine_WriteReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_KERNEL_SIZE_DATA, Data);
}

u32 XConv_engine_Get_kernel_size(XConv_engine *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XConv_engine_ReadReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_KERNEL_SIZE_DATA);
    return Data;
}

void XConv_engine_Set_stride(XConv_engine *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XConv_engine_WriteReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_STRIDE_DATA, Data);
}

u32 XConv_engine_Get_stride(XConv_engine *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XConv_engine_ReadReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_STRIDE_DATA);
    return Data;
}

void XConv_engine_Set_padding(XConv_engine *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XConv_engine_WriteReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_PADDING_DATA, Data);
}

u32 XConv_engine_Get_padding(XConv_engine *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XConv_engine_ReadReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_PADDING_DATA);
    return Data;
}

void XConv_engine_Set_use_pool(XConv_engine *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XConv_engine_WriteReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_USE_POOL_DATA, Data);
}

u32 XConv_engine_Get_use_pool(XConv_engine *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XConv_engine_ReadReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_USE_POOL_DATA);
    return Data;
}

void XConv_engine_Set_use_relu(XConv_engine *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XConv_engine_WriteReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_USE_RELU_DATA, Data);
}

u32 XConv_engine_Get_use_relu(XConv_engine *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XConv_engine_ReadReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_USE_RELU_DATA);
    return Data;
}

void XConv_engine_InterruptGlobalEnable(XConv_engine *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XConv_engine_WriteReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_GIE, 1);
}

void XConv_engine_InterruptGlobalDisable(XConv_engine *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XConv_engine_WriteReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_GIE, 0);
}

void XConv_engine_InterruptEnable(XConv_engine *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XConv_engine_ReadReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_IER);
    XConv_engine_WriteReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_IER, Register | Mask);
}

void XConv_engine_InterruptDisable(XConv_engine *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XConv_engine_ReadReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_IER);
    XConv_engine_WriteReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_IER, Register & (~Mask));
}

void XConv_engine_InterruptClear(XConv_engine *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XConv_engine_WriteReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_ISR, Mask);
}

u32 XConv_engine_InterruptGetEnabled(XConv_engine *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XConv_engine_ReadReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_IER);
}

u32 XConv_engine_InterruptGetStatus(XConv_engine *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XConv_engine_ReadReg(InstancePtr->Control_BaseAddress, XCONV_ENGINE_CONTROL_ADDR_ISR);
}

