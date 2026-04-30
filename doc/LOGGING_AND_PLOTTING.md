# Logging And Plotting Watermark Results

You can log both the transmitter side and receiver side, then use those logs to plot graphs for the DITMC watermarking experiment.

## Which Laptop Saves Which Results

Use the transmitter laptop to log transmission statistics:

- `Watermark blocks sent`
- `Watermark bytes sent`
- `Watermark blocks skipped`
- `Packets sent`
- `Send failures`

Use the receiver laptop to log verification statistics:

- `Watermark blocks received`
- `Valid watermarks`
- `Invalid watermarks`
- `Watermark accuracy`
- `Watermark overhead`

The receiver log is the main proof that watermarking worked, because the receiver recalculates the CRC32 watermark and reports whether it is valid.

## Find COM Ports

Run this on each laptop:

```powershell
C:\Users\pyppr\.platformio\penv\Scripts\platformio.exe device list
```

Use the transmitter board COM port on the transmitter laptop, and the receiver board COM port on the receiver laptop.

## Log Transmitter Side

Run this on the transmitter laptop:

```powershell
C:\Users\pyppr\.platformio\penv\Scripts\platformio.exe device monitor -b 115200 -p COM_TRANSMITTER --filter log2file
```

Replace `COM_TRANSMITTER` with the real port, for example `COM8`.

## Log Receiver Side

Run this on the receiver laptop:

```powershell
C:\Users\pyppr\.platformio\penv\Scripts\platformio.exe device monitor -b 115200 -p COM_RECEIVER --filter log2file
```

Replace `COM_RECEIVER` with the real receiver port.

PlatformIO saves serial output into a log file named similar to:

```text
platformio-device-monitor-*.log
```

After the experiment, copy both log files into one folder, for example:

```text
C:\Users\pyppr\projects\DATA_inter_esp_watermarking\logs
```

## Suggested Graphs

Useful plots for the watermarking experiment:

- Watermark blocks sent vs watermark blocks received
- Valid watermarks vs invalid watermarks
- Watermark accuracy over time
- Watermark overhead over time
- Packets sent vs packets received
- Send failures over time

Expected successful receiver output:

```text
Watermark blocks received:   X
Valid watermarks:            X
Invalid watermarks:          0
Watermark accuracy:          100.00%
Watermark overhead:          Y%
```

If `Invalid watermarks` is greater than `0`, check receiver MAC address, ESP-NOW channel, board distance, and packet corruption/loss conditions.
