# DITMC PlatformIO Implementation - Quick Start

## 🎯 What You Have

Complete DITMC implementation for ESP32-S3 using PlatformIO with your existing configuration.

---

## 📁 Project Structure

```
DITMC_PlatformIO/
├── platformio.ini              # PlatformIO configuration (3 environments)
├── include/
│   ├── config.h                # Main configuration (SET RECEIVER MAC HERE!)
│   ├── get_mac.h               # MAC address detector module
│   ├── transmitter.h           # Transmitter module
│   └── receiver.h              # Receiver module
└── ditmc_node/
    └── main.cpp                # Main entry point (conditional compilation)
```

---

## 🚀 Setup Steps (30 minutes)

### STEP 1: Get MAC Addresses (10 minutes)

#### On Laptop 1 (Transmitter ESP32):
```bash
# Connect first ESP32-S3
# Build and upload MAC detector
pio run -e get_mac -t upload -t monitor

# You'll see output like:
# 📡 Your ESP32 MAC Address:
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# AA:BB:CC:DD:EE:FF
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# 
# For include/config.h, update these defines:
# #define RECEIVER_MAC_0 0xAA
# #define RECEIVER_MAC_1 0xBB
# ...

# WRITE DOWN this MAC address!
# Label this ESP32 as "TRANSMITTER"
```

#### On Laptop 2 (Receiver ESP32):
```bash
# Connect second ESP32-S3
# Build and upload MAC detector
pio run -e get_mac -t upload -t monitor

# WRITE DOWN this MAC address!
# Label this ESP32 as "RECEIVER"
```

**Record MAC Addresses:**
- Transmitter MAC: `__ : __ : __ : __ : __ : __`
- Receiver MAC: `__ : __ : __ : __ : __ : __` ← You need this one!

---

### STEP 2: Configure Receiver MAC (2 minutes)

Edit `include/config.h`:

```cpp
// REPLACE THESE WITH YOUR RECEIVER'S MAC ADDRESS!
#define RECEIVER_MAC_0 0xAA  // Replace with actual values
#define RECEIVER_MAC_1 0xBB
#define RECEIVER_MAC_2 0xCC
#define RECEIVER_MAC_3 0xDD
#define RECEIVER_MAC_4 0xEE
#define RECEIVER_MAC_5 0xFF
```

**Example:**
If Receiver MAC is `A4:CF:12:34:56:78`, then:
```cpp
#define RECEIVER_MAC_0 0xA4
#define RECEIVER_MAC_1 0xCF
#define RECEIVER_MAC_2 0x12
#define RECEIVER_MAC_3 0x34
#define RECEIVER_MAC_4 0x56
#define RECEIVER_MAC_5 0x78
```

---

### STEP 3: Build and Upload Transmitter (5 minutes)

#### On Laptop 1:
```bash
# Build transmitter
pio run -e esp32s3_transmitter

# Upload and monitor
pio run -e esp32s3_transmitter -t upload -t monitor

# You should see:
# ╔════════════════════════════════════════╗
# ║   DITMC Transmitter - ESP32-S3         ║
# ╚════════════════════════════════════════╝
# 
# 📡 Transmitter MAC Address: XX:XX:XX:XX:XX:XX
# ✓ ESP-NOW initialized
# ✓ Receiver registered: AA:BB:CC:DD:EE:FF
# 🚀 Starting DITMC transmission...
```

---

### STEP 4: Build and Upload Receiver (5 minutes)

#### On Laptop 2:
```bash
# Build receiver
pio run -e esp32s3_receiver

# Upload and monitor
pio run -e esp32s3_receiver -t upload -t monitor

# You should see:
# ╔════════════════════════════════════════╗
# ║    DITMC Receiver - ESP32-S3           ║
# ╚════════════════════════════════════════╝
# 
# 📡 Receiver MAC Address: AA:BB:CC:DD:EE:FF
# ✓ ESP-NOW initialized
# ✓ Receiver ready
# 🎧 Waiting for packets...
```

---

### STEP 5: Verify Communication (5 minutes)

#### Transmitter Output:
```
✓ Packet sent: 156 voice bytes [INTERLEAVED]
→ Repetition detected: Interleaving data
→ Silence detected: Interleaving data

╔════════════════════════════════════════╗
║      DITMC STATISTICS (Transmitter)    ║
╠════════════════════════════════════════╣
║ Voice bytes sent:       5234           ║
║ Interleaved data bytes: 3421           ║
║ Interleaving %:         47.32%         ║ ← Target!
║ Overhead %:             0.45%          ║
╚════════════════════════════════════════╝
```

#### Receiver Output:
```
📦 Packet received [#142]: 156 bytes [INTERLEAVED]
→ BATTERY OFF detected: Entering data mode
Voice: 98 bytes | Data: 54 bytes

╔════════════════════════════════════════╗
║      DITMC STATISTICS (Receiver)       ║
╠════════════════════════════════════════╣
║ Packets received:       142            ║
║ Data percentage:        39.82%         ║
║ Data sample: DATAINTRDATAINTR...       ║
╚════════════════════════════════════════╝
```

