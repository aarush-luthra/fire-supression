#!/usr/bin/env python3
"""
FireGuard ML — Fire Image Detection Training Script
Trains a MobileNetV2-based binary classifier on fire/non-fire images.
Outputs: Keras model (.h5) + TensorFlow.js model for browser inference.
"""

# Fix macOS SSL certificate issue
import os
import ssl
import certifi
os.environ['SSL_CERT_FILE'] = certifi.where()
ssl._create_default_https_context = ssl._create_unverified_context

import sys
import numpy as np
from pathlib import Path

# ── Configuration ──────────────────────────────────────────
IMG_SIZE = 128          # Input image size (128x128 — small for fast browser inference)
BATCH_SIZE = 16
EPOCHS = 15
VALIDATION_SPLIT = 0.2
LEARNING_RATE = 0.0005

# Paths
SCRIPT_DIR = Path(__file__).parent.resolve()
PROJECT_DIR = SCRIPT_DIR.parent
DATASET_DIR = PROJECT_DIR / "fire_dataset"
MODEL_DIR = SCRIPT_DIR / "model"
TFJS_DIR = PROJECT_DIR / "data" / "tfjs_model"

def check_dataset():
    """Verify dataset exists and show stats."""
    no_fire = DATASET_DIR / "0"
    fire = DATASET_DIR / "1"

    if not DATASET_DIR.exists():
        print(f"❌ Dataset not found at: {DATASET_DIR}")
        sys.exit(1)

    no_fire_count = len([f for f in no_fire.iterdir() if f.is_file()]) if no_fire.exists() else 0
    fire_count = len([f for f in fire.iterdir() if f.is_file()]) if fire.exists() else 0

    print(f"\n🔍 Dataset: {DATASET_DIR}")
    print(f"   No Fire (0): {no_fire_count} images")
    print(f"   Fire    (1): {fire_count} images")
    print(f"   Total:       {no_fire_count + fire_count} images\n")

    if fire_count == 0 or no_fire_count == 0:
        print("❌ Need images in both 0/ and 1/ folders.")
        sys.exit(1)

    return no_fire_count, fire_count

def load_and_preprocess():
    """Load images, resize, normalize, and split into train/val."""
    from PIL import Image

    images = []
    labels = []
    skipped = 0

    for label in [0, 1]:
        folder = DATASET_DIR / str(label)
        label_name = "No Fire" if label == 0 else "Fire"
        print(f"📂 Loading {label_name} images...")

        for img_file in sorted(folder.iterdir()):
            if not img_file.is_file():
                continue
            if img_file.suffix.lower() not in ('.jpg', '.jpeg', '.png', '.bmp', '.webp'):
                continue
            try:
                img = Image.open(img_file).convert('RGB')
                img = img.resize((IMG_SIZE, IMG_SIZE))
                arr = np.array(img, dtype=np.float32) / 255.0
                images.append(arr)
                labels.append(label)
            except Exception as e:
                skipped += 1
                continue

    if skipped > 0:
        print(f"   ⚠️  Skipped {skipped} corrupted/unreadable images")

    images = np.array(images)
    labels = np.array(labels)

    # Shuffle
    indices = np.arange(len(images))
    np.random.seed(42)
    np.random.shuffle(indices)
    images = images[indices]
    labels = labels[indices]

    # Split
    split_idx = int(len(images) * (1 - VALIDATION_SPLIT))
    X_train, X_val = images[:split_idx], images[split_idx:]
    y_train, y_val = labels[:split_idx], labels[split_idx:]

    print(f"\n📊 Split: {len(X_train)} train / {len(X_val)} validation")
    print(f"   Train fire ratio: {y_train.sum()/len(y_train)*100:.1f}%")
    print(f"   Val fire ratio:   {y_val.sum()/len(y_val)*100:.1f}%\n")

    return X_train, X_val, y_train, y_val

def build_model():
    """Build a MobileNetV2-based binary classifier."""
    import tensorflow as tf
    from tensorflow import keras
    from tensorflow.keras import layers

    # Use MobileNetV2 as feature extractor (pretrained on ImageNet)
    base_model = keras.applications.MobileNetV2(
        input_shape=(IMG_SIZE, IMG_SIZE, 3),
        include_top=False,
        weights='imagenet'
    )
    base_model.trainable = False  # Freeze base layers

    model = keras.Sequential([
        base_model,
        layers.GlobalAveragePooling2D(),
        layers.Dropout(0.3),
        layers.Dense(64, activation='relu'),
        layers.Dropout(0.2),
        layers.Dense(1, activation='sigmoid')  # Binary: fire vs no fire
    ])

    model.compile(
        optimizer=keras.optimizers.Adam(learning_rate=LEARNING_RATE),
        loss='binary_crossentropy',
        metrics=['accuracy']
    )

    model.summary()
    return model, base_model

