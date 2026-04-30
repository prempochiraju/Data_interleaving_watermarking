# Project Structure Notes

## Runtime Modes

The single PlatformIO source entry point is [ditmc_node/main.cpp](../ditmc_node/main.cpp). It selects one of three modes using `DITMC_MODE` from [platformio.ini](../platformio.ini):

- `get_mac`: prints the ESP32-S3 MAC address.
- `esp32s3_transmitter`: runs DITMC transmission.
- `esp32s3_receiver`: runs DITMC reception and reconstruction.

## Important Headers

- [include/config.h](../include/config.h): receiver MAC address, ESP-NOW settings, DITMC thresholds, battery markers, and the shared `SECRET_KEY`.
- [include/packet_format.h](../include/packet_format.h): shared variable-length packet layout. Metadata is packed and appended after payload bytes.
- [include/watermark.h](../include/watermark.h): reusable CRC32 helpers used by both boards.
- [include/transmitter.h](../include/transmitter.h): detects silence/repetition, inserts DITMC data, appends watermark bytes, and sends ESP-NOW packets.
- [include/receiver.h](../include/receiver.h): parses DITMC markers, extracts data, verifies watermark bytes, and reports detection statistics.

## Packet Layout

ESP-NOW sends a compact variable-length frame:

```text
[payload bytes][PacketMetadata]
```

`PacketMetadata` contains:

```text
uint16_t voiceDataSize
uint8_t  containsInterleavedData
uint16_t timestamp
```

The project uses `uint8_t` instead of `bool` in packet metadata to avoid size differences between compilers or platforms. The receiver uses `sizeof(PacketMetadata)` rather than a hard-coded metadata size.

## Watermark Flow

Inside an interleaved DITMC region, the transmitter writes:

```text
BATTERY_OFF_CODE
interleaved data bytes
4 watermark bytes
BATTERY_ON_CODE
```

The CRC32 watermark is calculated as:

```text
crc32(interleaved data bytes + timestamp + SECRET_KEY)
```

The receiver stores data-mode bytes until `BATTERY_ON_CODE`, treats the final four bytes as the received watermark, recalculates the CRC32 using the original data-mode timestamp, and updates valid/invalid counters.

## Statistics To Watch

Transmitter:

- `Watermark blocks sent`
- `Watermark blocks skipped`
- `Watermark bytes sent`
- `Watermark overhead`

Receiver:

- `Watermark blocks received`
- `Valid watermarks`
- `Invalid watermarks`
- `Watermark accuracy`
- `Watermark overhead`
