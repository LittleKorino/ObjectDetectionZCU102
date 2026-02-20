# Custom FPGA-Accelerated Tiny-YOLO on Xilinx ZCU102

> End-to-end project: custom OS image → model training → HLS accelerator → live inference on FPGA.
> **Final result:** 2.7 FPS on ZCU102 — **11.6× faster** than the V1 baseline.

---

## Demo

[![Demo Video](https://img.youtube.com/vi/qymDbAOg1Cg/0.jpg)](https://youtu.be/qymDbAOg1Cg)

---

## Project Overview

This project builds a complete hardware-accelerated object detection pipeline from scratch on the Xilinx ZCU102 (Zynq UltraScale+). The Tiny-YOLO v2 model runs with all 10 convolutional layers offloaded to the FPGA programmable logic via a custom HLS IP, while the ARM PS handles preprocessing, weight loading, and YOLO post-processing.

```
┌─────────────────────────────────────────────────────────────────────────┐
│                              ZCU102 SoC                                 │
│                                                                         │
│   PS (ARM Cortex-A53)                     PL (FPGA Fabric)              │
│   ┌──────────────────────┐                ┌────────────────────────┐    │
│   │ • Image capture      │  AXI4 256-bit  │  Fetch ─▶ Execute      │    │
│   │ • Weight loading     │◀──────────────▶│               ─▶ Write │    │
│   │ • BN fusion (Q8.8)   │  AXI-Lite      │                        │    │
│   │ • YOLO decode + NMS  │───────────────▶│  256-MAC array         │    │
│   │ • Display output     │                │  Fused BN + LeakyReLU  │    │
│   └──────────────────────┘                └────────────────────────┘    │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## Repository Structure

```
├── OS/                          # Step 0 — Custom PYNQ image build
│   └── README.md
├── Training/                    # Step 1 — Model training (RTX 4090)
│   ├── ObjectDetection.ipynb
│   └── README.md
├── Models/                      # Trained weights
│   ├── tiny_yolo_best.pth
│   └── tiny_yolo_best_iou.pth
├── PS/                          # Step 2 — CPU inference (PS-side testing)
│   ├── run_yolo_opencv.py
│   └── README.md
├── HLS/                         # Step 3 — HLS accelerator design
│   ├── conv_engine.h
│   ├── conv_engine.cpp
│   ├── conv_engine_tb.cpp
│   ├── run_hls.tcl
│   └── Vivado/
│       └── ConvBlock/           # Vivado block design project
├── PL/                          # Step 4 — PYNQ deployment & inference
│   ├── tinyyolo_pynq.ipynb
│   └── README.md
└── .gitignore
```

---

## End-to-End Flow

### Step 0 — Build the OS
**[OS/README.md](OS/README.md)**

Built a custom PYNQ 3.1.2 SD card image for the ZCU102 using Vivado/PetaLinux **2024.1** and Docker.

Key lesson: use the correct toolchain version and build from source — do not use cached prebuilt rootfs.

```bash
make BOARDDIR=/workspace/sdbuild/boards BOARD=ZCU102
# ~2 hours → ZCU102-3.1.2.img
```

---

### Step 1 — Train the Model
**[Training/README.md](Training/README.md)**

Trained Tiny-YOLO v2 on COCO 2017 (80 classes) from scratch using PyTorch on an **RTX 4090** with full CUDA optimizations.

| Setting | Value |
|---------|-------|
| Architecture | Tiny-YOLO v2 (9× ConvBlock + detection) |
| Dataset | COCO 2017 (118K train / 5K val) |
| Loss | CIoU + label smoothing |
| Precision | AMP (FP16) with TF32 tensor cores |
| Output | `tiny_yolo_best.pth`, `tiny_yolo_best_iou.pth` |

---

### Step 2 — Test on PS (CPU Inference)
**[PS/README.md](PS/README.md)**

Quick sanity-check script to verify model detection quality on the ARM PS before committing to hardware deployment. Uses PyTorch + OpenCV, no FPGA required.

```bash
python PS/run_yolo_opencv.py --weights Models/tiny_yolo_best.pth --image photo.jpg
```

---

### Step 3 — HLS Accelerator Design
**[HLS/](HLS/)**

Custom three-stage dataflow HLS IP core (`conv_engine`) that executes all 10 Tiny-YOLO layers on the FPGA fabric.

**Architecture:**
```
DDR ──▶ Fetch_Layer ──▶ Execute_Layer ──▶ Write_Layer ──▶ DDR
          (DMA)          (256-MAC array)     (MaxPool)
```

**Data format:** `ap_fixed<16,8>` (Q8.8 fixed point)  
**Memory bus:** 256-bit AXI (`ap_uint<256>` = 16 values/beat)  
**MAC array:** 16 OC × 16 IC = **256 MACs/cycle** at II=1

Three major design revisions:

| Version | Clock | FPS | Status |
|---------|-------|-----|--------|
| V1 | 143 MHz | 0.31 | Baseline — naive DRAM access, 1.8% compute utilization |
| V2 | 250 MHz | — | FAILED — timing violation (−2.05 ns), 61B cycle latency |
| **V3** | **167 MHz** | **2.7** | **Deployed — 4/4 burst ports, 29.6% compute utilization** |

Key V3 breakthroughs:
- **IC-outer loop ordering** — input read once per IC tile, reused across all OC tiles (up to 64× fewer DRAM reads)
- **DMA staging buffers** — `dma_line`, `dma_wt`, `dma_out` guarantee burst inference on all 4 AXI ports
- **Phase-separated writes** — READ → PACK → BURST WRITE, no interleaved R/W

Build:
```bash
vitis-run --mode hls --tcl HLS/run_hls.tcl
```

---

### Step 4 — Vivado Block Design
**[HLS/Vivado/ConvBlock/](HLS/Vivado/ConvBlock/)**

Vivado block design connecting the HLS IP to the Zynq PS via 4 AXI master ports (`gmem0`–`gmem3`) and an AXI-Lite control interface.

| Component | Role |
|-----------|------|
| `conv_engine_1` | HLS IP core |
| `axi_smc_gmem0–3` | SmartConnect for 4 DDR channels |
| `zynq_ultra_ps_e_0` | PS + DDR4 controller |

---

### Step 5 — Deploy and Run on PYNQ
**[PL/README.md](PL/README.md)**

Full inference pipeline notebook (`tinyyolo_pynq.ipynb`) running on the ZCU102 under PYNQ:

```python
from pynq import Overlay
ol = Overlay('tinyyolo_zcu102_v3.3.bit')
conv_ip = ol.conv_engine_1
# → loads weights, runs all 10 layers on FPGA, decodes YOLO output, draws boxes
```

---

## Results

### Performance (V3, ZCU102 @ 167 MHz)

| Layer | Time (ms) | Output |
|-------|----------:|--------|
| conv1 | 57.31 | 16×208×208 |
| conv2 | 37.70 | 32×104×104 |
| conv3 | 24.30 | 64×52×52 |
| conv4 | 19.82 | 128×26×26 |
| conv5 | 15.99 | 256×13×13 |
| conv6 | 17.17 | 512×13×13 |
| conv7 | 57.58 | 1024×13×13 |
| conv8 | 16.69 | 256×13×13 |
| conv9 | 17.08 | 512×13×13 |
| det | 13.40 | 425×13×13 |
| **Total HW** | **277 ms** | — |
| **Wall time** | **~370 ms** | **~2.7 FPS** |

### Resource Utilization (Post-Route)

| Resource | Used | Available | Util% |
|----------|-----:|----------:|------:|
| LUT | 105,148 | 274,080 | 38.4% |
| FF | 145,010 | 548,160 | 26.5% |
| BRAM | 362 | 912 | 39.7% |
| DSP48E2 | 355 | 2,520 | 14.1% |

**Power:** 6.98 W total (PL: ~3.48 W)

### Version Comparison

| Metric | V1 | V3 | Gain |
|--------|----|----|------|
| HW time | 3,207 ms | 277 ms | **11.6×** |
| Compute utilization | 1.8% | 29.6% | **16.4×** |
| LUT utilization | 74% | 38% | **halved** |
| Burst ports active | 0/4 | 4/4 | **full** |

---

## Requirements

| Component | Version |
|-----------|---------|
| Board | Xilinx ZCU102 (xczu9eg-ffvb1156-2-e) |
| OS | Custom PYNQ 3.1.2 (see OS/) |
| HLS/Vivado | Vitis HLS 2024.1, Vivado 2024.1 |
| Training | PyTorch, CUDA, RTX 4090 |
| Runtime | Python 3, PYNQ 3.0+, OpenCV, NumPy |

---

## Roadmap

Current: **2.7 FPS** → Target: **30 FPS**

| Optimization | Expected Gain | Complexity |
|-------------|---------------|------------|
| Wider MAC (TILE_OC=32 → 512 MACs) | 1.5–2× | Low |
| 512-bit AXI bus | 1.3× | Low |
| Clock to 200 MHz | 1.2× | Medium |
| Multi-CU (2× conv_engine) | 1.8× | Medium |
| Layer fusion (no inter-layer DDR) | 1.3× | High |
