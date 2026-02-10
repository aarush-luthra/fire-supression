#!/usr/bin/env python3
"""
Test Script for FireGuard ML Model
Usage: python3 ml/test_model.py [image_path]
"""

import os
import sys
import numpy as np
import tensorflow as tf
from tensorflow import keras
from PIL import Image
from pathlib import Path

# Configuration
IMG_SIZE = 128
SCRIPT_DIR = Path(__file__).parent.resolve()
MODEL_PATH = SCRIPT_DIR / "model" / "fire_detector.keras"
DEFAULT_IMAGE = SCRIPT_DIR.parent / "fire_dataset" / "1" / "1.jpg" # Default to a fire image

def load_model():
    if not MODEL_PATH.exists():
        print(f"❌ Model not found at: {MODEL_PATH}")
        print("   Run ml/train.py first to train and save the model.")
        sys.exit(1)
    
    print(f"🔄 Loading model from {MODEL_PATH}...")
    try:
        model = keras.models.load_model(MODEL_PATH)
        return model
    except Exception as e:
        print(f"❌ Error loading model: {e}")
        sys.exit(1)

def preprocess_image(image_path):
    if not os.path.exists(image_path):
        print(f"❌ Image not found at: {image_path}")
        sys.exit(1)
        
    try:
        img = Image.open(image_path).convert('RGB')
        img = img.resize((IMG_SIZE, IMG_SIZE))
        img_array = np.array(img, dtype=np.float32) / 255.0
        img_array = np.expand_dims(img_array, axis=0) # Add batch dimension
        return img_array, img
    except Exception as e:
        print(f"❌ Error processing image: {e}")
        sys.exit(1)

def main():
    # Determine image path
    if len(sys.argv) > 1:
        image_path = sys.argv[1]
    else:
        print(f"ℹ️  No image provided, using default: {DEFAULT_IMAGE}")
        image_path = DEFAULT_IMAGE

    # Load model
    model = load_model()

    # Preprocess image
    img_array, original_img = preprocess_image(image_path)

    # Predict
    print(f"🔍 Analyzing image: {image_path}")
    prediction = model.predict(img_array, verbose=0)[0][0]
    
    # Output results
    threshold = 0.5
    is_fire = prediction > threshold
    confidence = prediction if is_fire else 1 - prediction
    
    print("-" * 30)
    print(f"🔥 Prediction: {'FIRE DETECTED' if is_fire else 'NO FIRE'}")
    print(f"📊 Confidence: {confidence*100:.2f}%")
    print(f"🔢 Raw Score:  {prediction:.4f}")
    print("-" * 30)

if __name__ == "__main__":
    main()
