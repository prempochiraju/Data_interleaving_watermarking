#ifndef TRANSMITTER_H
#define TRANSMITTER_H

#include <WiFi.h>
#include <cstring>
#include <esp_now.h>
#include <esp_wifi.h>
#include "config.h"
#include "packet_format.h"
#include "watermark.h"

struct Statistics {
    unsigned long totalVoiceBytes;
    unsigned long redundantBytes;
    unsigned long interleavedDataBytes;
    unsigned long batteryOffSent;
    unsigned long batteryOnSent;
    unsigned long packetsSent;
    unsigned long sendFailures;
    unsigned long totalWatermarkBytesSent;
    unsigned long watermarkBlocksSent;
    unsigned long watermarkBlocksSkipped;
};

Statistics stats = {0};

bool batteryOffTransmitted = false;
unsigned long lastSendTime = 0;
unsigned long lastStatsTime = 0;
unsigned long simulationStartTime = 0;

uint8_t sampleBuffer[SAMPLE_BUFFER_SIZE];
int bufferIndex = 0;

DataPacket currentPacket = {};

void sendPacket();

void ensureCurrentPacketTimestamp() {
    if (currentPacket.timestamp == 0) {
        currentPacket.timestamp = (uint16_t)(millis() & 0xFFFF);
    }
}

bool hasPacketSpace(size_t bytesNeeded) {
    return (currentPacket.voiceDataSize + bytesNeeded) <= MAX_VOICE_PAYLOAD_SIZE;
}

void appendPacketByte(uint8_t value) {
    if (currentPacket.voiceDataSize < MAX_VOICE_PAYLOAD_SIZE) {
        currentPacket.voiceData[currentPacket.voiceDataSize++] = value;
    }
}

void appendBatteryOnCode() {
    if (!hasPacketSpace(1)) {
        sendPacket();
    }

    if (hasPacketSpace(1)) {
        appendPacketByte(BATTERY_ON_CODE);
        currentPacket.containsInterleavedData = 1;
        stats.batteryOnSent++;
    }
}

void appendWatermarkedInterleavedData(const uint8_t *interleavedData, size_t dataLength) {
    const size_t bytesNeeded = 1 + dataLength + WATERMARK_SIZE_BYTES;

    if (dataLength == 0 || bytesNeeded > MAX_VOICE_PAYLOAD_SIZE) {
        stats.watermarkBlocksSkipped++;
        return;
    }

    if (!hasPacketSpace(bytesNeeded)) {
        sendPacket();
    }

    if (!hasPacketSpace(bytesNeeded)) {
        stats.watermarkBlocksSkipped++;
        return;
    }

    ensureCurrentPacketTimestamp();

    appendPacketByte(BATTERY_OFF_CODE);
    stats.batteryOffSent++;
    batteryOffTransmitted = true;

    for (size_t i = 0; i < dataLength; i++) {
        appendPacketByte(interleavedData[i]);
        stats.interleavedDataBytes++;
    }

    // Watermark bytes are inserted inside the DITMC data region:
    // BATTERY_OFF_CODE, data bytes, 4 CRC32 bytes, BATTERY_ON_CODE.
    uint32_t watermark = calculateWatermark(interleavedData, dataLength, currentPacket.timestamp);
    writeWatermarkBytes(currentPacket.voiceData + currentPacket.voiceDataSize, watermark);
    currentPacket.voiceDataSize += WATERMARK_SIZE_BYTES;

    currentPacket.containsInterleavedData = 1;
    stats.totalWatermarkBytesSent += WATERMARK_SIZE_BYTES;
    stats.watermarkBlocksSent++;
}

uint8_t generateVoiceSample() {
    static uint8_t lastValue = 128;
    static int repetitionCount = 0;
    static int silenceCount = 0;

    if (random(100) < 20) {
        silenceCount++;
        return 0x00;
    }

    if (random(100) < 30 && repetitionCount < 8) {
        repetitionCount++;
        return lastValue;
    }

    repetitionCount = 0;
    silenceCount = 0;
    lastValue = (uint8_t)random(1, 255);
    return lastValue;
}

