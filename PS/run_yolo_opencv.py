import torch
import torch.nn as nn
import numpy as np
import cv2
import argparse
import time
from pathlib import Path


# ============================================
# MODEL ARCHITECTURE (must match training)
# ============================================

class ConvBlock(nn.Module):
    """Convolutional block: Conv2d + BatchNorm + LeakyReLU"""
    def __init__(self, in_channels, out_channels, kernel_size, stride=1, padding=0):
        super(ConvBlock, self).__init__()
        self.conv = nn.Conv2d(in_channels, out_channels, kernel_size, stride, padding, bias=False)
        self.bn = nn.BatchNorm2d(out_channels)
        self.leaky = nn.LeakyReLU(0.1, inplace=True)
    
    def forward(self, x):
        return self.leaky(self.bn(self.conv(x)))


class TinyYOLO(nn.Module):
    """Tiny-YOLO Architecture"""
    
    def __init__(self, num_classes=80, num_anchors=5, dropout_rate=0.0):
        super(TinyYOLO, self).__init__()
        self.num_classes = num_classes
        self.num_anchors = num_anchors
        
        # Layer 0: Input (3 channels) -> 16 filters
        self.conv1 = ConvBlock(3, 16, kernel_size=3, stride=1, padding=1)
        self.pool1 = nn.MaxPool2d(kernel_size=2, stride=2)
        
        # Layer 1: 16 -> 32 filters
        self.conv2 = ConvBlock(16, 32, kernel_size=3, stride=1, padding=1)
        self.pool2 = nn.MaxPool2d(kernel_size=2, stride=2)
        
        # Layer 2: 32 -> 64 filters
        self.conv3 = ConvBlock(32, 64, kernel_size=3, stride=1, padding=1)
        self.pool3 = nn.MaxPool2d(kernel_size=2, stride=2)
        
        # Layer 3: 64 -> 128 filters
        self.conv4 = ConvBlock(64, 128, kernel_size=3, stride=1, padding=1)
        self.pool4 = nn.MaxPool2d(kernel_size=2, stride=2)
        
        # Layer 4: 128 -> 256 filters
        self.conv5 = ConvBlock(128, 256, kernel_size=3, stride=1, padding=1)
        self.pool5 = nn.MaxPool2d(kernel_size=2, stride=2)
        
        # Dropout (inactive during inference)
        self.dropout1 = nn.Dropout2d(p=dropout_rate)
        
        # Layer 5: 256 -> 512 filters
        self.conv6 = ConvBlock(256, 512, kernel_size=3, stride=1, padding=1)
        self.pad6 = nn.ZeroPad2d((0, 1, 0, 1))
        self.pool6 = nn.MaxPool2d(kernel_size=2, stride=1)
        
        self.dropout2 = nn.Dropout2d(p=dropout_rate)
        
        # Layer 6: 512 -> 1024 filters
        self.conv7 = ConvBlock(512, 1024, kernel_size=3, stride=1, padding=1)
        
        self.dropout3 = nn.Dropout2d(p=dropout_rate)
        
        # Layer 7: 1024 -> 256 filters
        self.conv8 = ConvBlock(1024, 256, kernel_size=1, stride=1, padding=0)
        
        # Layer 8: 256 -> 512 filters
        self.conv9 = ConvBlock(256, 512, kernel_size=3, stride=1, padding=1)
        
        # Detection Layer
        self.detection = nn.Conv2d(512, num_anchors * (5 + num_classes), kernel_size=1)
    
    def forward(self, x):
        x = self.pool1(self.conv1(x))
        x = self.pool2(self.conv2(x))
        x = self.pool3(self.conv3(x))
        x = self.pool4(self.conv4(x))
        x = self.pool5(self.conv5(x))
        x = self.dropout1(x)
        
        x = self.pool6(self.pad6(self.conv6(x)))
        x = self.dropout2(x)
        
        x = self.conv7(x)
        x = self.dropout3(x)
        
        x = self.conv8(x)
        x = self.conv9(x)
        x = self.detection(x)
        return x


