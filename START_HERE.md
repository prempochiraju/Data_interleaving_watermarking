# YOUR COMPLETE PROJECT FILES

## 📁 Project Structure Created

```
DITMC_ESP32_Implementation/
│
├── 📘 README.md                          # Project overview
├── 🚀 QUICK_START.md                     # Fast setup checklist (START HERE!)
├── 📖 SETUP_GUIDE.md                     # Detailed setup instructions
│
├── 📂 docs/
│   └── DITMC_EXPLAINED.md                # Algorithm explanation
│
├── 📂 get_mac_address/
│   └── get_mac_address.ino               # First sketch to upload (get MAC addresses)
│
├── 📂 transmitter/
│   ├── transmitter.ino                   # Main transmitter code
│   └── config.h                          # Transmitter configuration
│
└── 📂 receiver/
    ├── receiver.ino                      # Main receiver code
    └── config.h                          # Receiver configuration
```

---

## 🎯 YOUR ACTION PLAN (Follow This Order!)

### STEP 1: Read Documentation (10 minutes)
1. ✅ Open `QUICK_START.md` - This is your roadmap
2. ✅ Skim `DITMC_EXPLAINED.md` - Understand what you're building
3. ✅ Bookmark `SETUP_GUIDE.md` - For troubleshooting

### STEP 2: Prepare Your Laptops (20 minutes)

#### On BOTH Laptops:
```
1. Install Arduino IDE 2.0+
   Download: https://www.arduino.cc/en/software

2. Install ESP32 Support:
   - Open Arduino IDE
   - File → Preferences
   - Add this URL to "Additional Board Manager URLs":
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   - Tools → Board → Boards Manager
   - Search "esp32"
   - Install "esp32 by Espressif Systems"

3. Configure Board:
   - Tools → Board → esp32 → ESP32S3 Dev Module
   - Tools → USB CDC On Boot → Enabled
   - Tools → Upload Speed → 921600
```

### STEP 3: Get MAC Addresses (10 minutes)

#### On Laptop 1 (Transmitter):
```
1. Connect first ESP32-S3 to USB
2. Select port: Tools → Port → [your COM port]
3. Open: get_mac_address/get_mac_address.ino
4. Upload (Click → button)
5. Open Serial Monitor (115200 baud)
6. WRITE DOWN the MAC address
7. LABEL this ESP32 as "TRANSMITTER"
```

#### On Laptop 2 (Receiver):
```
1. Connect second ESP32-S3 to USB
2. Select port: Tools → Port → [your COM port]
3. Open: get_mac_address/get_mac_address.ino
4. Upload
5. Open Serial Monitor (115200 baud)
6. WRITE DOWN the MAC address
7. LABEL this ESP32 as "RECEIVER"
```

**Record MAC Addresses Here:**
- Transmitter MAC: `__ : __ : __ : __ : __ : __`
- Receiver MAC: `__ : __ : __ : __ : __ : __`

### STEP 4: Configure and Upload Transmitter (5 minutes)

#### On Laptop 1:
```
1. Open: transmitter/config.h
2. Find line: uint8_t receiverMAC[] = {0xFF, ...}
3. Replace with RECEIVER's MAC address
   Example: {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}
4. Save file
5. Open: transmitter/transmitter.ino
6. Verify (✓) - Check for errors
7. Upload (→)
8. Wait for "Done uploading"
```

### STEP 5: Upload Receiver (3 minutes)

#### On Laptop 2:
```
1. Open: receiver/receiver.ino
2. Verify (✓)
3. Upload (→)
4. Wait for "Done uploading"
```

### STEP 6: Test Communication (5 minutes)

#### On Laptop 1 (Transmitter):
```
1. Open Serial Monitor (Ctrl+Shift+M)
2. Set baud: 115200
3. Press RESET on ESP32
4. Look for:
   ✓ ESP-NOW initialized
   ✓ Receiver registered
   🚀 Starting DITMC transmission...
```

#### On Laptop 2 (Receiver):
```
1. Open Serial Monitor
2. Set baud: 115200
3. Press RESET on ESP32
4. Look for:
   ✓ ESP-NOW initialized
   🎧 Waiting for packets...
   📦 Packet received...
```

### STEP 7: Verify Results (10 minutes)

**Let the system run for 60 seconds**, then check statistics:

#### Expected Transmitter Output:
```
╔════════════════════════════════════════╗
║      DITMC STATISTICS (Transmitter)    ║
╠════════════════════════════════════════╣
║ Voice bytes sent:       ~5000-8000     ║
║ Interleaved data bytes: ~3000-5000     ║
║ Interleaving %:         40-50%         ║
║ Overhead %:             <1%            ║
╚════════════════════════════════════════╝
```

#### Expected Receiver Output:
```
╔════════════════════════════════════════╗
║      DITMC STATISTICS (Receiver)       ║
╠════════════════════════════════════════╣
║ Packets received:       100-200        ║
║ Data percentage:        30-50%         ║
║ Reconstruction rate:    100%           ║
╚════════════════════════════════════════╝
```

---

