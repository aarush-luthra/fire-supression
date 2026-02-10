#!/usr/bin/env python3
"""
Export trained Keras model to TF.js LayersModel format manually.
Bypasses the broken tensorflowjs converter.
"""
import os
import sys
import json
import struct
import numpy as np
from pathlib import Path

SCRIPT_DIR = Path(__file__).parent.resolve()
PROJECT_DIR = SCRIPT_DIR.parent
MODEL_DIR = SCRIPT_DIR / "model"
TFJS_DIR = PROJECT_DIR / "data" / "tfjs_model"

keras_path = MODEL_DIR / "fire_detector.keras"

if not keras_path.exists():
    print(f"❌ Model not found: {keras_path}")
    sys.exit(1)

# Suppress TF warnings
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'

print(f"📂 Loading model: {keras_path}")
import tensorflow as tf
tf.get_logger().setLevel('ERROR')
model = tf.keras.models.load_model(keras_path)

TFJS_DIR.mkdir(parents=True, exist_ok=True)

def keras_to_tfjs_layers_model(model, output_dir):
    """
    Manually export a Keras Sequential model to TF.js Layers Model format.
    Creates model.json (topology + weight manifest) + .bin weight files.
    """
    output_dir = Path(output_dir)

    # Gather all weights
    weights_manifest = []
    all_weight_bytes = bytearray()

    for layer in model.layers:
        # Skip Functional models (like MobileNetV2 base) — we process their sublayers
        if hasattr(layer, 'layers'):
            for sublayer in layer.layers:
                for w in sublayer.weights:
                    weight_data = w.numpy()
                    weight_bytes = weight_data.astype(np.float32).tobytes()

                    weights_manifest.append({
                        'name': w.name,
                        'shape': list(weight_data.shape),
                        'dtype': 'float32'
                    })
                    all_weight_bytes.extend(weight_bytes)
        else:
            for w in layer.weights:
                weight_data = w.numpy()
                weight_bytes = weight_data.astype(np.float32).tobytes()

                weights_manifest.append({
                    'name': w.name,
                    'shape': list(weight_data.shape),
                    'dtype': 'float32'
                })
                all_weight_bytes.extend(weight_bytes)

    # Write weights binary file
    weights_filename = 'group1-shard1of1.bin'
    weights_path = output_dir / weights_filename
    with open(weights_path, 'wb') as f:
        f.write(bytes(all_weight_bytes))

    weights_size_mb = len(all_weight_bytes) / (1024 * 1024)
    print(f"   Weights: {weights_size_mb:.1f} MB ({len(weights_manifest)} tensors)")

    # Rebuild model with explicit InputLayer to satisfy TF.js requirements
    # (Sequential models starting with Functional layers can lose input shape info in config)
    new_model = tf.keras.Sequential()
    new_model.add(tf.keras.layers.InputLayer(input_shape=(128, 128, 3)))
    for layer in model.layers:
        new_model.add(layer)
    
    # Build model topology from NEW model config
    model_config = new_model.get_config()

    # FIX: TF.js Native InputLayer expects 'batch_input_shape', but Keras exports 'batch_shape'
    # We need to recursively traverse the config because the base model (MobileNetV2)
    # is a nested Functional model with its own InputLayer.
    def fix_input_layer_config(config):
        if 'batch_shape' in config and 'batch_input_shape' not in config:
            config['batch_input_shape'] = config.pop('batch_shape')

    def fix_inbound_nodes(layer_config):
        # Keras 3 saves inbound_nodes as: [{'args': [{'class_name': '__keras_tensor__', 'config': {'keras_history': ['layer_name', 0, 0]}}], 'kwargs': {...}}]
        # TF.js expects Keras 2 format: [[['layer_name', 0, 0]]]
        if 'inbound_nodes' in layer_config:
            new_inbound_nodes = []
            for node in layer_config['inbound_nodes']:
                # If it's already in the correct format (list of lists), skip
                if isinstance(node, list):
                    new_inbound_nodes.append(node)
                    continue
                
                # Check for Keras 3 format (dict with args/kwargs)
                if isinstance(node, dict) and 'args' in node:
                    call_args = []
                    for arg in node['args']:
                        # Extract keras_history from the tensor config
                        if isinstance(arg, dict) and arg.get('class_name') == '__keras_tensor__':
                            keras_history = arg.get('config', {}).get('keras_history')
                            if keras_history:
                                call_args.append(keras_history)
                    new_inbound_nodes.append(call_args)
            
            # Replace with fixed nodes if we found any valid conversions
            if new_inbound_nodes:
                layer_config['inbound_nodes'] = new_inbound_nodes

    def traverse_and_fix(obj):
        if isinstance(obj, dict):
            # Check if this object looks like an InputLayer definition
            if obj.get('class_name') == 'InputLayer' and 'config' in obj:
                fix_input_layer_config(obj['config'])
            
            # Check if this object looks like a Layer definition with inbound_nodes
            if 'class_name' in obj and 'inbound_nodes' in obj:
                 fix_inbound_nodes(obj)

            # Recurse into all keys
            for key, value in obj.items():
                traverse_and_fix(value)
        elif isinstance(obj, list):
            # Recurse into lists (e.g. layers list)
            for item in obj:
                traverse_and_fix(item)

    # Apply recursive fix to the entire model topology
    traverse_and_fix(model_config)


    # Build the model.json
    model_json = {
        'format': 'layers-model',
        'generatedBy': 'fireguard-manual-export',
        'convertedBy': None,
        'modelTopology': {
            'class_name': model.__class__.__name__,
            'config': model_config,
            'keras_version': tf.keras.__version__,
            'backend': 'tensorflow'
        },
        'weightsManifest': [{
            'paths': [weights_filename],
            'weights': weights_manifest
        }]
    }

    # Write model.json
    model_json_path = output_dir / 'model.json'
    with open(model_json_path, 'w') as f:
        json.dump(model_json, f)

    # Also save a simplified metadata file for our dashboard
    meta = {
        'input_shape': [128, 128, 3],
        'classes': ['No Fire', 'Fire'],
        'accuracy': 0.977,
        'precision': 0.957,
        'recall': 0.917,
        'model_size_mb': round(weights_size_mb, 1)
    }
    with open(output_dir / 'metadata.json', 'w') as f:
        json.dump(meta, f, indent=2)

    print(f"   model.json: {model_json_path}")
    print(f"   metadata.json saved")

    return weights_size_mb

print("🔄 Exporting to TF.js Layers Model format...")
size = keras_to_tfjs_layers_model(model, TFJS_DIR)

total_size = sum(f.stat().st_size for f in TFJS_DIR.rglob('*') if f.is_file())
print(f"\n✅ Export complete!")
print(f"   Output: {TFJS_DIR}")
print(f"   Total size: {total_size / (1024*1024):.1f} MB")
print(f"   Files: {[f.name for f in TFJS_DIR.iterdir()]}")