def train(model, base_model, X_train, X_val, y_train, y_val):
    """Train the model with class weight balancing."""
    import tensorflow as tf
    from tensorflow import keras

    # Handle class imbalance (more non-fire than fire images)
    n_no_fire = (y_train == 0).sum()
    n_fire = (y_train == 1).sum()
    total = len(y_train)
    class_weights = {
        0: total / (2 * n_no_fire),
        1: total / (2 * n_fire)
    }
    print(f"⚖️  Class weights: No Fire={class_weights[0]:.2f}, Fire={class_weights[1]:.2f}\n")

    # Phase 1: Train top layers (base frozen)
    print("🏋️  Phase 1: Training classification head...")
    history1 = model.fit(
        X_train, y_train,
        validation_data=(X_val, y_val),
        epochs=EPOCHS,
        batch_size=BATCH_SIZE,
        class_weight=class_weights,
        verbose=1
    )

    # Phase 2: Fine-tune last 30 layers of MobileNetV2
    print("\n🏋️  Phase 2: Fine-tuning base model...")
    base_model.trainable = True
    for layer in base_model.layers[:-30]:
        layer.trainable = False

    model.compile(
        optimizer=keras.optimizers.Adam(learning_rate=LEARNING_RATE / 10),
        loss='binary_crossentropy',
        metrics=['accuracy']
    )

    history2 = model.fit(
        X_train, y_train,
        validation_data=(X_val, y_val),
        epochs=5,
        batch_size=BATCH_SIZE,
        class_weight=class_weights,
        verbose=1
    )

    return model

def evaluate(model, X_val, y_val):
    """Print evaluation metrics."""
    loss, accuracy = model.evaluate(X_val, y_val, verbose=0)
    print(f"\n📈 Final Results:")
    print(f"   Validation Accuracy: {accuracy*100:.1f}%")
    print(f"   Validation Loss:     {loss:.4f}")

    # Detailed per-class metrics
    y_pred = (model.predict(X_val, verbose=0) > 0.5).astype(int).flatten()
    tp = ((y_pred == 1) & (y_val == 1)).sum()
    tn = ((y_pred == 0) & (y_val == 0)).sum()
    fp = ((y_pred == 1) & (y_val == 0)).sum()
    fn = ((y_pred == 0) & (y_val == 1)).sum()

    precision = tp / (tp + fp) if (tp + fp) > 0 else 0
    recall = tp / (tp + fn) if (tp + fn) > 0 else 0

    print(f"   Precision (Fire):    {precision*100:.1f}%")
    print(f"   Recall (Fire):       {recall*100:.1f}%")
    print(f"   Confusion: TP={tp} TN={tn} FP={fp} FN={fn}\n")

    return accuracy

def save_model(model):
    """Save Keras model and convert to TensorFlow.js."""
    import subprocess

    MODEL_DIR.mkdir(parents=True, exist_ok=True)
    TFJS_DIR.mkdir(parents=True, exist_ok=True)

    # Save Keras model
    keras_path = MODEL_DIR / "fire_detector.keras"
    model.save(keras_path)
    print(f"💾 Keras model saved: {keras_path}")

    # Convert to TensorFlow.js using CLI (avoids numpy import issues)
    print(f"🔄 Converting to TensorFlow.js...")
    result = subprocess.run([
        sys.executable, "-m", "tensorflowjs.converters.keras_tfjs_loader",
        "--input_model_file", str(keras_path),
        "--output_dir", str(TFJS_DIR)
    ], capture_output=True, text=True)

    # Fallback: try the tensorflowjs_converter CLI
    if result.returncode != 0:
        print("   Trying alternate converter...")
        result = subprocess.run([
            "tensorflowjs_converter",
            "--input_format", "keras",
            str(keras_path),
            str(TFJS_DIR)
        ], capture_output=True, text=True)

    # Fallback 2: manual TF SavedModel → TF.js
    if result.returncode != 0:
        print("   Trying SavedModel conversion path...")
        saved_model_dir = MODEL_DIR / "saved_model"
        import tensorflow as tf
        tf.saved_model.save(model, str(saved_model_dir))
        result = subprocess.run([
            "tensorflowjs_converter",
            "--input_format", "tf_saved_model",
            str(saved_model_dir),
            str(TFJS_DIR)
        ], capture_output=True, text=True)

    if result.returncode != 0:
        print(f"   ⚠️  TF.js conversion error: {result.stderr[:500]}")
        print(f"   Keras model still saved. Run manually:")
        print(f"   tensorflowjs_converter --input_format keras {keras_path} {TFJS_DIR}")
    else:
        print(f"✅ TF.js model saved: {TFJS_DIR}")
        total_size = sum(f.stat().st_size for f in TFJS_DIR.rglob('*') if f.is_file())
        print(f"   Model size: {total_size / (1024*1024):.1f} MB\n")

def main():
    print("=" * 55)
    print("  🔥 FireGuard ML — Fire Image Detection Training")
    print("=" * 55)

    # Step 1: Check dataset
    check_dataset()

    # Step 2: Load and preprocess
    X_train, X_val, y_train, y_val = load_and_preprocess()

    # Step 3: Build model
    model, base_model = build_model()

    # Step 4: Train
    model = train(model, base_model, X_train, X_val, y_train, y_val)

    # Step 5: Evaluate
    accuracy = evaluate(model, X_val, y_val)

    # Step 6: Save and convert
    save_model(model)

    print("=" * 55)
    print(f"  ✅ Training complete! Accuracy: {accuracy*100:.1f}%")
    print(f"  Open index.html → Simulate → upload an image to test")
    print("=" * 55)

if __name__ == "__main__":
    main()