# ============================================
# CONFIGURATION
# ============================================

IMAGE_SIZE = 416
GRID_SIZE = 13
NUM_CLASSES = 80
NUM_ANCHORS = 5

# COCO Classes (80 classes)
COCO_CLASSES = [
    'person', 'bicycle', 'car', 'motorcycle', 'airplane', 'bus', 'train', 'truck', 'boat', 'traffic light',
    'fire hydrant', 'stop sign', 'parking meter', 'bench', 'bird', 'cat', 'dog', 'horse', 'sheep', 'cow',
    'elephant', 'bear', 'zebra', 'giraffe', 'backpack', 'umbrella', 'handbag', 'tie', 'suitcase', 'frisbee',
    'skis', 'snowboard', 'sports ball', 'kite', 'baseball bat', 'baseball glove', 'skateboard', 'surfboard',
    'tennis racket', 'bottle', 'wine glass', 'cup', 'fork', 'knife', 'spoon', 'bowl', 'banana', 'apple',
    'sandwich', 'orange', 'broccoli', 'carrot', 'hot dog', 'pizza', 'donut', 'cake', 'chair', 'couch',
    'potted plant', 'bed', 'dining table', 'toilet', 'tv', 'laptop', 'mouse', 'remote', 'keyboard',
    'cell phone', 'microwave', 'oven', 'toaster', 'sink', 'refrigerator', 'book', 'clock', 'vase',
    'scissors', 'teddy bear', 'hair drier', 'toothbrush'
]

# Anchor boxes
ANCHORS = np.array([
    [1.08, 1.19],
    [3.42, 4.41],
    [6.63, 11.38],
    [9.42, 5.11],
    [16.62, 10.52]
])

# Colors for visualization (one per class)
np.random.seed(42)
COLORS = np.random.randint(0, 255, size=(NUM_CLASSES, 3), dtype=np.uint8)


# ============================================
# INFERENCE FUNCTIONS
# ============================================

def preprocess_image(image, target_size=416):
    """Preprocess image for model input"""
    # Resize
    h, w = image.shape[:2]
    scale = target_size / max(h, w)
    new_h, new_w = int(h * scale), int(w * scale)
    resized = cv2.resize(image, (new_w, new_h))
    
    # Pad to square
    canvas = np.full((target_size, target_size, 3), 128, dtype=np.uint8)
    pad_h = (target_size - new_h) // 2
    pad_w = (target_size - new_w) // 2
    canvas[pad_h:pad_h+new_h, pad_w:pad_w+new_w] = resized
    
    # Convert BGR to RGB and normalize
    rgb = cv2.cvtColor(canvas, cv2.COLOR_BGR2RGB)
    normalized = rgb.astype(np.float32) / 255.0
    normalized = (normalized - [0.485, 0.456, 0.406]) / [0.229, 0.224, 0.225]
    
    # HWC to CHW and add batch dimension
    tensor = np.transpose(normalized, (2, 0, 1))
    tensor = np.expand_dims(tensor, 0)
    
    return tensor, scale, pad_w, pad_h


def sigmoid(x):
    return 1 / (1 + np.exp(-np.clip(x, -500, 500)))