---

## 🎯 PlatformIO Commands Cheat Sheet

### Get MAC Address:
```bash
pio run -e get_mac -t upload -t monitor
```

### Build Transmitter:
```bash
pio run -e esp32s3_transmitter
```

### Upload Transmitter:
```bash
pio run -e esp32s3_transmitter -t upload
```

### Upload + Monitor Transmitter:
```bash
pio run -e esp32s3_transmitter -t upload -t monitor
```

### Build Receiver:
```bash
pio run -e esp32s3_receiver
```

### Upload + Monitor Receiver:
```bash
pio run -e esp32s3_receiver -t upload -t monitor
```

### Clean Build:
```bash
pio run -t clean
```

### Monitor Only (after upload):
```bash
pio device monitor
```

---

## 🔧 Troubleshooting

### Issue: "Error: Could not find the package with 'espressif32' requirements"
**Solution:**
```bash
pio pkg update
pio platform update espressif32
```

### Issue: Upload fails
**Solution:**
```bash
# Hold BOOT button while uploading
# Or add to platformio.ini:
upload_flags = 
  --before=default_reset
  --after=hard_reset
```

### Issue: Serial monitor shows garbage
**Solution:**
- Ensure monitor_speed = 115200 in platformio.ini (already set)
- Press RESET button after upload
- Check USB cable (use data cable, not charge-only)

### Issue: "ESP-NOW init failed"
**Solution:**
- Verify ARDUINO_USB_CDC_ON_BOOT=1 in build flags (already set)
- Try different USB port
- Press RESET button

### Issue: Transmitter says "Send failed"
**Solution:**
1. Verify Receiver MAC in `include/config.h`
2. Ensure both ESP32s are powered on
3. Press RESET on both devices
4. Move ESP32s closer together

---

## 📊 Configuration Options

### In `include/config.h`:

```cpp
// === CRITICAL SETTING ===
// Set to your RECEIVER's MAC address!
#define RECEIVER_MAC_0 0xXX  // ← CHANGE THESE!
#define RECEIVER_MAC_1 0xXX
// ... etc

// === ALGORITHM TUNING ===
#define REPETITION_THRESHOLD 4    // 3-8: Lower = more interleaving
#define SAMPLE_BUFFER_SIZE 8      // 4-16: Detection window
#define SEND_INTERVAL_MS 50       // 10-100: Packet rate

// === DEBUG ===
#define ENABLE_DEBUG true         // Detailed output
#define STATS_INTERVAL 5000       // Statistics every 5 seconds

// === SIMULATION ===
#define USE_SIMULATED_VOICE true  // Use test data
#define SIMULATION_DURATION 60000 // Run for 60 seconds
```

---

## 🎓 PlatformIO vs Arduino IDE

### Advantages of PlatformIO:
- ✅ Better dependency management
- ✅ Easier multi-environment builds
- ✅ Command-line interface
- ✅ Better for version control
- ✅ Unified configuration

### Your Setup:
```ini
[platformio]
default_envs = esp32s3_transmitter  # Change as needed

[env:esp32s3_transmitter]  # Upload to transmitter
[env:esp32s3_receiver]     # Upload to receiver  
[env:get_mac]              # Get MAC addresses
```

Switch environments easily:
```bash
pio run -e esp32s3_transmitter -t upload
pio run -e esp32s3_receiver -t upload
```

---

## ✅ Success Checklist

- [ ] Both ESP32s programmed with get_mac
- [ ] Both MAC addresses recorded
- [ ] ESP32s labeled (Transmitter/Receiver)
- [ ] Receiver MAC configured in config.h
- [ ] Transmitter uploaded successfully
- [ ] Receiver uploaded successfully
- [ ] Both show "initialized" messages
- [ ] Transmitter shows "Receiver registered"
- [ ] Receiver shows "Packet received"
- [ ] Interleaving percentage 30-50%
- [ ] System runs stable for 60+ seconds

---

## 🎯 Quick Command Summary

**First Time Setup:**
```bash
# 1. Get transmitter MAC
pio run -e get_mac -t upload -t monitor

# 2. Get receiver MAC  
pio run -e get_mac -t upload -t monitor

# 3. Edit include/config.h with receiver MAC

# 4. Upload transmitter
pio run -e esp32s3_transmitter -t upload -t monitor

# 5. Upload receiver (in new terminal/laptop)
pio run -e esp32s3_receiver -t upload -t monitor
```

**Daily Use:**
```bash
# Laptop 1 - Transmitter
pio run -e esp32s3_transmitter -t upload -t monitor

# Laptop 2 - Receiver
pio run -e esp32s3_receiver -t upload -t monitor
```

---

## 🚀 You're Ready!

Total setup time: **~30 minutes**

The code is optimized for your PlatformIO setup and ready to go!

**Start with Step 1 (Get MAC Addresses) and follow in order.**
