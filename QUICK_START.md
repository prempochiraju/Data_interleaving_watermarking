# QUICK START CHECKLIST

## Before You Begin - Materials Check
- [ ] 2x ESP32-S3 WROOM-1 boards
- [ ] 2x USB-C cables (data cables, not charge-only!)
- [ ] Laptop 1 with Arduino IDE installed
- [ ] Laptop 2 with Arduino IDE installed (or use one laptop and switch)
- [ ] Pen and paper to write down MAC addresses

---

## Step-by-Step Setup (30-45 minutes)

### PHASE 1: Software Setup (Both Laptops)

#### On Laptop 1:
- [ ] Install Arduino IDE 2.0+
- [ ] Add ESP32 board support URL to Preferences
- [ ] Install "esp32 by Espressif Systems" from Boards Manager
- [ ] Select Board: Tools → ESP32S3 Dev Module
- [ ] Set "USB CDC On Boot" to "Enabled"

#### On Laptop 2:
- [ ] Repeat all steps from Laptop 1

**Time estimate: 15 minutes per laptop**

---

### PHASE 2: Get MAC Addresses

#### On Laptop 1 (Transmitter ESP32):
- [ ] Connect first ESP32-S3 via USB
- [ ] Select correct COM port in Arduino IDE
- [ ] Open `get_mac_address/get_mac_address.ino`
- [ ] Upload the sketch
- [ ] Open Serial Monitor (115200 baud)
- [ ] **WRITE DOWN the MAC address shown**
- [ ] Label this ESP32 as "TRANSMITTER" (use tape/sticker)

**MAC Address: __ : __ : __ : __ : __ : __**

#### On Laptop 2 (Receiver ESP32):
- [ ] Connect second ESP32-S3 via USB
- [ ] Select correct COM port in Arduino IDE
- [ ] Open `get_mac_address/get_mac_address.ino`
- [ ] Upload the sketch
- [ ] Open Serial Monitor (115200 baud)
- [ ] **WRITE DOWN the MAC address shown**
- [ ] Label this ESP32 as "RECEIVER" (use tape/sticker)

**MAC Address: __ : __ : __ : __ : __ : __**

**Time estimate: 10 minutes**

---

### PHASE 3: Configure Transmitter

#### On Laptop 1:
- [ ] Open `transmitter/config.h` in Arduino IDE
- [ ] Find the line: `uint8_t receiverMAC[] = {...}`
- [ ] Replace with RECEIVER's MAC address (from Phase 2)
  
  Example:
  ```cpp
  // If Receiver MAC is: AA:BB:CC:DD:EE:FF
  uint8_t receiverMAC[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
  ```

- [ ] Save the file
- [ ] Open `transmitter/transmitter.ino`
- [ ] Click Verify (✓) - should compile without errors
- [ ] Click Upload (→)
- [ ] Wait for "Done uploading"

**Time estimate: 5 minutes**

---

### PHASE 4: Upload Receiver

#### On Laptop 2:
- [ ] Open `receiver/receiver.ino` in Arduino IDE
- [ ] Click Verify (✓) - should compile without errors
- [ ] Click Upload (→)
- [ ] Wait for "Done uploading"

**Time estimate: 3 minutes**

---

### PHASE 5: Test Communication

#### On Laptop 1 (Transmitter):
- [ ] Open Serial Monitor (Ctrl+Shift+M)
- [ ] Set baud rate to 115200
- [ ] Press RESET button on ESP32
- [ ] You should see:
  ```
  ✓ ESP-NOW initialized
  ✓ Receiver registered: [MAC address]
  🚀 Starting DITMC transmission...
  ```

#### On Laptop 2 (Receiver):
- [ ] Open Serial Monitor
- [ ] Set baud rate to 115200
- [ ] Press RESET button on ESP32
- [ ] You should see:
  ```
  ✓ ESP-NOW initialized
  🎧 Waiting for packets...
  📦 Packet received...
  ```

**Time estimate: 5 minutes**

---

## Verification - Is It Working?

### Transmitter Serial Monitor Should Show:
```
✓ Packet sent: XX voice bytes
→ Repetition detected: Interleaving data
→ Silence detected: Interleaving data

=== DITMC STATISTICS ===
Voice bytes sent: XXXX
Interleaved data bytes: XXX
Interleaving %: ~47%
```

### Receiver Serial Monitor Should Show:
```
📦 Packet received: XX bytes [INTERLEAVED]
→ BATTERY OFF detected: Entering data mode
Voice: XX bytes | Data: XX bytes

=== DITMC STATISTICS ===
Data percentage: ~30-50%
```

**If you see these messages: SUCCESS! ✅**

---

## Troubleshooting Checklist

### No output in Serial Monitor?
- [ ] Check baud rate is 115200
- [ ] Press RESET button on ESP32
- [ ] Try different USB port
- [ ] Check "USB CDC On Boot" is Enabled

### "Upload failed" error?
- [ ] Hold BOOT button while uploading
- [ ] Check correct port is selected
- [ ] Close Serial Monitor before uploading
- [ ] Try lower upload speed (115200)

### Transmitter shows "Send failed"?
- [ ] Verify Receiver MAC address in config.h
- [ ] Check both ESP32s are powered on
- [ ] Move ESP32s closer together
- [ ] Press RESET on both devices

### Receiver shows "No packets received"?
- [ ] Check Transmitter is running (check its Serial Monitor)
- [ ] Verify MAC address is correct
- [ ] Press RESET on Transmitter
- [ ] Check both are on same WiFi channel (channel 1)

---

## Expected Timeline

| Phase | Task | Time |
|-------|------|------|
| 1 | Software setup (both laptops) | 15 min |
| 2 | Get MAC addresses | 10 min |
| 3 | Configure & upload transmitter | 5 min |
| 4 | Upload receiver | 3 min |
| 5 | Test communication | 5 min |
| **TOTAL** | | **~40 min** |

---

## What's Next?

After successful communication:

1. **Let it run for 60 seconds** - The simulation runs automatically
2. **Observe statistics** - Updated every 5 seconds
3. **Compare with paper results** - Target: ~47% interleaving
4. **Experiment** - Try modifying parameters in config.h
5. **Document** - Take screenshots of statistics

---

## Success Criteria

You've successfully implemented DITMC when:
- ✅ Both ESP32s communicate via ESP-NOW
- ✅ Transmitter detects repetitions and silence
- ✅ Receiver separates voice and data
- ✅ Interleaving percentage is 30-50%
- ✅ System runs without errors for 60+ seconds

---

## Getting Help

If stuck:
1. Check Serial Monitor for error messages
2. Review SETUP_GUIDE.md Section 7 (Troubleshooting)
3. Verify all connections and settings
4. Try uploading get_mac_address.ino again to verify basic functionality

Good luck! 🚀
