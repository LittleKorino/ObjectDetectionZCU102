# Tiny-YOLO CNN Accelerator on Xilinx ZCU102 (PYNQ)

> **HLS-based convolutional neural network inference engine** running all 10 layers of Tiny-YOLO v2 on the FPGA programmable logic, driven from the ARM PS via PYNQ.

![Platform](https://img.shields.io/badge/Platform-ZCU102-blue)
![Tool](https://img.shields.io/badge/Vitis%20HLS-2024.2-green)
![Clock](https://img.shields.io/badge/Clock-167%20MHz-orange)
![Status](https://img.shields.io/badge/Status-Deployed-brightgreen)

---

## Overview

A three-stage **dataflow** HLS accelerator (`Fetch → Execute → Write`) that processes Tiny-YOLO v2 object detection entirely on FPGA fabric. The PS (ARM Cortex-A53) handles weight loading, image preprocessing, and YOLO post-processing (decode + NMS).

| Component | Role |
|-----------|------|
| **Fetch_Layer** | Tiled DMA of inputs + weights from DDR via burst-friendly staging buffers |
| **Execute_Layer** | 256-MAC tiled convolution, fused BatchNorm (scale+bias), LeakyReLU / ReLU / Linear |
| **Write_Layer** | Optional 2×2 MaxPool (stride 1 or 2), phase-separated 256-bit wide-bus DMA writes |

**Data format:** `ap_fixed<16,8>` — 16-bit fixed point (Q8.8)
**Memory bus:** 256-bit AXI (`ap_uint<256>` = 16 values per beat)
**MAC array:** 16 OC × 16 IC = **256 MACs/cycle** at II=1

---

## Architecture

```
┌─────────────────────────────────────────────────────────────────────┐
│                         ZCU102 SoC                                  │
│  ┌───────────────┐                    ┌───────────────────────────┐ │
│  │   PS (ARM)    │  AXI-Lite (ctrl)   │     PL (FPGA Fabric)      │ │
│  │               │───────────────────▶│                           │ │
│  │ • Preprocess  │                    │  ┌───────┐  ┌─────────┐  │ │
│  │ • Weight load │  AXI4 (256-bit)    │  │ Fetch │─▶│ Execute │  │ │
│  │ • YOLO decode │◀──────────────────▶│  │ Layer │  │  Layer  │  │ │
│  │ • NMS         │      DDR4          │  └───┬───┘  └────┬────┘  │ │
│  │ • Display     │                    │      │weights    │       │ │
│  └───────────────┘                    │      ▼           ▼       │ │
│                                       │  ┌──────────────────┐    │ │
│                                       │  │   Write_Layer    │    │ │
│                                       │  │  (+ MaxPool)     │    │ │
│                                       │  └──────────────────┘    │ │
│                                       └───────────────────────────┘ │
└─────────────────────────────────────────────────────────────────────┘
```

### Layer Execution Plan

| # | Layer | IC→OC | K | Pool | Activation |
|---|-------|-------|---|------|------------|
| 0 | conv1 | 3→16 | 3 | 2×2 s2 | LeakyReLU |
| 1 | conv2 | 16→32 | 3 | 2×2 s2 | LeakyReLU |
| 2 | conv3 | 32→64 | 3 | 2×2 s2 | LeakyReLU |
| 3 | conv4 | 64→128 | 3 | 2×2 s2 | LeakyReLU |
| 4 | conv5 | 128→256 | 3 | 2×2 s2 | LeakyReLU |
| 5 | conv6 | 256→512 | 3 | 2×2 s1* | LeakyReLU |
| 6 | conv7 | 512→1024 | 3 | — | LeakyReLU |
| 7 | conv8 | 1024→256 | 1 | — | LeakyReLU |
| 8 | conv9 | 256→512 | 3 | — | LeakyReLU |
| 9 | det | 512→125 | 1 | — | Linear |

\*Conv6 pooling (stride-1) is handled in software on the PS.

---

## Performance Results

### V3 On-Board Measurements (ZCU102 @ 167 MHz)

| Layer | Time (ms) | Output Shape |
|-------|----------:|--------------|
| conv1 | 57.31 | 16×208×208 |
| conv2 | 37.70 | 32×104×104 |
| conv3 | 24.30 | 64×52×52 |
| conv4 | 19.82 | 128×26×26 |
| conv5 | 15.99 | 256×13×13 |
| conv6 | 17.17 | 512×13×13 |
| SW pool1 | 8.45 | 512×13×13 |
| conv7 | 57.58 | 1024×13×13 |
| conv8 | 16.69 | 256×13×13 |
| conv9 | 17.08 | 512×13×13 |
| det | 13.40 | 425×13×13 |
| **Total** | **277.04** | — |

**Throughput:** ~2.7 FPS (wall time) / 3.6 FPS (HW only)

### Design Evolution

| Version | Clock | Burst Status | LUT | HW Time | FPS | Status |
|---------|-------|-------------|-----|---------|-----|--------|
| **V1** | 143 MHz | ALL FAIL | 74% | 3,207 ms | 0.31 | Baseline |
| **V2** | 250 MHz | Partial | 85% | — | — | FAILED (timing) |
| **V3** | 167 MHz | **ALL PASS** | **38%** | **277 ms** | **~2.7** | **Deployed** |

### V3 vs V1 Improvement

| Metric | V1 | V3 | Change |
|--------|----|----|--------|
| HW time | 3,207 ms | 277 ms | **11.6× faster** |
| Compute utilization | 1.8% | 29.6% | **16.4× improvement** |
| LUT utilization | 74% | 38% | **49% reduction** |
| AXI bursts | 0/4 ports | 4/4 ports | All inferred |

---

## Resource Utilization (Post-Route)

| Resource | Used | Available | Util% |
|----------|-----:|----------:|------:|
| LUT | 105,148 | 274,080 | 38.4% |
| FF | 145,010 | 548,160 | 26.5% |
| BRAM | 362 | 912 | 39.7% |
| DSP48E2 | 355 | 2,520 | 14.1% |

**Power:** 6.98 W total (PL accelerator: ~3.48 W)

---

## Key Technical Innovations (V3)

1. **IC-outer loop ordering** — Input data loaded once per IC tile, reused across all OC tiles. Eliminates up to 64× redundant DRAM reads.

2. **DMA staging buffers** — Explicit burst read/write channels (`dma_line[4]`, `dma_wt[12]`, `dma_out[28]`) guarantee AXI burst inference on all ports.

3. **Phase-separated output writes** — READ edge → PACK → BURST WRITE stages prevent interleaved access patterns that defeated burst inference in V1/V2.

4. **Partial sum buffer** — `psum_buf[64][16][16][16]` enables IC-outer accumulation across multiple IC tiles without redundant input reads.

5. **Verilog-like coding style** — Address computation via shifts/masks (`>> 4`, `& 0xF`), explicit `.range()` bit-slicing, and phase-separated FSM design.



---

## Quick Start

### 1. Build the HLS IP

```bash
vitis-run --mode hls --tcl run_hls_v3.tcl | tee build_v3.log
```

### 2. Vivado Block Design

- Import the exported IP from `tinyyolo_zcu102_v3/solution1/impl/ip/`
- Connect to Zynq UltraScale+ PS via AXI interconnects (4 master ports: gmem0–3)
- Generate bitstream

### 3. Run on PYNQ

```python
from pynq import Overlay
ol = Overlay('tinyyolo_zcu102_v3.bit')
conv_ip = ol.conv_engine_1
# See tinyyolo_pynq.ipynb for full inference pipeline
```

---

## Numerical Accuracy

All 10 layers verified against PyTorch float32 reference:

| Layers 0–8 | Max Δ < 1.0, Correlation > 0.999 |
|------------|----------------------------------|
| Layer 9 (det) | Max Δ = 43.4, Correlation = 0.9991 |

Detection layer deviation is due to accumulated Q8.8 quantization across the full network and the unbounded linear activation range.

---

## Roadmap to 30 FPS

Current: **2.7 FPS** → Target: **30 FPS** (requires ~8.4× further speedup)

| Optimization | Expected Gain | Complexity |
|-------------|---------------|------------|
| Wider MAC (TILE_OC=32, 512 MACs) | 1.5–2× | Low |
| 512-bit AXI bus | 1.3× | Low |
| Clock to 200 MHz | 1.2× | Medium |
| Multi-CU instantiation (2× engines) | 1.8× | Medium |
| Layer fusion (eliminate inter-layer DDR) | 1.3× | High |

---

## Requirements

- **Board:** Xilinx ZCU102 (xczu9eg-ffvb1156-2-e)
- **Tools:** Vitis HLS 2024.2, Vivado 2024.2
- **Runtime:** PYNQ 3.0+ with Python 3.8+, PyTorch (for weight extraction), OpenCV

## License

This project is for research and educational purposes.