def decode_predictions(predictions, anchors, num_classes, conf_threshold=0.3, nms_threshold=0.4):
    """Decode YOLO predictions to bounding boxes"""
    batch_size = predictions.shape[0]
    grid_size = predictions.shape[2]
    num_anchors = anchors.shape[0]
    
    # Reshape: (B, A*(5+C), G, G) -> (B, G, G, A, 5+C)
    predictions = predictions.reshape(batch_size, num_anchors, 5 + num_classes, grid_size, grid_size)
    predictions = np.transpose(predictions, (0, 3, 4, 1, 2))
    
    all_detections = []
    
    for b in range(batch_size):
        detections = []
        
        for cy in range(grid_size):
            for cx in range(grid_size):
                for a in range(num_anchors):
                    pred = predictions[b, cy, cx, a]
                    
                    # Get confidence
                    conf = sigmoid(pred[4])
                    
                    if conf < conf_threshold:
                        continue
                    
                    # Get class probabilities
                    class_probs = sigmoid(pred[5:])
                    class_idx = np.argmax(class_probs)
                    class_score = class_probs[class_idx]
                    
                    # Final score
                    score = conf * class_score
                    
                    if score < conf_threshold:
                        continue
                    
                    # Decode bounding box
                    tx, ty = sigmoid(pred[0]), sigmoid(pred[1])
                    tw, th = pred[2], pred[3]
                    
                    # Convert to absolute coordinates (normalized 0-1)
                    bx = (cx + tx) / grid_size
                    by = (cy + ty) / grid_size
                    bw = anchors[a, 0] * np.exp(np.clip(tw, -10, 10)) / grid_size
                    bh = anchors[a, 1] * np.exp(np.clip(th, -10, 10)) / grid_size
                    
                    # Convert to corner format
                    x1 = max(0, bx - bw / 2)
                    y1 = max(0, by - bh / 2)
                    x2 = min(1, bx + bw / 2)
                    y2 = min(1, by + bh / 2)
                    
                    detections.append({
                        'bbox': [x1, y1, x2, y2],
                        'score': float(score),
                        'class': int(class_idx)
                    })
        
        # Apply NMS
        if len(detections) > 0:
            detections = apply_nms(detections, nms_threshold)
        
        all_detections.append(detections)
    
    return all_detections


def compute_iou(box1, box2):
    """Compute IoU between two boxes"""
    x1 = max(box1[0], box2[0])
    y1 = max(box1[1], box2[1])
    x2 = min(box1[2], box2[2])
    y2 = min(box1[3], box2[3])
    
    inter_area = max(0, x2 - x1) * max(0, y2 - y1)
    box1_area = (box1[2] - box1[0]) * (box1[3] - box1[1])
    box2_area = (box2[2] - box2[0]) * (box2[3] - box2[1])
    
    return inter_area / (box1_area + box2_area - inter_area + 1e-6)


def apply_nms(detections, threshold):
    """Apply Non-Maximum Suppression"""
    if len(detections) == 0:
        return []
    
    # Sort by score
    detections = sorted(detections, key=lambda x: x['score'], reverse=True)
    
    keep = []
    while len(detections) > 0:
        best = detections.pop(0)
        keep.append(best)
        
        detections = [d for d in detections if 
                      d['class'] != best['class'] or 
                      compute_iou(best['bbox'], d['bbox']) < threshold]
    
    return keep


def draw_detections(image, detections, scale, pad_w, pad_h, class_names, colors):
    """Draw bounding boxes and labels on image"""
    h, w = image.shape[:2]
    target_size = IMAGE_SIZE
    
    for det in detections:
        bbox = det['bbox']
        class_idx = det['class']
        score = det['score']
        
        # Convert normalized coords to padded image coords
        x1_pad = int(bbox[0] * target_size)
        y1_pad = int(bbox[1] * target_size)
        x2_pad = int(bbox[2] * target_size)
        y2_pad = int(bbox[3] * target_size)
        
        # Remove padding and scale back to original
        x1 = int((x1_pad - pad_w) / scale)
        y1 = int((y1_pad - pad_h) / scale)
        x2 = int((x2_pad - pad_w) / scale)
        y2 = int((y2_pad - pad_h) / scale)
        
        # Clamp to image bounds
        x1 = max(0, min(x1, w - 1))
        y1 = max(0, min(y1, h - 1))
        x2 = max(0, min(x2, w - 1))
        y2 = max(0, min(y2, h - 1))
        
        # Get color
        color = tuple(int(c) for c in colors[class_idx])
        
        # Draw rectangle
        cv2.rectangle(image, (x1, y1), (x2, y2), color, 2)
        
        # Draw label background
        label = f'{class_names[class_idx]}: {score:.2f}'
        (label_w, label_h), baseline = cv2.getTextSize(label, cv2.FONT_HERSHEY_SIMPLEX, 0.5, 1)
        cv2.rectangle(image, (x1, y1 - label_h - 10), (x1 + label_w + 5, y1), color, -1)
        
        # Draw label text
        cv2.putText(image, label, (x1 + 2, y1 - 5), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)
    
    return image