bool checkForRepetition(uint8_t *buffer, int size, int threshold) {
    if (size < threshold) return false;

    int count = 1;
    for (int i = 1; i < size; i++) {
        if (buffer[i] == buffer[i - 1]) {
            count++;
            if (count >= threshold) return true;
        } else {
            count = 1;
        }
    }
    return false;
}

bool checkForSilence(uint8_t *buffer, int size, int threshold) {
    if (size < threshold) return false;

    int silenceCount = 0;
    for (int i = 0; i < size; i++) {
        if (buffer[i] == 0x00) silenceCount++;
    }
    return silenceCount >= threshold;
}

void processSamples() {
    if (bufferIndex < REPETITION_THRESHOLD) return;

    bool hasRepetition = checkForRepetition(sampleBuffer, bufferIndex, REPETITION_THRESHOLD);
    bool hasSilence = checkForSilence(sampleBuffer, bufferIndex, REPETITION_THRESHOLD);

    if (hasSilence) {
        if (!batteryOffTransmitted) {
            uint8_t interleavedData[] = {'D', 'A', 'T', 'A'};
            appendWatermarkedInterleavedData(interleavedData, sizeof(interleavedData));

            if (ENABLE_DEBUG) Serial.println("-> Silence detected: interleaving data");
        }
        stats.redundantBytes += bufferIndex;
    } else if (hasRepetition) {
        if (!batteryOffTransmitted) {
            if (!hasPacketSpace(1)) {
                sendPacket();
            }

            if (hasPacketSpace(1)) {
                appendPacketByte(sampleBuffer[0]);
                stats.totalVoiceBytes++;
            }

            uint8_t interleavedData[] = {'I', 'N', 'T', 'R'};
            appendWatermarkedInterleavedData(interleavedData, sizeof(interleavedData));

            if (ENABLE_DEBUG) Serial.println("-> Repetition detected: interleaving data");
        }
        stats.redundantBytes += (bufferIndex - 1);
    } else {
        if (batteryOffTransmitted) {
            appendBatteryOnCode();
            batteryOffTransmitted = false;

            if (ENABLE_DEBUG) Serial.println("-> Normal voice: sending BATTERY ON");
        }

        for (int i = 0; i < bufferIndex; i++) {
            if (!hasPacketSpace(1)) {
                sendPacket();
            }

            if (hasPacketSpace(1)) {
                appendPacketByte(sampleBuffer[i]);
                stats.totalVoiceBytes++;
            }
        }
    }

    bufferIndex = 0;
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    if (status == ESP_NOW_SEND_SUCCESS) {
        stats.packetsSent++;
    } else {
        stats.sendFailures++;
        Serial.println("Send failed");
    }
}

void sendPacket() {
    if (currentPacket.voiceDataSize == 0) return;

    ensureCurrentPacketTimestamp();

    uint8_t buffer[MAX_PACKET_SIZE];
    size_t offset = 0;

    PacketMetadata metadata = {};
    metadata.voiceDataSize = currentPacket.voiceDataSize;
    metadata.containsInterleavedData = currentPacket.containsInterleavedData;
    metadata.timestamp = currentPacket.timestamp;

    // Send only payload + packed metadata. This avoids struct padding and bool size ambiguity.
    memcpy(buffer + offset, currentPacket.voiceData, currentPacket.voiceDataSize);
    offset += currentPacket.voiceDataSize;

    memcpy(buffer + offset, &metadata, PACKET_METADATA_SIZE);
    offset += PACKET_METADATA_SIZE;

    esp_err_t result = esp_now_send(receiverMAC, buffer, offset);

    if (result == ESP_OK && ENABLE_DEBUG) {
        Serial.printf("Packet sent: %u payload bytes", currentPacket.voiceDataSize);
        if (currentPacket.containsInterleavedData) Serial.print(" [INTERLEAVED]");
        Serial.println();
    } else if (result != ESP_OK) {
        Serial.printf("Send error code: %d\n", result);
    }

    currentPacket.voiceDataSize = 0;
    currentPacket.containsInterleavedData = 0;
    currentPacket.timestamp = 0;
}

