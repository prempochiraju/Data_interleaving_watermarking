# 🎉 DITMC PlatformIO Project - Ready to Use!

## ✅ What You Have

I've adapted the complete DITMC implementation specifically for **PlatformIO** with your existing configuration. Everything is ready to upload to your ESP32-S3 boards!

---

## 📦 Project Files

```
DITMC_PlatformIO/
├── platformio.ini              ← 3 build environments
├── README.md                   ← Project overview
├── PLATFORMIO_QUICKSTART.md    ← START HERE! Complete guide
├── include/
│   ├── config.h                ← Configure receiver MAC here!
│   ├── get_mac.h               ← MAC address detector
│   ├── transmitter.h           ← Transmitter implementation
│   └── receiver.h              ← Receiver implementation
└── ditmc_node/
    └── main.cpp                ← Entry point (conditional build)
```

---

## 🚀 Super Quick Start (5 Commands!)

### 1️⃣ Get MAC Addresses (Both ESP32s)
```bash
# Connect first ESP32
pio run -e get_mac -t upload -t monitor
# Write down MAC → Label as "TRANSMITTER"

# Connect second ESP32
pio run -e get_mac -t upload -t monitor
# Write down MAC → Label as "RECEIVER"
```

### 2️⃣ Configure (Edit include/config.h)
```cpp
// Replace with YOUR receiver's MAC:
#define RECEIVER_MAC_0 0xXX
#define RECEIVER_MAC_1 0xXX
#define RECEIVER_MAC_2 0xXX
#define RECEIVER_MAC_3 0xXX
#define RECEIVER_MAC_4 0xXX
#define RECEIVER_MAC_5 0xXX
```

### 3️⃣ Upload Transmitter (Laptop 1)
```bash
pio run -e esp32s3_transmitter -t upload -t monitor
```

### 4️⃣ Upload Receiver (Laptop 2)
```bash
pio run -e esp32s3_receiver -t upload -t monitor
```

### 5️⃣ Watch It Work! 🎊
Both serial monitors will show statistics every 5 seconds with ~40-50% interleaving!

---

## 🎯 Three Build Environments

Your `platformio.ini` has 3 environments ready to use:

### 1. `get_mac` - MAC Address Detector
```bash
pio run -e get_mac -t upload -t monitor
```
**Use this FIRST** to get both ESP32 MAC addresses

### 2. `esp32s3_transmitter` - Transmitter
```bash
pio run -e esp32s3_transmitter -t upload -t monitor
```
Implements DITMC algorithm, detects redundancy, interleaves data

### 3. `esp32s3_receiver` - Receiver  
```bash
pio run -e esp32s3_receiver -t upload -t monitor
```
Receives packets, separates voice/data, shows statistics

---

## 🔑 Key Differences from Arduino IDE Version

### Single Codebase
- ✅ One `main.cpp` with conditional compilation
- ✅ Switch between modes with build environment
- ✅ No need to switch sketches manually

### PlatformIO Configuration
```ini
[platformio]
src_dir = ditmc_node          # Your existing config!

[env:esp32s3_transmitter]
board = esp32-s3-devkitc-1    # Your board!
build_flags = 
  -DDITMC_MODE=TRANSMITTER    # Automatic mode selection
```

### Better Workflow
```bash
# Arduino IDE: Manual file switching
# PlatformIO: Just change -e flag!

pio run -e get_mac -t upload           # Upload MAC detector
pio run -e esp32s3_transmitter -t upload  # Upload transmitter
pio run -e esp32s3_receiver -t upload     # Upload receiver
```

---

## 📊 Expected Serial Monitor Output

### Transmitter (After 5 seconds):
```
╔════════════════════════════════════════╗
║   DITMC Transmitter - ESP32-S3         ║
╚════════════════════════════════════════╝

📡 Transmitter MAC Address: AA:BB:CC:DD:EE:FF
✓ ESP-NOW initialized
✓ Receiver registered: 11:22:33:44:55:66
🚀 Starting DITMC transmission...

✓ Packet sent: 148 voice bytes [INTERLEAVED]
→ Repetition detected: Interleaving data
→ Silence detected: Interleaving data

╔════════════════════════════════════════╗
║      DITMC STATISTICS (Transmitter)    ║
╠════════════════════════════════════════╣
║ Voice bytes sent:       4892           ║
║ Redundant bytes:        2341           ║
║ Interleaved data bytes: 3567           ║
║ Interleaving %:         42.15%         ║ ← Success!
║ Overhead %:             0.52%          ║
╠════════════════════════════════════════╣
║ BATTERY OFF codes:      89             ║
║ BATTERY ON codes:       85             ║
║ Packets sent:           98             ║
║ Send failures:          0              ║
╚════════════════════════════════════════╝
```

### Receiver (After 5 seconds):
```
╔════════════════════════════════════════╗
║    DITMC Receiver - ESP32-S3           ║
╚════════════════════════════════════════╝

📡 Receiver MAC Address: 11:22:33:44:55:66
⚠️  Copy this address to include/config.h!

✓ ESP-NOW initialized
✓ Receiver ready
🎧 Waiting for packets...

📦 Packet received [#1]: 148 bytes [INTERLEAVED]
  → BATTERY OFF detected: Entering data mode
  Voice: 94 bytes | Data: 52 bytes

╔════════════════════════════════════════╗
║      DITMC STATISTICS (Receiver)       ║
╠════════════════════════════════════════╣
║ Packets received:       98             ║
║ Voice bytes received:   4892           ║
║ Data bytes received:    3567           ║
║ Data percentage:        42.15%         ║ ← Success!
╠════════════════════════════════════════╣
║ BATTERY OFF received:   89             ║
║ BATTERY ON received:    85             ║
║ Reconstruction rate:    100.00%        ║
╠════════════════════════════════════════╣
║ Data sample: DATAINTRDATAINTR...       ║
╚════════════════════════════════════════╝
```

