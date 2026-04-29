#ifndef RECEIVER_H
#define RECEIVER_H

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "config.h"

// Statistics structure
struct RxStatistics {
    unsigned long totalPacketsReceived;
    unsigned long totalVoiceBytes;
    unsigned long totalDataBytes;
    unsigned long batteryOffReceived;
    unsigned long batteryOnReceived;
    unsigned long reconstructedVoiceBytes;
    unsigned long extractedDataBytes;
    unsigned long lastPacketTimestamp;
};

RxStatistics stats = {0};

// Reception state
enum ReceiverState {
    STATE_NORMAL_VOICE,
    STATE_INTERLEAVED_DATA
};

ReceiverState currentState = STATE_NORMAL_VOICE;

// Buffers
uint8_t reconstructedVoice[RECEPTION_BUFFER_SIZE];
uint16_t reconstructedVoiceSize = 0;

uint8_t extractedData[RECEPTION_BUFFER_SIZE];
uint16_t extractedDataSize = 0;

unsigned long lastStatsTime = 0;
unsigned long lastPacketTime = 0;

// Packet structure (must match transmitter)
struct DataPacket {
    uint8_t voiceData[MAX_PACKET_SIZE];
    uint16_t voiceDataSize;
    bool containsInterleavedData;
    uint16_t timestamp;
};

// Process received packet
void processReceivedPacket(const DataPacket* packet) {
    stats.totalPacketsReceived++;
    stats.lastPacketTimestamp = packet->timestamp;
    
    if (ENABLE_DEBUG) {
        Serial.printf("\n📦 Packet received [#%lu]: %d bytes", 
                      stats.totalPacketsReceived, packet->voiceDataSize);
        if (packet->containsInterleavedData) {
            Serial.print(" [INTERLEAVED]");
        }
        Serial.println();
    }
    
    for (int i = 0; i < packet->voiceDataSize; i++) {
        uint8_t byte = packet->voiceData[i];
        
        if (byte == BATTERY_OFF_CODE) {
            currentState = STATE_INTERLEAVED_DATA;
            stats.batteryOffReceived++;
            
            if (ENABLE_DEBUG) {
                Serial.println("  → BATTERY OFF detected: Entering data mode");
            }
            continue;
        }
        
        if (byte == BATTERY_ON_CODE) {
            currentState = STATE_NORMAL_VOICE;
            stats.batteryOnReceived++;
            
            if (ENABLE_DEBUG) {
                Serial.println("  → BATTERY ON detected: Returning to voice mode");
            }
            continue;
        }
        
        if (currentState == STATE_NORMAL_VOICE) {
            if (reconstructedVoiceSize < RECEPTION_BUFFER_SIZE) {
                reconstructedVoice[reconstructedVoiceSize++] = byte;
                stats.totalVoiceBytes++;
                stats.reconstructedVoiceBytes++;
            }
        } else {
            if (extractedDataSize < RECEPTION_BUFFER_SIZE) {
                extractedData[extractedDataSize++] = byte;
                stats.totalDataBytes++;
                stats.extractedDataBytes++;
            }
        }
    }
    
    if (ENABLE_DEBUG) {
        Serial.printf("  Voice: %d bytes | Data: %d bytes\n", 
                      stats.reconstructedVoiceBytes, stats.extractedDataBytes);
    }
    
    digitalWrite(LED_PIN, HIGH);
    delay(10);
    digitalWrite(LED_PIN, LOW);
}

// ESP-NOW receive callback
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
    lastPacketTime = millis();
    
    DataPacket* packet = (DataPacket*)data;
    
    if (data_len != sizeof(DataPacket)) {
        Serial.println("✗ Invalid packet size received");
        return;
    }
    
    processReceivedPacket(packet);
}

// Display statistics
void displayStatistics() {
    float dataPercentage = 0.0;
    unsigned long totalBytes = stats.totalVoiceBytes + stats.totalDataBytes;
    
    if (totalBytes > 0) {
        dataPercentage = (float)stats.totalDataBytes * 100.0 / totalBytes;
    }
    
    float successRate = 100.0;
    if (stats.totalPacketsReceived > 0) {
        successRate = 100.0;
    }
    
    Serial.println("\n╔════════════════════════════════════════╗");
    Serial.println("║      DITMC STATISTICS (Receiver)       ║");
    Serial.println("╠════════════════════════════════════════╣");
    Serial.printf("║ Packets received:       %-14lu ║\n", stats.totalPacketsReceived);
    Serial.printf("║ Voice bytes received:   %-14lu ║\n", stats.totalVoiceBytes);
    Serial.printf("║ Data bytes received:    %-14lu ║\n", stats.totalDataBytes);
    Serial.printf("║ Data percentage:        %-13.2f%% ║\n", dataPercentage);
    Serial.println("╠════════════════════════════════════════╣");
    Serial.printf("║ BATTERY OFF received:   %-14lu ║\n", stats.batteryOffReceived);
    Serial.printf("║ BATTERY ON received:    %-14lu ║\n", stats.batteryOnReceived);
    Serial.printf("║ Reconstruction rate:    %-13.2f%% ║\n", successRate);
    Serial.println("╠════════════════════════════════════════╣");
    
    Serial.print("║ Data sample: ");
    int sampleSize = min(20, (int)extractedDataSize);
    for (int i = 0; i < sampleSize; i++) {
        if (extractedData[i] >= 32 && extractedData[i] <= 126) {
            Serial.printf("%c", extractedData[i]);
        } else {
            Serial.print(".");
        }
    }
    for (int i = sampleSize; i < 20; i++) {
        Serial.print(" ");
    }
    Serial.println("   ║");
    
    Serial.println("╚════════════════════════════════════════╝\n");
}

// Setup function
void setup_receiver() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n╔════════════════════════════════════════╗");
    Serial.println("║    DITMC Receiver - ESP32-S3           ║");
    Serial.println("╚════════════════════════════════════════╝\n");
    
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    
    Serial.print("📡 Receiver MAC Address: ");
    Serial.println(WiFi.macAddress());
    Serial.println("   ⚠️  Copy this address to include/config.h!\n");
    
    esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);
    
    if (esp_now_init() != ESP_OK) {
        Serial.println("✗ ESP-NOW initialization failed!");
        return;
    }
    Serial.println("✓ ESP-NOW initialized");
    
    esp_now_register_recv_cb(OnDataRecv);
    
    Serial.println("✓ Receiver ready");
    Serial.println("🎧 Waiting for packets...\n");
    
    lastStatsTime = millis();
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
}

// Loop function
void loop_receiver() {
    unsigned long currentTime = millis();
    
    if (currentTime - lastStatsTime >= STATS_INTERVAL) {
        if (stats.totalPacketsReceived > 0) {
            displayStatistics();
        } else {
            Serial.println("⏳ No packets received yet...");
        }
        lastStatsTime = currentTime;
    }
    
    if (stats.totalPacketsReceived > 0 && 
        (currentTime - lastPacketTime > 10000)) {
        Serial.println("\n⚠️  No packets received for 10 seconds");
        Serial.println("   Check transmitter status\n");
        lastPacketTime = currentTime;
    }
    
    delay(10);
}

#endif // RECEIVER_H