# ============================================
# DETECTOR CLASS
# ============================================

class TinyYOLODetector:
    def __init__(self, weights_path, device='cuda', conf_threshold=0.3, nms_threshold=0.4):
        self.device = torch.device(device if torch.cuda.is_available() else 'cpu')
        self.conf_threshold = conf_threshold
        self.nms_threshold = nms_threshold
        
        print(f"Using device: {self.device}")
        
        # Load model
        self.model = TinyYOLO(num_classes=NUM_CLASSES, num_anchors=NUM_ANCHORS).to(self.device)
        
        # Load weights
        print(f"Loading weights from: {weights_path}")
        checkpoint = torch.load(weights_path, map_location=self.device)
        
        # Handle compiled model weights
        state_dict = checkpoint['model_state_dict']
        # Remove '_orig_mod.' prefix if present (from torch.compile)
        new_state_dict = {}
        for k, v in state_dict.items():
            if k.startswith('_orig_mod.'):
                new_state_dict[k[10:]] = v
            else:
                new_state_dict[k] = v
        
        self.model.load_state_dict(new_state_dict)
        self.model.eval()
        
        if 'val_loss' in checkpoint:
            print(f"Model validation loss: {checkpoint['val_loss']:.4f}")
        if 'val_iou' in checkpoint:
            print(f"Model validation IoU: {checkpoint['val_iou']:.4f}")
        
        print("✓ Model loaded successfully!")
    
    def detect(self, image):
        """Run detection on a single image (BGR format)"""
        # Preprocess
        tensor, scale, pad_w, pad_h = preprocess_image(image, IMAGE_SIZE)
        tensor = torch.from_numpy(tensor).float().to(self.device)
        
        # Inference
        with torch.no_grad():
            predictions = self.model(tensor)
        
        # Decode
        predictions = predictions.cpu().numpy()
        detections = decode_predictions(
            predictions, ANCHORS, NUM_CLASSES,
            self.conf_threshold, self.nms_threshold
        )[0]
        
        return detections, scale, pad_w, pad_h
    
    def detect_and_draw(self, image):
        """Run detection and draw results on image"""
        detections, scale, pad_w, pad_h = self.detect(image)
        result = draw_detections(image.copy(), detections, scale, pad_w, pad_h, COCO_CLASSES, COLORS)
        return result, detections


# ============================================
# MAIN FUNCTIONS
# ============================================

