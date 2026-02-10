# FireGuard - Adaptive Edge-Based Fire Risk System

FireGuard is an intelligent fire detection system that combines IoT hardware (ESP32) with a machine learning backend to detect fire and gas hazards in real-time.

## Features

- **Multi-Sensor Fusion**: Combines data from flame and gas sensors using a statistical risk model.
- **Adaptive Baseline**: Learns the "normal" environment levels to reduce false alarms.
- **ML-Powered Verification**: Uses a MobileNetV2-based computer vision model to verify fire incidents via camera feed (simulated).
- **Real-Time Dashboard**: Web interface for monitoring sensor data and system status.

## Hardware Components

- **ESP32 Development Board**
- **Flame Sensor** (Digital/Analog)
- **MQ-2 Gas/Smoke Sensor**
- **Buzzer & LED** for alerts

## Software Architecture

- **Firmware (`src/`)**: C++ code for ESP32 using PlatformIO. Handles sensor reading, risk calculation, and state management.
- **ML Backend (`ml/`)**: Python Flask server hosting a TensorFlow model for image-based fire detection.
- **Dashboard (`ml/data/`)**: HTML/JS frontend for visualizing data.

## Getting Started

### 1. Firmware Setup (ESP32)

1.  Open the project in **VS Code** with the **PlatformIO** extension installed.
2.  Connect your ESP32 board.
3.  Upload the firmware:
    ```bash
    pio run --target upload
    ```
4.  Monitor serial output:
    ```bash
    pio device monitor
    ```

### 2. ML Backend Setup

The backend serves the fire detection model and handles image classification requests.

1.  Install dependencies:
    ```bash
    pip3 install -r requirements.txt
    ```

2.  Start the backend server:
    ```bash
    python3 ml/backend.py
    ```
    The server will run on `http://localhost:5001`.

### 3. Dashboard Setup

To view the dashboard and test the ML model in the browser:

1.  Serve the `ml/data` directory:
    ```bash
    python3 serve.py
    ```
2.  Open your browser to `http://localhost:8080`.

## Project Structure

- `src/`: ESP32 firmware source code.
- `ml/`: Machine learning model training and inference scripts.
- `ml/data/`: Web dashboard and TensorFlow.js model files.
- `platformio.ini`: PlatformIO configuration.
- `serve.py`: Simple HTTP server for the dashboard.
