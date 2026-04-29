# DITMC ESP32-S3 Setup Guide

## Table of Contents
1. [Computer Setup](#1-computer-setup)
2. [Arduino IDE Configuration](#2-arduino-ide-configuration)
3. [ESP32 Connection](#3-esp32-connection)
4. [Code Upload](#4-code-upload)
5. [Testing Communication](#5-testing-communication)
6. [Monitoring Results](#6-monitoring-results)
7. [Troubleshooting](#7-troubleshooting)

---

## 1. Computer Setup

### Laptop 1 (Transmitter)
**Purpose**: Program and monitor the transmitting ESP32

1. Install Arduino IDE 2.0+ or PlatformIO
2. Install USB-to-Serial drivers (usually automatic for ESP32-S3)
3. Connect Transmitter ESP32 via USB-C cable

### Laptop 2 (Receiver)  
**Purpose**: Program and monitor the receiving ESP32

1. Install Arduino IDE 2.0+ or PlatformIO
2. Install USB-to-Serial drivers
3. Connect Receiver ESP32 via USB-C cable

---

## 2. Arduino IDE Configuration

### On BOTH Laptops:

#### Step 2.1: Install ESP32 Board Support
```
1. Open Arduino IDE
2. File → Preferences
3. In "Additional Board Manager URLs", add:
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
4. Click OK
5. Tools → Board → Boards Manager
6. Search: "esp32"
7. Install: "esp32 by Espressif Systems" (version 2.0.11 or later)
```

#### Step 2.2: Configure Board Settings
```
Tools → Board → esp32 → ESP32S3 Dev Module

Critical Settings:
- USB CDC On Boot: "Enabled"
- CPU Frequency: "240MHz (WiFi)"
- Flash Mode: "QIO 80MHz"
- Flash Size: "8MB (64Mb)"
- Partition Scheme: "Default 4MB with spiffs"
- PSRAM: "OPI PSRAM" (if your board has PSRAM)
- Upload Speed: "921600"
```

---

## 3. ESP32 Connection

### Identifying Your ESP32-S3

1. **Connect ESP32 to USB port**
2. **Check Device Manager (Windows)** or **ls /dev/tty* (Linux/Mac)**
   - Windows: Look for "USB Serial Device (COMx)"
   - Linux: Look for "/dev/ttyUSB0" or "/dev/ttyACM0"
   - Mac: Look for "/dev/cu.usbserial-xxxxx"

3. **Select Port in Arduino IDE**:
   ```
   Tools → Port → [Your COM/tty port]
   ```

### Getting MAC Address (Important!)

Upload this sketch to BOTH ESP32s to get their MAC addresses:

```cpp
#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  WiFi.mode(WIFI_STA);
  Serial.println("ESP32 MAC Address:");
  Serial.println(WiFi.macAddress());
}

void loop() {
  delay(1000);
}
```

**Record these MAC addresses - you'll need them!**
- Transmitter MAC: ___:___:___:___:___:___
- Receiver MAC: ___:___:___:___:___:___

---

## 4. Code Upload

### Step 4.1: Prepare Transmitter Code

1. Open `transmitter/transmitter.ino` on Laptop 1
2. Open `transmitter/config.h`
3. **Update the receiver MAC address**:
   ```cpp
   uint8_t receiverMAC[] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX};
   ```
   Replace with Receiver's actual MAC address

4. Verify code: Click ✓ (Verify button)
5. Upload: Click → (Upload button)
6. Wait for "Done uploading" message

### Step 4.2: Prepare Receiver Code

1. Open `receiver/receiver.ino` on Laptop 2
2. Verify code: Click ✓
3. Upload: Click →
4. Wait for "Done uploading"

---

## 5. Testing Communication

### Initial Test - ESP-NOW Connection

#### On Laptop 1 (Transmitter):
```
1. Tools → Serial Monitor (Ctrl+Shift+M)
2. Set baud rate: 115200
3. Press EN/Reset button on ESP32
4. You should see:
   - "Initializing DITMC Transmitter..."
   - "ESP-NOW Init Success"
   - Transmitter MAC address
```

#### On Laptop 2 (Receiver):
```
1. Tools → Serial Monitor
2. Set baud rate: 115200
3. Press EN/Reset button on ESP32
4. You should see:
   - "Initializing DITMC Receiver..."
   - "ESP-NOW Init Success"
   - Receiver MAC address
```

### Transmission Test

The transmitter will automatically start sending test voice data. On the receiver monitor, you should see:

```
✓ Packet received
  Voice bytes: XX
  Interleaved data bytes: XX
  Total: XXX bytes
```

---

## 6. Monitoring Results

### Key Metrics to Watch

#### Transmitter Output:
```
=== DITMC Statistics ===
Total voice bytes sent: XXXX
Redundant bytes detected: XXX
Data bytes interleaved: XXX
Interleaving percentage: XX.XX%
BATTERY OFF codes sent: XX
BATTERY ON codes sent: XX
========================
```

#### Receiver Output:
```
=== Reception Statistics ===
Total packets received: XXX
Voice bytes received: XXXX
Data bytes received: XXX
Reconstruction success rate: XX.XX%
============================
```

### Expected Performance

According to the paper:
- **Interleaving %**: ~47%
- **Channel utilization**: 83% (up from 36%)
- **Overhead**: <0.05%

---

## 7. Troubleshooting

### Problem: ESP32 Not Detected

**Solution**:
1. Try different USB cable (must be data cable, not charge-only)
2. Hold BOOT button while connecting USB
3. Install CH340 or CP2102 drivers manually
4. Try different USB port

### Problem: Upload Failed

**Solution**:
1. Hold BOOT button, press RESET, release RESET, release BOOT
2. Reduce upload speed: Tools → Upload Speed → 115200
3. Check port selection
4. Close Serial Monitor before uploading

### Problem: "Brownout detector was triggered"

**Solution**:
1. Use USB port with more power (avoid hubs)
2. Add external 5V power supply
3. Reduce CPU frequency temporarily

### Problem: ESP-NOW "Send Failed"

**Solution**:
1. Verify MAC addresses are correct
2. Ensure both ESP32s are powered on
3. Check WiFi channel compatibility
4. Move devices closer together
5. Ensure WiFi.mode(WIFI_STA) is called before ESP-NOW init

### Problem: No Data Received

**Solution**:
1. Verify both devices show "ESP-NOW Init Success"
2. Check Serial Monitor baud rate (115200)
3. Press RESET on both devices
4. Verify MAC addresses in config.h
5. Check for interference (move away from WiFi routers)

### Problem: Compilation Errors

**Solution**:
```
1. Verify ESP32 board package is installed
2. Check Arduino IDE version (use 2.0+)
3. Ensure all files are in correct directories
4. Clean build: Sketch → Clean
5. Restart Arduino IDE
```

---

## Communication Range Tips

### For ESP-NOW:
- **Indoor**: Up to 50 meters with clear line of sight
- **Outdoor**: Up to 200 meters
- **Interference**: Avoid 2.4GHz WiFi routers, microwaves

### Improving Range:
1. Use external antenna (if your board supports it)
2. Increase ESP-NOW transmission power in code
3. Position devices with clear line of sight
4. Minimize obstacles between devices

---

## Next Steps After Setup

1. ✓ Verify both devices communicate
2. ✓ Observe initial statistics
3. → Experiment with different voice data patterns
4. → Measure and compare with paper's results
5. → Document your findings

---

## Quick Reference: Button Functions

### ESP32-S3 Buttons:
- **BOOT**: Used during upload (hold to enter bootloader)
- **EN/RST**: Reset the board

### Upload Process:
1. Click Upload in Arduino IDE
2. When "Connecting..." appears, hold BOOT
3. Release BOOT when upload starts
4. (Some boards auto-reset, no button needed)

---

## Getting Help

If you encounter issues:

1. Check Serial Monitor output for error messages
2. Verify all connection settings
3. Try the basic MAC address test sketch first
4. Ensure proper power supply (quality USB cable)
5. Test each ESP32 individually before pair testing

Good luck with your implementation! 🚀
