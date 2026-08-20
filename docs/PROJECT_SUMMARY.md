# Project Implementation Summary — FPGA-Accelerated Tiny-YOLO on ZCU102

**Repository:** `ObjectDetectionZCU102`
**Target device:** Xilinx Zynq UltraScale+ MPSoC ZCU102 (`xczu9eg-ffvb1156-2-e`)
**Toolchain:** Vitis HLS / Vivado 2024.1–2024.2, PYNQ 3.1.2, PyTorch
**Headline result:** all 10 convolutional layers of Tiny-YOLO v2 executed on programmable logic at 167 MHz, 277 ms hardware time per frame (~2.7 FPS wall clock), 11.6× faster than the first working version.

---

## 1. What the project is

An end-to-end hardware/software co-design for CNN object detection on an FPGA SoC. Nothing is taken from a vendor DPU or Vitis AI — the convolution accelerator is written from scratch in C++ for Vitis HLS, integrated in a Vivado block design, and driven from Python on the ARM cores through PYNQ.

The work is split across five stages, each of which lives in its own top-level directory:

| Stage | Directory | Contents |
|---|---|---|
| 0. Operating system | `OS/` | Notes for building a custom PYNQ 3.1.2 SD image for ZCU102 with PetaLinux |
| 1. Model training | `Training/`, `Models/` | `ObjectDetection.ipynb` (PyTorch, COCO 2017), two `.pth` checkpoints |
| 2. Software reference | `PS/` | `run_yolo_opencv.py` — float32 CPU inference used as a golden reference |
| 3. Accelerator design | `HLS/` | `conv_engine.{h,cpp}`, testbench, `run_hls.tcl` |
| 4. System integration | `HLS/Vivado/ConvBlock/` | Block design, bitstream `tinyyolo_zcu102_v3.3.bit`, `.hwh`, `.xsa` |
| 5. Deployment | `PL/` | `tinyyolo_pynq.ipynb` — full on-board inference pipeline |

### Network

Tiny-YOLO v2: nine convolution blocks plus a 1×1 detection head. Input 416×416×3, output grid 13×13 with 5 anchors × (5 + 80 classes) = 425 channels. Trained from scratch on COCO 2017 (118K train / 5K val) with CIoU loss, label smoothing, AdamW with OneCycle scheduling, and mixed precision on an RTX 4090. Best validation IoU reached roughly 0.52.

---

## 2. The HLS implementation (core of the project)

All hardware is generated from four files in `HLS/`. The design philosophy stated in the header is *"treat HLS C++ like a hardware description language"* — every buffer is an explicit BRAM instantiation, every DRAM access passes through a DMA staging buffer, and all bit manipulation is done through `.range()` rather than implicit conversion.

### 2.1 Top-level kernel and interfaces

One call to `conv_engine()` executes **one convolution layer**. The kernel is fully runtime-parameterised, so the same bitstream runs all ten layers back to back — the PS simply rewrites the AXI-Lite registers and re-pulses `ap_start`.

```795:834:HLS/conv_engine.cpp
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
    #pragma HLS INTERFACE m_axi port=input_dram    bundle=gmem0 depth=1000000 \
        max_read_burst_length=64
    // ... gmem1 (output, R/W), gmem2 (weights), gmem3 (BN params) ...
    #pragma HLS INTERFACE s_axilite port=return        bundle=control
```

Four independent AXI4 master bundles let input, output, weight and BN traffic proceed concurrently instead of contending on a single port:

| Bundle | Port | Width | Direction | Payload |
|---|---|---|---|---|
| `gmem0` | `input_dram` | 256-bit | read | input feature map |
| `gmem1` | `output_dram` | 256-bit | read/write | output feature map |
| `gmem2` | `weights_dram` | 256-bit | read | convolution weights |
| `gmem3` | `bn_params_dram` | 16-bit | read | fused BatchNorm scale/bias |
| `control` | AXI-Lite | 32-bit | — | base addresses and all scalar parameters |

Maximum burst length is 64 beats, i.e. 64 × 256 bit = 2 KB per AXI transaction.

### 2.2 Number formats

```39:42:HLS/conv_engine.h
typedef ap_fixed<16, 8, AP_RND, AP_SAT>  data_t;   /* 16-bit activation/weight  */
typedef ap_fixed<32, 16, AP_RND, AP_SAT> acc_t;    /* 32-bit MAC accumulator    */
typedef ap_int<256>   wide_t;                        /* 256-bit DRAM word (m_axi) */
typedef ap_uint<256>  vec_t;                         /* 256-bit stream word (16×16b) */
```

Activations and weights are Q8.8 fixed point with rounding and saturation; accumulation happens in Q16.16 to avoid overflow across a 3×3×512 reduction. A 256-bit memory word therefore carries exactly 16 values, which is why every tile dimension in the design is 16.

