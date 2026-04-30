#ifndef DITMC_PACKET_FORMAT_H
#define DITMC_PACKET_FORMAT_H

#include <Arduino.h>
#include "config.h"

// Packet metadata is appended after the variable-length payload.
// Keep this packed and use uint8_t flags so both ESP32-S3 nodes agree on size.
struct __attribute__((packed)) PacketMetadata {
    uint16_t voiceDataSize;
    uint8_t containsInterleavedData;
    uint16_t timestamp;
};

static const size_t PACKET_METADATA_SIZE = sizeof(PacketMetadata);
static const size_t MAX_VOICE_PAYLOAD_SIZE = MAX_PACKET_SIZE - PACKET_METADATA_SIZE;

struct DataPacket {
    uint8_t voiceData[MAX_VOICE_PAYLOAD_SIZE];
    uint16_t voiceDataSize;
    uint8_t containsInterleavedData;
    uint16_t timestamp;
};

#endif // DITMC_PACKET_FORMAT_H
