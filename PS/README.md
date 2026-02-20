# PS — Processing System Inference

## Purpose

`run_yolo_opencv.py` is a **quick testing script** used to evaluate Tiny-YOLO model performance directly on the PS (ARM CPU) during the training phase.

When iterating on the model during training, this script lets you immediately verify detection quality on real inputs — webcam, image, or video — without needing the PL (FPGA) fabric loaded. It serves as a fast sanity check before committing to hardware deployment.

---

## Usage

```bash
# Webcam (default)
python run_yolo_opencv.py

# Single image
python run_yolo_opencv.py --image photo.jpg --output result.jpg

# Video file
python run_yolo_opencv.py --video video.mp4 --output out.mp4

# Different camera index
python run_yolo_opencv.py --camera 1

# Force CPU
python run_yolo_opencv.py --cpu

# Adjust thresholds
python run_yolo_opencv.py --conf 0.4 --nms 0.5
```

## Arguments

| Argument | Default | Description |
|----------|---------|-------------|
| `--weights` | `tiny_yolo_best.pth` | Path to model weights `.pth` file |
| `--image` | — | Path to input image |
| `--video` | — | Path to input video |
| `--camera` | `0` | Camera index for webcam |
| `--output` | — | Path to save output image/video |
| `--conf` | `0.3` | Confidence threshold |
| `--nms` | `0.4` | NMS IoU threshold |
| `--cpu` | `False` | Force CPU inference |

## Requirements

```bash
pip install torch torchvision opencv-python numpy
```

## Weights

Copy the trained weights from the training machine into the `Models/` directory:

```
Models/
├── tiny_yolo_best.pth       # Best validation IoU checkpoint
└── tiny_yolo_best_iou.pth   # Best IoU-specific checkpoint
```

Then point the script to the weights:

```bash
python run_yolo_opencv.py --weights ../Models/tiny_yolo_best.pth
```

## Notes

- This runs purely on the **PS (ARM CPU)** — no FPGA/PL involvement
- Intended for **model validation during training**, not production deployment
- For hardware-accelerated inference using the PL, see the `PL/` directory
- Model architecture defined in this script must match the training architecture exactly