### 2.3 Three-stage dataflow architecture

```749:781:HLS/conv_engine.cpp
static void conv_dataflow(...)
{
    #pragma HLS DATAFLOW
    hls::stream<vec_t> input_stream("input_stream");
    hls::stream<vec_t> weight_stream("weight_stream");
    hls::stream<vec_t> output_stream("output_stream");
    #pragma HLS STREAM variable=input_stream  depth=8192
    #pragma HLS STREAM variable=weight_stream depth=4096
    #pragma HLS STREAM variable=output_stream depth=4096

    Fetch_Layer(...);
    Execute_Layer(...);
    Write_Layer(...);
}
```

```
DDR ──▶ Fetch_Layer ──[input_stream, weight_stream]──▶ Execute_Layer ──[output_stream]──▶ Write_Layer ──▶ DDR
         burst DMA                                     256-MAC + BN + activation           maxpool + burst DMA
```

The three functions run concurrently as separate hardware processes connected by FIFOs, so DMA latency for tile *n+1* is hidden behind the arithmetic of tile *n*.

**Fetch_Layer** walks tiles in ROW → COL → IC → OC order, burst-reads input rows into `dma_line[4]` and weights into `dma_wt[12]`, unpacks them into on-chip caches (`input_cache[16][35][35]`, `weight_cache[16][16][3][3]`), and pushes 256-bit packed words into the two input FIFOs. The 35 in the cache dimension is `TILE × MAX_STRIDE + K_MAX − 1`, i.e. the halo needed for a 3×3 kernel at stride 2.

**Execute_Layer** is the arithmetic core. The inner loop issues 16 output channels × 16 input channels = **256 multiply-accumulates every cycle at II = 1**, all bound to DSP48E2 slices:

```457:471:HLS/conv_engine.cpp
MAC_OC: for (int oc = 0; oc < TILE_OC; oc++) {
    #pragma HLS UNROLL
    acc_t dot = 0;
    MAC_IC: for (int ic = 0; ic < TILE_IC; ic++) {
        #pragma HLS UNROLL
        data_t w_val = wt_buf[oc][ic][ky][kx];
        data_t in_val;
        in_val.range(15, 0) = in_pkg.range(ic*16+15, ic*16);
        acc_t prod = (acc_t)(w_val * in_val);
        #pragma HLS BIND_OP variable=prod op=mul impl=dsp
        dot += prod;
    }
    acc_buf[oc][i][j] += dot;
}
```

At 167 MHz that is a peak of 256 × 2 × 167 M ≈ **85 GOP/s**. BatchNorm is folded into the same stage as a single multiply-add on the accumulator, followed by LeakyReLU (implemented as `x × 13 >> 7 ≈ 0.1x`), ReLU, or a linear pass-through for the detection head.

**Write_Layer** performs optional 2×2 max-pooling and writes results back. Its critical trick is phase separation — read the DRAM edge words, pack in registers, then issue one clean burst write — because interleaved read-modify-write defeated burst inference in earlier versions.

### 2.4 Loop ordering and the partial-sum buffer

The single most important optimisation is **IC-outer / OC-inner** loop ordering. With input channels on the outside, an input tile is fetched once and reused across every output-channel tile, eliminating up to 64× redundant DRAM reads for the widest layer (conv7, 1024 output channels). The cost is that partial sums must survive between input-channel tiles, which is what `psum_buf` is for:

```345:347:HLS/conv_engine.cpp
acc_t psum_buf[MAX_OC_STEPS][TILE_OC][TILE_H][TILE_W];
#pragma HLS ARRAY_PARTITION variable=psum_buf dim=4 complete
#pragma HLS BIND_STORAGE variable=psum_buf type=ram_2p impl=bram
```

That is 64 × 16 × 16 × 16 = 262 144 accumulator entries, roughly 512 BRAM18K blocks. It is partitioned along the width dimension so all 16 columns load and store in parallel at II = 1. On the first input-channel tile the accumulator is cleared, on intermediate tiles it is restored from and saved back to `psum_buf`, and only on the last tile is BatchNorm, activation and streaming-out performed.

### 2.5 Pragma inventory

| Pragma | Where and why |
|---|---|
| `DATAFLOW` | `conv_dataflow()` — three concurrent stages |
| `STREAM depth=8192/4096` | inter-stage FIFOs sized to hold a full tile |
| `PIPELINE II=1` | every DMA, unpack, MAC, BN and pack loop |
| `UNROLL` | IC and OC dimensions of the MAC tree (the 256-way parallelism) |
| `ARRAY_PARTITION complete` | `acc_buf`, `wt_buf`, `scale_buf`, `bias_buf`, caches; `dim=4` on `psum_buf` |
| `BIND_STORAGE ram_2p impl=bram` | `input_cache`, `input_lcl`, `psum_buf` |
| `BIND_OP op=mul impl=dsp` | MAC products and the BN multiply |
| `LOOP_TRIPCOUNT` | all variable-bound loops, so synthesis reports usable latency estimates |
| `LOOP_FLATTEN off` | kernel ky/kx loops, to keep the pipeline structure explicit |
| `INTERFACE m_axi / s_axilite` | four memory bundles plus the control register bank |

