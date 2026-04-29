# DITMC Implementation on ESP32-S3

Implementation of "Data Interleaving for Congestion Reduction in Mobile Traffic Transmission" using two ESP32-S3 WROOM-1 modules.

## Project Overview

This project implements the Data Interleaving Technique in Mobile Communication (DITMC) using:
- **Transmitter ESP32**: Simulates voice data transmission with interleaving
- **Receiver ESP32**: Receives and reconstructs the interleaved data

### Key Concepts from the Paper
- Detects repetition in voice bytes (≥4 repeated bytes)
- Inserts data during redundant/silent periods
- Uses Battery OFF/ON codes as markers
- Achieves ~47% channel utilization enhancement

## Hardware Requirements

### Per ESP32-S3 Module
- ESP32-S3 WROOM-1 development board
- USB-C cable for programming/power
- (Optional) LED for status indication
- (Optional) Breadboard and jumper wires

### Computers
- Laptop 1: For programming/monitoring Transmitter ESP32
- Laptop 2: For programming/monitoring Receiver ESP32

## Software Requirements

1. **Arduino IDE** (v2.0 or later) OR **PlatformIO**
2. **ESP32 Board Support Package**
3. **Required Libraries**:
   - WiFi.h (built-in)
   - esp_now.h (built-in for ESP-NOW)
   - OR WiFi.h for WiFi-based communication

## Project Structure

```
DITMC_ESP32_Implementation/
├── README.md (this file)
├── SETUP_GUIDE.md
├── transmitter/
│   ├── transmitter.ino
│   └── config.h
├── receiver/
│   ├── receiver.ino
│   └── config.h
├── docs/
│   └── DITMC_explanation.md
└── test_data/
    └── sample_voice_data.h
```

## Quick Start

### Step 1: Install Arduino IDE
1. Download from https://www.arduino.cc/en/software
2. Install for your operating system

### Step 2: Install ESP32 Board Support
1. Open Arduino IDE → File → Preferences
2. Add to "Additional Board Manager URLs":
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Go to Tools → Board → Boards Manager
4. Search "ESP32" and install "esp32 by Espressif Systems"

### Step 3: Select Board
1. Tools → Board → ESP32 Arduino → ESP32S3 Dev Module
2. Tools → USB CDC On Boot → Enabled
3. Tools → Port → Select your COM port

### Step 4: Upload and Monitor with PlatformIO

Run these commands from Windows PowerShell or Command Prompt.

**Transmitter ESP32-S3**

```powershell
cd C:\Users\pyppr\projects\DATA_inter_esp_new
C:\Users\pyppr\.platformio\penv\Scripts\platformio.exe run -e esp32s3_transmitter -t upload -t monitor
```

**Receiver ESP32-S3**

```powershell
cd C:\Users\pyppr\projects\DATA_inter_esp_new
C:\Users\pyppr\.platformio\penv\Scripts\platformio.exe run -e esp32s3_receiver -t upload -t monitor
```

### Arduino IDE Alternative
1. **On Laptop 1**: Upload the transmitter code
2. **On Laptop 2**: Upload the receiver code

## Communication Methods

This implementation supports two communication methods:

### Method 1: ESP-NOW (Recommended)
- Direct peer-to-peer communication
- No WiFi network required
- Lower latency
- Better for simulating GSM channel behavior

### Method 2: WiFi UDP
- Requires both ESP32s on same network
- Good for testing over longer distances
- Easier debugging with network tools

## Next Steps

1. Read `SETUP_GUIDE.md` for detailed setup
2. Review `docs/DITMC_explanation.md` for algorithm details
3. Upload and test with sample data
4. Monitor Serial output for results

## Expected Results

Based on the paper:
- **Channel Utilization**: 83.32% (up from 36%)
- **Interleaving Percentage**: ~47.32%
- **Overhead**: ~0.034%

## Troubleshooting

See `SETUP_GUIDE.md` Section 7 for common issues and solutions.

## License

This is an educational implementation of the research paper. Please cite the original authors if using for research purposes.