def run_webcam(detector, camera_idx=0):
    """Run real-time detection on webcam"""
    print(f"Opening camera {camera_idx}...")
    cap = cv2.VideoCapture(camera_idx)
    
    if not cap.isOpened():
        print("Error: Could not open camera")
        return
    
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, 1280)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 720)
    
    print("Press 'q' to quit, 's' to save screenshot")
    
    fps_history = []
    
    while True:
        ret, frame = cap.read()
        if not ret:
            break
        
        # Detect
        start_time = time.time()
        result, detections = detector.detect_and_draw(frame)
        inference_time = time.time() - start_time
        
        # Calculate FPS
        fps = 1 / inference_time
        fps_history.append(fps)
        if len(fps_history) > 30:
            fps_history.pop(0)
        avg_fps = sum(fps_history) / len(fps_history)
        
        # Draw FPS
        cv2.putText(result, f'FPS: {avg_fps:.1f} | Objects: {len(detections)}', 
                    (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
        
        # Show
        cv2.imshow('Tiny-YOLO Detection', result)
        
        key = cv2.waitKey(1) & 0xFF
        if key == ord('q'):
            break
        elif key == ord('s'):
            filename = f'detection_{int(time.time())}.jpg'
            cv2.imwrite(filename, result)
            print(f"Saved: {filename}")
    
    cap.release()
    cv2.destroyAllWindows()


def run_video(detector, video_path, output_path=None):
    """Run detection on video file"""
    print(f"Processing video: {video_path}")
    cap = cv2.VideoCapture(video_path)
    
    if not cap.isOpened():
        print(f"Error: Could not open video {video_path}")
        return
    
    # Get video properties
    width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    fps = cap.get(cv2.CAP_PROP_FPS)
    total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
    
    print(f"Video: {width}x{height} @ {fps:.1f} FPS, {total_frames} frames")
    
    # Setup output writer
    writer = None
    if output_path:
        fourcc = cv2.VideoWriter_fourcc(*'mp4v')
        writer = cv2.VideoWriter(output_path, fourcc, fps, (width, height))
    
    frame_count = 0
    
    while True:
        ret, frame = cap.read()
        if not ret:
            break
        
        result, detections = detector.detect_and_draw(frame)
        
        frame_count += 1
        print(f"\rFrame {frame_count}/{total_frames} - {len(detections)} objects", end='')
        
        if writer:
            writer.write(result)
        
        cv2.imshow('Tiny-YOLO Detection', result)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
    
    print(f"\nProcessed {frame_count} frames")
    
    cap.release()
    if writer:
        writer.release()
        print(f"Saved: {output_path}")
    cv2.destroyAllWindows()


def run_image(detector, image_path, output_path=None):
    """Run detection on single image"""
    print(f"Processing image: {image_path}")
    
    image = cv2.imread(image_path)
    if image is None:
        print(f"Error: Could not load image {image_path}")
        return
    
    start_time = time.time()
    result, detections = detector.detect_and_draw(image)
    inference_time = time.time() - start_time
    
    print(f"Inference time: {inference_time*1000:.1f} ms")
    print(f"Found {len(detections)} objects:")
    
    for det in detections:
        class_name = COCO_CLASSES[det['class']]
        score = det['score']
        print(f"  - {class_name}: {score:.2f}")
    
    if output_path:
        cv2.imwrite(output_path, result)
        print(f"Saved: {output_path}")
    
    cv2.imshow('Tiny-YOLO Detection', result)
    print("Press any key to close...")
    cv2.waitKey(0)
    cv2.destroyAllWindows()


def main():
    parser = argparse.ArgumentParser(description='Tiny-YOLO Object Detection with OpenCV')
    parser.add_argument('--weights', type=str, default='tiny_yolo_best.pth',
                        help='Path to model weights')
    parser.add_argument('--image', type=str, help='Path to input image')
    parser.add_argument('--video', type=str, help='Path to input video')
    parser.add_argument('--camera', type=int, default=0, help='Camera index')
    parser.add_argument('--output', type=str, help='Path to save output')
    parser.add_argument('--conf', type=float, default=0.3, help='Confidence threshold')
    parser.add_argument('--nms', type=float, default=0.4, help='NMS threshold')
    parser.add_argument('--cpu', action='store_true', help='Use CPU instead of GPU')
    
    args = parser.parse_args()
    
    # Check weights exist
    weights_path = Path(args.weights)
    if not weights_path.exists():
        print(f"Error: Weights file not found: {weights_path}")
        print("Make sure to copy 'tiny_yolo_best.pth' from the training machine.")
        return
    
    # Initialize detector
    device = 'cpu' if args.cpu else 'cuda'
    detector = TinyYOLODetector(
        weights_path=str(weights_path),
        device=device,
        conf_threshold=args.conf,
        nms_threshold=args.nms
    )
    
    # Run appropriate mode
    if args.image:
        run_image(detector, args.image, args.output)
    elif args.video:
        run_video(detector, args.video, args.output)
    else:
        run_webcam(detector, args.camera)


if __name__ == '__main__':
    print("=" * 60)
    print("Tiny-YOLO Object Detection")
    print("=" * 60)
    main()