## ✅ Success Checklist

You've successfully implemented DITMC when:
- [ ] Both ESP32s power on without errors
- [ ] Transmitter shows "Receiver registered"
- [ ] Receiver shows "Packet received"
- [ ] Interleaving percentage is 30-50%
- [ ] Both devices run stable for 60+ seconds
- [ ] Statistics display every 5 seconds

---

## 🔧 Common Issues & Quick Fixes

### Issue: "Board not found"
**Fix:** Install CH340/CP2102 USB drivers

### Issue: "Upload failed"
**Fix:** Hold BOOT button while uploading

### Issue: "ESP-NOW init failed"
**Fix:** Check "USB CDC On Boot" is Enabled

### Issue: "Send failed"
**Fix:** Verify Receiver MAC address in transmitter/config.h

### Issue: "No packets received"
**Fix:** 
1. Check both ESP32s are on
2. Press RESET on both
3. Move closer together

---

## 📊 What to Observe

### During Operation:

1. **Transmitter Messages:**
   - `→ Repetition detected` - Algorithm is working!
   - `→ Silence detected` - Algorithm is working!
   - `✓ Packet sent` - Communication is working!

2. **Receiver Messages:**
   - `📦 Packet received [INTERLEAVED]` - Data interleaving happening!
   - `→ BATTERY OFF detected` - Codes are recognized!
   - `Voice: XX bytes | Data: XX bytes` - Separation working!

3. **Statistics (Every 5 Seconds):**
   - Watch interleaving percentage
   - Should stabilize around 40-50%
   - Overhead should be < 1%

---

## 🎓 Understanding Your Results

### Good Results:
- **Interleaving 40-50%**: Excellent! Matches paper.
- **Interleaving 30-40%**: Good! Voice has less redundancy.
- **Interleaving 50-60%**: Great! High redundancy detected.

### Check If:
- **Interleaving < 20%**: Repetition threshold may be too high
- **Interleaving > 70%**: May have configuration issue
- **Packets lost > 5%**: Move ESP32s closer or check interference

---

## 🚀 Next Experiments (After Basic Setup Works)

### Experiment 1: Adjust Repetition Threshold
```cpp
// In transmitter/config.h
#define REPETITION_THRESHOLD 3  // Try 3, 4, 5, 6
// Lower = more interleaving, Higher = less interleaving
```

### Experiment 2: Change Sample Buffer Size
```cpp
// In transmitter/config.h
#define SAMPLE_BUFFER_SIZE 4   // Try 4, 8, 16
// Affects detection window
```

### Experiment 3: Modify Send Interval
```cpp
// In transmitter/config.h
#define SEND_INTERVAL_MS 10    // Try 10, 50, 100
// Faster = more packets, Slower = fewer packets
```

### Experiment 4: Test Range
- Start with ESP32s 1 meter apart
- Gradually increase distance
- Note when communication fails
- Expected range: 20-50 meters indoors

---

## 📝 Documentation Tips

### Record Your Results:

1. **Take Screenshots:**
   - Transmitter statistics
   - Receiver statistics
   - Any errors or unusual behavior

2. **Note Your Settings:**
   - Repetition threshold used
   - Buffer size
   - Send interval
   - Distance between ESP32s

3. **Compare with Paper:**
   - Paper reports 47.32% interleaving
   - Paper reports 0.034% overhead
   - Your results should be within ±10%

---

## 🎯 Project Milestones

- [ ] **Milestone 1**: Both ESP32s programmed and running
- [ ] **Milestone 2**: Communication established
- [ ] **Milestone 3**: Data interleaving observed
- [ ] **Milestone 4**: Statistics matching paper expectations
- [ ] **Milestone 5**: System runs stable for 10+ minutes
- [ ] **Milestone 6**: Successfully experimented with parameters

---

## 📚 Additional Resources

### Included Documentation:
- `README.md` - Project overview
- `QUICK_START.md` - Setup checklist
- `SETUP_GUIDE.md` - Detailed instructions
- `DITMC_EXPLAINED.md` - Algorithm details

### ESP32 Resources:
- ESP32 Arduino Core: https://github.com/espressif/arduino-esp32
- ESP-NOW Guide: https://randomnerdtutorials.com/esp-now-esp32-arduino-ide/
- ESP32-S3 Datasheet: https://www.espressif.com/en/products/socs/esp32-s3

---

## 🎉 You're All Set!

You now have everything needed to implement the DITMC paper on your ESP32-S3 boards.

**Start with QUICK_START.md and follow the steps in order.**

The entire process should take about 40-60 minutes for first-time setup.

Good luck with your implementation! 🚀

---

## 💡 Need Help?

If you get stuck:
1. Check `SETUP_GUIDE.md` Section 7 (Troubleshooting)
2. Verify all steps in `QUICK_START.md`
3. Review Serial Monitor output for error messages
4. Try the basic `get_mac_address.ino` sketch to verify hardware

Remember: The most common issues are:
- Wrong MAC address in config.h
- Incorrect board settings
- USB driver issues
- Forgetting to press RESET after upload
