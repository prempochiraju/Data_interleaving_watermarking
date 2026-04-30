#ifndef DITMC_WATERMARK_H
#define DITMC_WATERMARK_H

#include <Arduino.h>
#include "config.h"

static const size_t WATERMARK_SIZE_BYTES = sizeof(uint32_t);

inline uint32_t crc32Update(uint32_t crc, const uint8_t *data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; bit++) {
            uint32_t mask = -(crc & 1UL);
            crc = (crc >> 1) ^ (0xEDB88320UL & mask);
        }
    }

    return ~crc;
}

inline uint32_t calculateWatermark(const uint8_t *data, size_t length, uint16_t timestamp) {
    uint32_t crc = 0xFFFFFFFFUL;

    // The watermark authenticates the interleaved data block, packet timestamp,
    // and local secret. It is payload-level DITMC data, not audio watermarking.
    crc = crc32Update(crc, data, length);
    crc = crc32Update(crc, reinterpret_cast<const uint8_t *>(&timestamp), sizeof(timestamp));
    crc = crc32Update(crc, SECRET_KEY, SECRET_KEY_LENGTH);

    return ~crc;
}

inline void writeWatermarkBytes(uint8_t *destination, uint32_t watermark) {
    destination[0] = static_cast<uint8_t>(watermark & 0xFF);
    destination[1] = static_cast<uint8_t>((watermark >> 8) & 0xFF);
    destination[2] = static_cast<uint8_t>((watermark >> 16) & 0xFF);
    destination[3] = static_cast<uint8_t>((watermark >> 24) & 0xFF);
}

inline uint32_t readWatermarkBytes(const uint8_t *source) {
    return static_cast<uint32_t>(source[0]) |
           (static_cast<uint32_t>(source[1]) << 8) |
           (static_cast<uint32_t>(source[2]) << 16) |
           (static_cast<uint32_t>(source[3]) << 24);
}

#endif // DITMC_WATERMARK_H
