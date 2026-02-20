# Training — Tiny-YOLO on COCO

## Overview

`ObjectDetection.ipynb` contains the full training pipeline used to train our custom Tiny-YOLO model on the COCO dataset. Training was done on an **RTX 4090 (24GB VRAM)** using CUDA, taking full advantage of its tensor cores and mixed-precision capabilities.

The resulting weights (`tiny_yolo_best.pth`) are deployed to the ZCU102 board — first tested on the PS (see `PS/`) then accelerated via the PL FPGA fabric (see `PL/` and `HLS/`).

---

## Hardware

| Component | Spec |
|-----------|------|
| GPU | NVIDIA RTX 4090 |
| VRAM | 24 GB |
| CUDA | Used for training only |
| Target Deployment | Xilinx ZCU102 (ARM + FPGA) |

---

## Training Flow

### 1. Environment Setup
Installs PyTorch, torchvision, OpenCV, tqdm, and pycocotools.

### 2. RTX 4090 Optimizations
```python
torch.backends.cudnn.benchmark = True        # Auto-tune convolution algorithms
torch.backends.cuda.matmul.allow_tf32 = True # TF32 tensor cores (~2x faster)
torch.set_float32_matmul_precision('high')
```
Automatic Mixed Precision (AMP) with `torch.amp.autocast` provides an additional ~2x speedup on Ada Lovelace tensor cores.

### 3. Model Architecture — Tiny-YOLO
9 convolutional layers with BatchNorm + LeakyReLU, trained from scratch:

```
Input (416x416x3)
  → Conv1  (16)  → Pool (/2) → 208x208
  → Conv2  (32)  → Pool (/2) → 104x104
  → Conv3  (64)  → Pool (/2) →  52x52
  → Conv4  (128) → Pool (/2) →  26x26
  → Conv5  (256) → Pool (/2) →  13x13
  → Conv6  (512) → Pool (s1) →  13x13
  → Conv7  (1024)            →  13x13
  → Conv8  (256, 1x1)        →  13x13
  → Conv9  (512)             →  13x13
  → Detection (5 × (5 + 80)) →  13x13x425
```

Dropout (30%) is applied in the deep layers (Conv6–Conv7) to prevent overfitting.

### 4. Dataset — COCO 2017
- **Training set:** ~118K images (with augmentation)
- **Validation set:** 5K images (no augmentation)
- **Classes:** 80 COCO categories

Data augmentation applied during training:
- Random horizontal flip (p=0.5)
- HSV color jitter (brightness, saturation, contrast)
- Random scale (80%–120%)

### 5. Loss Function — CIoU
Custom YOLO loss using **Complete IoU (CIoU)** instead of MSE for bounding box regression. CIoU directly optimizes:
1. Overlap (IoU)
2. Center point distance
3. Aspect ratio consistency

Combined with label smoothing (0.1) for better classification generalization.

### 6. Training Configuration

| Parameter | Value |
|-----------|-------|
| Batch size | 32 |
| Learning rate | 1e-4 (OneCycleLR) |
| Weight decay | 5e-4 |
| Epochs | 100 (early stopping) |
| Early stopping patience | 15 |
| Mixed precision (AMP) | ✅ FP16 |
| `torch.compile` | ✅ `reduce-overhead` mode |
| Optimizer | AdamW (fused) |

### 7. Outputs

| File | Description |
|------|-------------|
| `tiny_yolo_best.pth` | Best checkpoint by validation loss |
| `tiny_yolo_best_iou.pth` | Best checkpoint by validation IoU |

Checkpoints are saved to the `Models/` directory.

---

## Model Diagnostic

The notebook includes a built-in diagnostic cell that analyzes training curves and detects:
- **Underfitting** — high training loss, train/val gap < 1.5×
- **Overfitting** — low training loss, train/val gap > 2×
- **Healthy training** — train and val losses track closely

---

## Usage

1. Ensure COCO 2017 dataset is available at `~/korino/arm/training/coco_data/`
2. Open `ObjectDetection.ipynb` on a CUDA-capable machine
3. Run all cells sequentially
4. Copy the output `.pth` files to the `Models/` directory for deployment

---

## Notes

- This training pipeline runs **only on the training workstation** (RTX 4090)
- The ZCU102 board has no discrete GPU — deployment uses ARM CPU (PS) and FPGA (PL)
- For PS-side inference testing during training, see `PS/run_yolo_opencv.py`
- For FPGA-accelerated inference, see `PL/` and `HLS/`
