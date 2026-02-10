#!/usr/bin/env python3
"""
FireGuard ML Backend
Serves the Fire Detection Model via HTTP API.
"""

import os
import sys
import numpy as np
import tensorflow as tf
from tensorflow import keras
from PIL import Image
from pathlib import Path
from flask import Flask, request, jsonify
from flask_cors import CORS

# Configuration
PORT = 5001
IMG_SIZE = 128
SCRIPT_DIR = Path(__file__).parent.resolve()
MODEL_PATH = SCRIPT_DIR / "model" / "fire_detector.keras"

app = Flask(__name__)
CORS(app)  # Enable CORS for all routes

# Load Model
print(f"🔄 Loading model from {MODEL_PATH}...")
try:
    model = keras.models.load_model(MODEL_PATH)
    print("✅ Model loaded successfully")
except Exception as e:
    print(f"❌ Error loading model: {e}")
    sys.exit(1)

def preprocess_image(image_file):
    """Preprocesses the uploaded image file for the model."""
    try:
        img = Image.open(image_file).convert('RGB')
        img = img.resize((IMG_SIZE, IMG_SIZE))
        img_array = np.array(img, dtype=np.float32) / 255.0
        img_array = np.expand_dims(img_array, axis=0)
        return img_array
    except Exception as e:
        print(f"❌ Error processing image: {e}")
        return None

@app.route('/predict', methods=['POST'])
def predict():
    if 'file' not in request.files:
        return jsonify({'error': 'No file uploaded'}), 400
    
    file = request.files['file']
    if file.filename == '':
        return jsonify({'error': 'No file selected'}), 400

    img_array = preprocess_image(file)
    if img_array is None:
        return jsonify({'error': 'Invalid image file'}), 400

    # Prediction
    prediction = model.predict(img_array, verbose=0)[0][0]
    
    # Check threshold
    threshold = 0.5
    is_fire = bool(prediction > threshold)
    confidence = float(prediction if is_fire else 1 - prediction)
    
    result = {
        'isFire': is_fire,
        'confidence': confidence,
        'rawScore': float(prediction)
    }
    
    print(f"🔍 Analysis: {result}")
    return jsonify(result)

@app.route('/', methods=['GET'])
def health_check():
    return jsonify({'status': 'running', 'model': 'loaded'}), 200

if __name__ == '__main__':
    print(f"🚀 Starting FireGuard Backend on port {PORT}...")
    app.run(host='0.0.0.0', port=PORT, debug=True)
