#ifndef TRANSMITTER_H
#define TRANSMITTER_H

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "config.h"

// Statistics structure
struct Statistics {
    unsigned long totalVoiceBytes;
    unsigned long redundantBytes;
    unsigned long interleavedDataBytes;
    unsigned long batteryOffSent;
    unsigned long batteryOnSent;
    unsigned long packetsSent;
    unsigned long sendFailures;
};

Statistics stats = {0};

// State variables
bool batteryOffTransmitted = false;
unsigned long lastSendTime = 0;
unsigned long lastStatsTime = 0;
unsigned long simulationStartTime = 0;

// Sample buffer
uint8_t sampleBuffer[SAMPLE_BUFFER_SIZE];
int bufferIndex = 0;

// Packet structure
struct DataPacket {
    uint8_t voiceData[MAX_PACKET_SIZE];
    uint16_t voiceDataSize;
    bool containsInterleavedData;
    uint16_t timestamp;
};

DataPacket currentPacket;

// Generate simulated voice data
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

// Check for repetition
bool checkForRepetition(uint8_t* buffer, int size, int threshold) {
    if (size < threshold) return false;
    
    int count = 1;
    for (int i = 1; i < size; i++) {
        if (buffer[i] == buffer[i-1]) {
            count++;
            if (count >= threshold) return true;
        } else {
            count = 1;
        }
    }
    return false;
}

// Check for silence
bool checkForSilence(uint8_t* buffer, int size, int threshold) {
    if (size < threshold) return false;
    
    int silenceCount = 0;
    for (int i = 0; i < size; i++) {
        if (buffer[i] == 0x00) silenceCount++;
    }
    return (silenceCount >= threshold);
}

// Process samples with DITMC
void processSamples() {
    if (bufferIndex < REPETITION_THRESHOLD) return;
    
    bool hasRepetition = checkForRepetition(sampleBuffer, bufferIndex, REPETITION_THRESHOLD);
    bool hasSilence = checkForSilence(sampleBuffer, bufferIndex, REPETITION_THRESHOLD);
    
    if (hasSilence) {
        if (!batteryOffTransmitted) {
            currentPacket.voiceData[currentPacket.voiceDataSize++] = BATTERY_OFF_CODE;
            stats.batteryOffSent++;
            batteryOffTransmitted = true;
            
            uint8_t interleavedData[] = {'D', 'A', 'T', 'A'};
            for (int i = 0; i < 4; i++) {
                if (currentPacket.voiceDataSize < MAX_PACKET_SIZE - 1) {
                    currentPacket.voiceData[currentPacket.voiceDataSize++] = interleavedData[i];
                    stats.interleavedDataBytes++;
                }
            }
            currentPacket.containsInterleavedData = true;
            
            if (ENABLE_DEBUG) Serial.println("→ Silence detected: Interleaving data");
        }
        stats.redundantBytes += bufferIndex;
    }
    else if (hasRepetition) {
        if (!batteryOffTransmitted) {
            currentPacket.voiceData[currentPacket.voiceDataSize++] = sampleBuffer[0];
            stats.totalVoiceBytes++;
            
            currentPacket.voiceData[currentPacket.voiceDataSize++] = BATTERY_OFF_CODE;
            stats.batteryOffSent++;
            batteryOffTransmitted = true;
            
            uint8_t interleavedData[] = {'I', 'N', 'T', 'R'};
            for (int i = 0; i < 4; i++) {
                if (currentPacket.voiceDataSize < MAX_PACKET_SIZE - 1) {
                    currentPacket.voiceData[currentPacket.voiceDataSize++] = interleavedData[i];
                    stats.interleavedDataBytes++;
                }
            }
            currentPacket.containsInterleavedData = true;
            
            if (ENABLE_DEBUG) Serial.println("→ Repetition detected: Interleaving data");
        }
        stats.redundantBytes += (bufferIndex - 1);
    }
    else {
        if (batteryOffTransmitted) {
            currentPacket.voiceData[currentPacket.voiceDataSize++] = BATTERY_ON_CODE;
            stats.batteryOnSent++;
            batteryOffTransmitted = false;
            
            if (ENABLE_DEBUG) Serial.println("→ Normal voice: Sending BATTERY ON");
        }
        
        for (int i = 0; i < bufferIndex; i++) {
            if (currentPacket.voiceDataSize < MAX_PACKET_SIZE) {
                currentPacket.voiceData[currentPacket.voiceDataSize++] = sampleBuffer[i];
                stats.totalVoiceBytes++;
            }
        }
    }
    
    bufferIndex = 0;
}