void displayStatistics() {
    unsigned long totalDitmcBytes = stats.totalVoiceBytes + stats.interleavedDataBytes;
    unsigned long totalTransmittedBytes = totalDitmcBytes + stats.batteryOffSent +
                                          stats.batteryOnSent + stats.totalWatermarkBytesSent;

    float interleavingPercentage = 0.0;
    float overheadPercentage = 0.0;
    float watermarkOverheadPercentage = 0.0;

    if (totalDitmcBytes > 0) {
        interleavingPercentage = (float)stats.interleavedDataBytes * 100.0 / totalDitmcBytes;
    }

    if (totalTransmittedBytes > 0) {
        overheadPercentage = (float)(stats.batteryOffSent + stats.batteryOnSent) * 100.0 /
                             totalTransmittedBytes;
        watermarkOverheadPercentage = (float)stats.totalWatermarkBytesSent * 100.0 /
                                      totalTransmittedBytes;
    }

    Serial.println("\n========== DITMC STATISTICS (Transmitter) ==========");
    Serial.printf("Voice bytes sent:             %lu\n", stats.totalVoiceBytes);
    Serial.printf("Redundant bytes:              %lu\n", stats.redundantBytes);
    Serial.printf("Interleaved data bytes:       %lu\n", stats.interleavedDataBytes);
    Serial.printf("Watermark bytes sent:         %lu\n", stats.totalWatermarkBytesSent);
    Serial.printf("Interleaving percentage:      %.2f%%\n", interleavingPercentage);
    Serial.printf("Marker overhead:              %.2f%%\n", overheadPercentage);
    Serial.printf("Watermark overhead:           %.2f%%\n", watermarkOverheadPercentage);
    Serial.printf("BATTERY OFF codes:            %lu\n", stats.batteryOffSent);
    Serial.printf("BATTERY ON codes:             %lu\n", stats.batteryOnSent);
    Serial.printf("Watermark blocks sent:        %lu\n", stats.watermarkBlocksSent);
    Serial.printf("Watermark blocks skipped:     %lu\n", stats.watermarkBlocksSkipped);
    Serial.printf("Packets sent:                 %lu\n", stats.packetsSent);
    Serial.printf("Send failures:                %lu\n", stats.sendFailures);
    Serial.println("====================================================\n");
}

void setup_transmitter() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\nDITMC Transmitter - ESP32-S3\n");

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    Serial.print("Transmitter MAC Address: ");
    Serial.println(WiFi.macAddress());

    esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);

    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW initialization failed!");
        return;
    }
    Serial.println("ESP-NOW initialized");

    esp_now_register_send_cb(OnDataSent);

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, receiverMAC, 6);
    peerInfo.channel = WIFI_CHANNEL;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add receiver peer!");
        return;
    }

    Serial.print("Receiver registered: ");
    for (int i = 0; i < 6; i++) {
        Serial.printf("%02X", receiverMAC[i]);
        if (i < 5) Serial.print(":");
    }
    Serial.println("\n");

    currentPacket.voiceDataSize = 0;
    currentPacket.containsInterleavedData = 0;
    currentPacket.timestamp = 0;

    simulationStartTime = millis();
    lastStatsTime = millis();

    Serial.println("Starting DITMC transmission with payload-level watermarking...\n");
    digitalWrite(LED_PIN, HIGH);
}

void loop_transmitter() {
    unsigned long currentTime = millis();

    if (USE_SIMULATED_VOICE &&
        (currentTime - simulationStartTime > SIMULATION_DURATION)) {
        if (currentPacket.voiceDataSize > 0) sendPacket();

        displayStatistics();
        Serial.println("Simulation complete. Entering idle mode...");
        digitalWrite(LED_PIN, LOW);

        while (1) delay(1000);
    }

    if (USE_SIMULATED_VOICE) {
        uint8_t sample = generateVoiceSample();
        sampleBuffer[bufferIndex++] = sample;

        if (bufferIndex >= SAMPLE_BUFFER_SIZE) {
            processSamples();
        }
    }

    if (currentTime - lastSendTime >= SEND_INTERVAL_MS) {
        if (bufferIndex > 0) processSamples();
        sendPacket();
        lastSendTime = currentTime;
    }

    if (currentTime - lastStatsTime >= STATS_INTERVAL) {
        displayStatistics();
        lastStatsTime = currentTime;
    }

    delay(1);
}

#endif // TRANSMITTER_H