### 2.6 Verification

`conv_engine_tb.cpp` contains a bit-accurate golden model (`conv_golden`, `pool_golden`) written in the same fixed-point types, and exercises six cases: tile-aligned 16×16, non-aligned 13×13, multi-tile 26×26, pooled variants, and LeakyReLU, with a tolerance of 0.05 per element. On hardware, layers 0–8 match the PyTorch float32 reference with correlation > 0.999; the detection layer shows a larger absolute deviation (max Δ 43.4, correlation 0.9991) because of accumulated Q8.8 error and the unbounded linear output.

### 2.7 Design evolution

| Version | Clock | Bursts | LUT | HW time | FPS | Outcome |
|---|---|---|---|---|---|---|
| V1 | 143 MHz | 0/4 ports | 74 % | 3 207 ms | 0.31 | Naive DRAM access, 1.8 % compute utilisation |
| V2 | 250 MHz | partial | 85 % | — | — | Timing failed (−2.05 ns); a `big_input[1024]` array serialised the pipeline to 61 billion cycles |
| **V3** | **167 MHz** | **4/4** | **38 %** | **277 ms** | **2.7** | Deployed |

V3's three breakthroughs were IC-outer reuse, explicit DMA staging buffers, and phase-separated writes. Compute utilisation rose from 1.8 % to 29.6 % while LUT usage roughly halved.

---

## 3. System integration and results

The Vivado block design (`Redesign`) instantiates `conv_engine_1` with four AXI SmartConnects into the Zynq UltraScale+ PS and DDR4 controller.

**Post-route utilisation** (`Redesign_wrapper_utilization_placed.rpt`):

| Resource | Used | Available | Utilisation |
|---|---:|---:|---:|
| LUT | 105 148 | 274 080 | 38.4 % |
| FF | 145 010 | 548 160 | 26.5 % |
| BRAM | 362 | 912 | 39.7 % |
| DSP48E2 | 355 | 2 520 | 14.1 % |

**Timing:** `clk_pl_0` at 6.0 ns (166.67 MHz), worst negative slack **+0.067 ns**, zero failing endpoints. The critical path runs from `psum_buf` BRAM into the `acc_buf` distributed RAM inside `Execute_Layer`.

**Power:** 6.98 W total on-chip (PS 2.75 W, PL accelerator roughly 3.5 W).

**Per-layer measured time at 167 MHz:** conv1 57.3 ms, conv2 37.7, conv3 24.3, conv4 19.8, conv5 16.0, conv6 17.2 (+8.5 ms software pooling), conv7 57.6, conv8 16.7, conv9 17.1, det 13.4 — total 277 ms, about 370 ms wall clock including host-side work.

## 4. Host software

`PL/tinyyolo_pynq.ipynb` loads the overlay, fuses each Conv+BatchNorm pair on the CPU (`scale = γ/√(var+ε)`, `bias = β − scale·mean`), quantises to int16 via `round(x × 256)`, allocates physically contiguous PYNQ buffers, and then loops over ten layers writing 64-bit DMA pointers and scalar parameters into the AXI-Lite register bank before pulsing `ap_start` and polling `ap_done`. YOLO decoding and non-maximum suppression run in NumPy.

## 5. Known gaps

1. `conv_engine.cpp` includes `"conv_engine_v3.h"`, but the file on disk is `conv_engine.h` — a clean HLS build needs a rename or symlink.
2. `run_hls.tcl` targets a 7 ns clock (143 MHz) while the deployed design closes at 6 ns; the clock was tuned in Vivado rather than in the checked-in script. The script also runs only `csynth_design` and `export_design`, never `csim_design` or `cosim_design`.
3. `run_hls_v3.tcl`, referenced by `PL/README.md`, is not in the repository.
4. HLS synthesis reports (latency, II, per-loop resource estimates) are gitignored, so only Vivado post-synthesis and post-route numbers are available.
5. Conv6 uses stride-1 pooling, which has no hardware path in `Write_Layer` and is therefore done in software at a cost of 8.45 ms per frame.

## 6. Headroom

The documented roadmap toward 30 FPS: widen the MAC array to `TILE_OC = 32` (512 MACs, 1.5–2×), move to a 512-bit AXI bus (1.3×), push the clock to 200 MHz (1.2×), instantiate two compute units (1.8×), and fuse layers to avoid round-tripping feature maps through DDR (1.3×). Note that DSP utilisation is only 14 %, so there is substantial arithmetic capacity still unused — the design is presently memory-bound, not compute-bound.
