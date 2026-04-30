#ifndef RECEIVER_H
#define RECEIVER_H

#include <WiFi.h>
#include <cstring>
#include <esp_now.h>
#include <esp_wifi.h>
#include "config.h"
#include "packet_format.h"
#include "watermark.h"

struct RxStatistics {
    unsigned long totalPacketsReceived;
    unsigned long totalVoiceBytes;
    unsigned long totalDataBytes;
    unsigned long batteryOffReceived;
    unsigned long batteryOnReceived;
    unsigned long reconstructedVoiceBytes;
    unsigned long extractedDataBytes;
    unsigned long lastPacketTimestamp;
    unsigned long watermarkBlocksReceived;
    unsigned long watermarkValid;
    unsigned long watermarkInvalid;
    unsigned long watermarkBlocksSkipped;
    unsigned long totalWatermarkBytesReceived;
};

RxStatistics stats = {0};

enum ReceiverState {
    STATE_NORMAL_VOICE,
    STATE_INTERLEAVED_DATA
};

ReceiverState currentState = STATE_NORMAL_VOICE;

uint8_t reconstructedVoice[RECEPTION_BUFFER_SIZE];
uint16_t reconstructedVoiceSize = 0;

uint8_t extractedData[RECEPTION_BUFFER_SIZE];
uint16_t extractedDataSize = 0;

uint8_t activeDataBlock[RECEPTION_BUFFER_SIZE];
uint16_t activeDataBlockSize = 0;
uint16_t activeDataBlockTimestamp = 0;

static const uint16_t WATERMARKED_BLOCK_PAYLOAD_SIZE =
    INTERLEAVED_DATA_BLOCK_SIZE + WATERMARK_SIZE_BYTES;

unsigned long lastStatsTime = 0;
unsigned long lastPacketTime = 0;

void appendExtractedData(const uint8_t *data, uint16_t dataLength) {
    for (uint16_t i = 0; i < dataLength; i++) {
        if (extractedDataSize < RECEPTION_BUFFER_SIZE) {
            extractedData[extractedDataSize++] = data[i];
        }
        stats.totalDataBytes++;
        stats.extractedDataBytes++;
    }
}

void finalizeInterleavedDataBlock() {
    if (activeDataBlockSize < WATERMARK_SIZE_BYTES) {
        // Sender can skip watermarking if a block cannot fit data + CRC32.
        // In that case everything collected is treated as ordinary interleaved data.
        appendExtractedData(activeDataBlock, activeDataBlockSize);
        stats.watermarkBlocksSkipped++;
        activeDataBlockSize = 0;
        return;
    }

    uint16_t dataLength = activeDataBlockSize - WATERMARK_SIZE_BYTES;
    uint32_t receivedWatermark = readWatermarkBytes(activeDataBlock + dataLength);
    uint32_t calculatedWatermark = calculateWatermark(activeDataBlock, dataLength, activeDataBlockTimestamp);

    appendExtractedData(activeDataBlock, dataLength);

    stats.watermarkBlocksReceived++;
    stats.totalWatermarkBytesReceived += WATERMARK_SIZE_BYTES;

    if (receivedWatermark == calculatedWatermark) {
        stats.watermarkValid++;
        if (ENABLE_DEBUG) Serial.println("  Watermark valid");
    } else {
        stats.watermarkInvalid++;
        if (ENABLE_DEBUG) {
            Serial.printf("  Watermark invalid: received=0x%08lX calculated=0x%08lX\n",
                          (unsigned long)receivedWatermark,
                          (unsigned long)calculatedWatermark);
        }
    }

    activeDataBlockSize = 0;
}

void processReceivedPacket(const DataPacket *packet) {
    stats.totalPacketsReceived++;
    stats.lastPacketTimestamp = packet->timestamp;

    if (ENABLE_DEBUG) {
        Serial.printf("\nPacket received [#%lu]: %u payload bytes",
                      stats.totalPacketsReceived, packet->voiceDataSize);
        if (packet->containsInterleavedData) {
            Serial.print(" [INTERLEAVED]");
        }
        Serial.println();
    }

    for (uint16_t i = 0; i < packet->voiceDataSize; i++) {
        uint8_t byte = packet->voiceData[i];

        if (currentState == STATE_NORMAL_VOICE) {
            if (byte == BATTERY_OFF_CODE) {
                currentState = STATE_INTERLEAVED_DATA;
                activeDataBlockSize = 0;
                activeDataBlockTimestamp = packet->timestamp;
                stats.batteryOffReceived++;

                if (ENABLE_DEBUG) {
                    Serial.println("  BATTERY OFF detected: entering data mode");
                }
                continue;
            }

            if (reconstructedVoiceSize < RECEPTION_BUFFER_SIZE) {
                reconstructedVoice[reconstructedVoiceSize++] = byte;
                stats.totalVoiceBytes++;
                stats.reconstructedVoiceBytes++;
            }
        } else {
            // The CRC32 watermark can contain marker-like byte values. Because the
            // demo DITMC payload is fixed-size, only treat BATTERY_ON_CODE as the
            // closing marker after the expected data + watermark bytes arrived.
            if (byte == BATTERY_ON_CODE &&
                activeDataBlockSize >= WATERMARKED_BLOCK_PAYLOAD_SIZE) {
                finalizeInterleavedDataBlock();
                currentState = STATE_NORMAL_VOICE;
                stats.batteryOnReceived++;

                if (ENABLE_DEBUG) {
                    Serial.println("  BATTERY ON detected: returning to voice mode");
                }
                continue;
            }

            if (activeDataBlockSize < RECEPTION_BUFFER_SIZE) {
                activeDataBlock[activeDataBlockSize++] = byte;
            }
        }
    }

    if (ENABLE_DEBUG) {
        Serial.printf("  Voice: %lu bytes | Data: %lu bytes\n",
                      stats.reconstructedVoiceBytes, stats.extractedDataBytes);
    }

    digitalWrite(LED_PIN, HIGH);
    delay(10);
    digitalWrite(LED_PIN, LOW);
}

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
    lastPacketTime = millis();

    if (data_len < (int)PACKET_METADATA_SIZE || data_len > MAX_PACKET_SIZE) {
        Serial.printf("Invalid packet size received: %d\n", data_len);
        return;
    }

    PacketMetadata metadata = {};
    size_t metadataOffset = data_len - PACKET_METADATA_SIZE;
    memcpy(&metadata, data + metadataOffset, PACKET_METADATA_SIZE);

    if (metadata.voiceDataSize != metadataOffset ||
        metadata.voiceDataSize > MAX_VOICE_PAYLOAD_SIZE) {
        Serial.println("Invalid packet metadata received");
        return;
    }

    DataPacket packet = {};
    memcpy(packet.voiceData, data, metadata.voiceDataSize);
    packet.voiceDataSize = metadata.voiceDataSize;
    packet.containsInterleavedData = metadata.containsInterleavedData;
    packet.timestamp = metadata.timestamp;

    processReceivedPacket(&packet);
}

