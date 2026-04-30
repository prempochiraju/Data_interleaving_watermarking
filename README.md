# DITMC ESP32-S3 Watermarking Project

This PlatformIO project implements Data Interleaving Technique in Mobile Communication (DITMC) on two ESP32-S3 boards using ESP-NOW.

One board runs as the transmitter and one board runs as the receiver. Voice bytes are sent normally until redundant/silent samples are detected. During that redundant region, the transmitter inserts DITMC data between `BATTERY_OFF_CODE` and `BATTERY_ON_CODE`.

The current implementation adds lightweight payload-level watermarking inside that DITMC interleaved data region:

```text
BATTERY_OFF_CODE
[interleaved data bytes]
[4 CRC32 watermark bytes]
BATTERY_ON_CODE
```

The watermark is calculated from the interleaved data bytes, packet timestamp, and `SECRET_KEY` in [include/config.h](include/config.h).

## Project Layout

```text
.
|-- platformio.ini              PlatformIO environments
|-- README.md                   Project overview
|-- ditmc_node/
|   `-- main.cpp                Selects transmitter, receiver, or MAC utility
|-- include/
|   |-- config.h                MAC address, DITMC constants, secret key
|   |-- packet_format.h         Shared payload metadata format
|   |-- watermark.h             CRC32 watermark helpers
|   |-- transmitter.h           Transmitter DITMC flow
|   |-- receiver.h              Receiver DITMC flow and watermark checks
|   `-- get_mac.h               MAC address utility
`-- doc/
    |-- PROJECT_STRUCTURE.md    Notes for understanding the code
    |-- guides/                 Setup and quick-start notes
    `-- *.html, *.md            Results and analysis artifacts
```

## Build

Use the receiver MAC printed by the receiver board and copy it into [include/config.h](include/config.h).

```powershell
C:\Users\pyppr\.platformio\penv\Scripts\platformio.exe run -e get_mac
C:\Users\pyppr\.platformio\penv\Scripts\platformio.exe run -e esp32s3_transmitter
C:\Users\pyppr\.platformio\penv\Scripts\platformio.exe run -e esp32s3_receiver
```

## Upload And Monitor

```powershell
C:\Users\pyppr\.platformio\penv\Scripts\platformio.exe run -e esp32s3_transmitter -t upload -t monitor
C:\Users\pyppr\.platformio\penv\Scripts\platformio.exe run -e esp32s3_receiver -t upload -t monitor
```

Expected receiver statistics include:

```text
Watermark blocks received:   X
Valid watermarks:            X
Invalid watermarks:          0
Watermark accuracy:          100.00%
Watermark overhead:          Y%
```

## More Notes

Detailed setup notes that used to live at the repository root are now under [doc/guides](doc/guides). Start with [doc/PROJECT_STRUCTURE.md](doc/PROJECT_STRUCTURE.md) for a code map.

For experiment capture, log both boards and plot the results using [doc/LOGGING_AND_PLOTTING.md](doc/LOGGING_AND_PLOTTING.md).