// ESP-NOW send callback
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    if (status == ESP_NOW_SEND_SUCCESS) {
        stats.packetsSent++;
    } else {
        stats.sendFailures++;
        Serial.println("✗ Send failed");
    }
}

// Send packet
void sendPacket() {
    if (currentPacket.voiceDataSize == 0) return;
    
    currentPacket.timestamp = (uint16_t)(millis() & 0xFFFF);
    
    esp_err_t result = esp_now_send(receiverMAC, (uint8_t*)&currentPacket, sizeof(currentPacket));
    
    if (result == ESP_OK && ENABLE_DEBUG) {
        Serial.printf("✓ Packet sent: %d voice bytes", currentPacket.voiceDataSize);
        if (currentPacket.containsInterleavedData) Serial.print(" [INTERLEAVED]");
        Serial.println();
    }
    
    currentPacket.voiceDataSize = 0;
    currentPacket.containsInterleavedData = false;
}

// Display statistics
void displayStatistics() {
    float interleavingPercentage = 0.0;
    if (stats.totalVoiceBytes > 0) {
        interleavingPercentage = (float)stats.interleavedDataBytes * 100.0 / 
                                 (stats.totalVoiceBytes + stats.interleavedDataBytes);
    }
    
    float overheadPercentage = (float)(stats.batteryOffSent + stats.batteryOnSent) * 100.0 / 
                               (stats.totalVoiceBytes + stats.interleavedDataBytes + 
                                stats.batteryOffSent + stats.batteryOnSent);
    
    Serial.println("\n╔════════════════════════════════════════╗");
    Serial.println("║      DITMC STATISTICS (Transmitter)    ║");
    Serial.println("╠════════════════════════════════════════╣");
    Serial.printf("║ Voice bytes sent:       %-14lu ║\n", stats.totalVoiceBytes);
    Serial.printf("║ Redundant bytes:        %-14lu ║\n", stats.redundantBytes);
    Serial.printf("║ Interleaved data bytes: %-14lu ║\n", stats.interleavedDataBytes);
    Serial.printf("║ Interleaving %%:         %-13.2f%% ║\n", interleavingPercentage);
    Serial.printf("║ Overhead %%:             %-13.2f%% ║\n", overheadPercentage);
    Serial.println("╠════════════════════════════════════════╣");
    Serial.printf("║ BATTERY OFF codes:      %-14lu ║\n", stats.batteryOffSent);
    Serial.printf("║ BATTERY ON codes:       %-14lu ║\n", stats.batteryOnSent);
    Serial.printf("║ Packets sent:           %-14lu ║\n", stats.packetsSent);
    Serial.printf("║ Send failures:          %-14lu ║\n", stats.sendFailures);
    Serial.println("╚════════════════════════════════════════╝\n");
}

// Setup function
void setup_transmitter() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n╔════════════════════════════════════════╗");
    Serial.println("║   DITMC Transmitter - ESP32-S3         ║");
    Serial.println("╚════════════════════════════════════════╝\n");
    
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    
    Serial.print("📡 Transmitter MAC Address: ");
    Serial.println(WiFi.macAddress());
    
    esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);
    
    if (esp_now_init() != ESP_OK) {
        Serial.println("✗ ESP-NOW initialization failed!");
        return;
    }
    Serial.println("✓ ESP-NOW initialized");
    
    esp_now_register_send_cb(OnDataSent);
    
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, receiverMAC, 6);
    peerInfo.channel = WIFI_CHANNEL;
    peerInfo.encrypt = false;
    
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("✗ Failed to add peer!");
        return;
    }
    
    Serial.print("✓ Receiver registered: ");
    for (int i = 0; i < 6; i++) {
        Serial.printf("%02X", receiverMAC[i]);
        if (i < 5) Serial.print(":");
    }
    Serial.println("\n");
    
    currentPacket.voiceDataSize = 0;
    currentPacket.containsInterleavedData = false;
    
    simulationStartTime = millis();
    lastStatsTime = millis();
    
    Serial.println("🚀 Starting DITMC transmission...\n");
    digitalWrite(LED_PIN, HIGH);
}

// Loop function
void loop_transmitter() {
    unsigned long currentTime = millis();
    
    if (USE_SIMULATED_VOICE && 
        (currentTime - simulationStartTime > SIMULATION_DURATION)) {
        if (currentPacket.voiceDataSize > 0) sendPacket();
        
        displayStatistics();
        Serial.println("✓ Simulation complete. Entering idle mode...");
        digitalWrite(LED_PIN, LOW);
        
        while(1) delay(1000);
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