void displayStatistics() {
    unsigned long totalBytes = stats.totalVoiceBytes + stats.totalDataBytes;
    unsigned long totalReceivedBytes = totalBytes + stats.batteryOffReceived +
                                       stats.batteryOnReceived + stats.totalWatermarkBytesReceived;

    float dataPercentage = 0.0;
    float successRate = 100.0;
    float watermarkAccuracy = 0.0;
    float watermarkOverheadPercentage = 0.0;

    if (totalBytes > 0) {
        dataPercentage = (float)stats.totalDataBytes * 100.0 / totalBytes;
    }

    if (stats.watermarkBlocksReceived > 0) {
        watermarkAccuracy = (float)stats.watermarkValid * 100.0 / stats.watermarkBlocksReceived;
    }

    if (totalReceivedBytes > 0) {
        watermarkOverheadPercentage = (float)stats.totalWatermarkBytesReceived * 100.0 /
                                      totalReceivedBytes;
    }

    Serial.println("\n========== DITMC STATISTICS (Receiver) ==========");
    Serial.printf("Packets received:            %lu\n", stats.totalPacketsReceived);
    Serial.printf("Voice bytes received:        %lu\n", stats.totalVoiceBytes);
    Serial.printf("Data bytes received:         %lu\n", stats.totalDataBytes);
    Serial.printf("Data percentage:             %.2f%%\n", dataPercentage);
    Serial.printf("BATTERY OFF received:        %lu\n", stats.batteryOffReceived);
    Serial.printf("BATTERY ON received:         %lu\n", stats.batteryOnReceived);
    Serial.printf("Reconstruction rate:         %.2f%%\n", successRate);
    Serial.printf("Watermark blocks received:   %lu\n", stats.watermarkBlocksReceived);
    Serial.printf("Valid watermarks:            %lu\n", stats.watermarkValid);
    Serial.printf("Invalid watermarks:          %lu\n", stats.watermarkInvalid);
    Serial.printf("Watermark blocks skipped:    %lu\n", stats.watermarkBlocksSkipped);
    Serial.printf("Watermark bytes received:    %lu\n", stats.totalWatermarkBytesReceived);
    Serial.printf("Watermark accuracy:          %.2f%%\n", watermarkAccuracy);
    Serial.printf("Watermark overhead:          %.2f%%\n", watermarkOverheadPercentage);

    Serial.print("Data sample: ");
    int sampleSize = min(20, (int)extractedDataSize);
    for (int i = 0; i < sampleSize; i++) {
        if (extractedData[i] >= 32 && extractedData[i] <= 126) {
            Serial.printf("%c", extractedData[i]);
        } else {
            Serial.print(".");
        }
    }
    Serial.println();
    Serial.println("=================================================\n");
}

void setup_receiver() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\nDITMC Receiver - ESP32-S3\n");

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    Serial.print("Receiver MAC Address: ");
    Serial.println(WiFi.macAddress());
    Serial.println("Copy this address to include/config.h.\n");

    esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);

    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW initialization failed!");
        return;
    }
    Serial.println("ESP-NOW initialized");

    esp_now_register_recv_cb(OnDataRecv);

    Serial.println("Receiver ready");
    Serial.println("Waiting for packets...\n");

    lastStatsTime = millis();
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
}

void loop_receiver() {
    unsigned long currentTime = millis();

    if (currentTime - lastStatsTime >= STATS_INTERVAL) {
        if (stats.totalPacketsReceived > 0) {
            displayStatistics();
        } else {
            Serial.println("No packets received yet...");
        }
        lastStatsTime = currentTime;
    }

    if (stats.totalPacketsReceived > 0 &&
        (currentTime - lastPacketTime > 10000)) {
        Serial.println("\nNo packets received for 10 seconds");
        Serial.println("Check transmitter status\n");
        lastPacketTime = currentTime;
    }

    delay(10);
}

#endif // RECEIVER_H