---

## ⚙️ Customization Options

All in `include/config.h`:

```cpp
// === ALGORITHM PARAMETERS ===
#define REPETITION_THRESHOLD 4    // 3-8: Lower = more interleaving
#define SAMPLE_BUFFER_SIZE 8      // 4-16: Detection window size
#define SEND_INTERVAL_MS 50       // 10-100: Packet transmission rate

// === COMMUNICATION ===
#define WIFI_CHANNEL 1            // 1-13: WiFi channel
#define MAX_PACKET_SIZE 250       // ESP-NOW max packet size

// === DEBUG ===
#define ENABLE_DEBUG true         // Show detailed messages
#define STATS_INTERVAL 5000       // Statistics every 5 seconds

// === TEST MODE ===
#define USE_SIMULATED_VOICE true  // Generate test voice data
#define SIMULATION_DURATION 60000 // Run for 60 seconds
```

---

## 🔧 Troubleshooting

### "Error: Unknown development platform 'espressif32'"
```bash
pio pkg update
pio platform install espressif32
```

### Upload fails / Timeout
```bash
# Hold BOOT button during upload
# Or press BOOT, then RESET, then release both
```

### "ESP-NOW init failed"
- Already configured correctly in platformio.ini!
- Just press RESET button after upload

### Transmitter shows "Send failed"
1. Double-check receiver MAC in `include/config.h`
2. Ensure receiver is powered on and running
3. Press RESET on both ESP32s
4. Move devices closer (start at 1 meter)

### No output in serial monitor
- PlatformIO monitor: `pio device monitor`
- Or use `-t monitor` flag with upload
- Baud rate already set to 115200 in platformio.ini

---

## 📋 Complete Workflow

### First Time Setup:
```bash
# 1. Clone/download project
cd DITMC_PlatformIO

# 2. Get MAC addresses (both ESP32s)
pio run -e get_mac -t upload -t monitor

# 3. Edit include/config.h with receiver MAC

# 4. Upload to both ESP32s
pio run -e esp32s3_transmitter -t upload -t monitor  # Laptop 1
pio run -e esp32s3_receiver -t upload -t monitor     # Laptop 2

# 5. Watch the statistics!
```

### Daily Use:
```bash
# Just upload and monitor
pio run -e esp32s3_transmitter -t upload -t monitor
pio run -e esp32s3_receiver -t upload -t monitor
```

---

## ✅ Success Checklist

- [ ] Project downloaded to both laptops
- [ ] PlatformIO installed (if not: `pip install platformio`)
- [ ] Both ESP32s connected via USB
- [ ] MAC addresses obtained and recorded
- [ ] Receiver MAC configured in `include/config.h`
- [ ] Transmitter uploaded successfully
- [ ] Receiver uploaded successfully
- [ ] Both show "initialized" messages
- [ ] Packets being sent/received
- [ ] Interleaving percentage 30-50%
- [ ] Statistics updating every 5 seconds

---

## 🎯 Timeline

- **MAC Address Detection**: 5 minutes (both ESP32s)
- **Configuration**: 2 minutes (edit config.h)
- **Transmitter Upload**: 3 minutes
- **Receiver Upload**: 3 minutes
- **Verification**: 5 minutes

**Total: ~20 minutes!** (Faster than Arduino IDE!)

---

## 📖 Documentation Priority

1. **PLATFORMIO_QUICKSTART.md** ← Read this first!
2. **include/config.h** ← Configure receiver MAC
3. **README.md** ← Project overview
4. **platformio.ini** ← Build configuration

---

## 🎓 What You'll Learn

- ✅ PlatformIO project structure
- ✅ Multi-environment builds
- ✅ ESP-NOW wireless communication
- ✅ Data interleaving algorithms
- ✅ Mobile network optimization
- ✅ Real-time statistics collection
- ✅ Embedded systems debugging

---

## 🚀 Ready to Start!

**Everything is configured for your PlatformIO setup:**
- ✓ Uses your `src_dir = ditmc_node`
- ✓ Compatible with `esp32-s3-devkitc-1`
- ✓ All build flags included
- ✓ Monitor speed set to 115200
- ✓ USB CDC configured correctly

**Just 3 steps:**
1. Get MAC addresses
2. Configure receiver MAC
3. Upload to both ESP32s

**Open `PLATFORMIO_QUICKSTART.md` and follow along!** 🎉

---

## 💡 Pro Tips

### Single Laptop Workflow:
```bash
# You can use one laptop and switch USB cables:
pio run -e get_mac -t upload -t monitor          # First ESP32
# Switch USB cable
pio run -e get_mac -t upload -t monitor          # Second ESP32
# Edit config.h
pio run -e esp32s3_transmitter -t upload -t monitor
# Switch USB cable  
pio run -e esp32s3_receiver -t upload -t monitor
```

### Monitor Without Upload:
```bash
pio device monitor -b 115200
# Or just:
pio device monitor  # Uses platformio.ini settings
```

### Quick Rebuild:
```bash
pio run -e esp32s3_transmitter  # Build only
pio run -t upload                # Upload only
pio run -t monitor               # Monitor only
```

---

Your PlatformIO DITMC implementation is complete and ready to use! 🚀
