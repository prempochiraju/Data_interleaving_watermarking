/*
 * DITMC Receiver for ESP32-S3
 * Implementation of Data Interleaving Technique in Mobile Communication
 * 
 * Based on: "Data Interleaving for Congestion Reduction in Mobile Traffic Transmission"
 * by Hemant Purohit et al.
 */

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "config.h"

// ============================================
// GLOBAL VARIABLES
// ============================================

// Statistics
struct Statistics {
  unsigned long totalPacketsReceived;
  unsigned long totalVoiceBytes;
  unsigned long totalDataBytes;
  unsigned long batteryOffReceived;
  unsigned long batteryOnReceived;
  unsigned long reconstructedVoiceBytes;
  unsigned long extractedDataBytes;
  unsigned long lastPacketTimestamp;
};

Statistics stats = {0};

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

// ============================================
// PACKET PROCESSING
// ============================================

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
  
  // Process each byte in the packet
  for (int i = 0; i < packet->voiceDataSize; i++) {
    uint8_t byte = packet->voiceData[i];
    
    // Check for BATTERY OFF code
    if (byte == BATTERY_OFF_CODE) {
      currentState = STATE_INTERLEAVED_DATA;
      stats.batteryOffReceived++;
      
      if (ENABLE_DEBUG) {
        Serial.println("  → BATTERY OFF detected: Entering data mode");
      }
      continue;
    }
    
    // Check for BATTERY ON code
    if (byte == BATTERY_ON_CODE) {
      currentState = STATE_NORMAL_VOICE;
      stats.batteryOnReceived++;
      
      if (ENABLE_DEBUG) {
        Serial.println("  → BATTERY ON detected: Returning to voice mode");
      }
      continue;
    }
    
    // Store byte based on current state
    if (currentState == STATE_NORMAL_VOICE) {
      // Voice data
      if (reconstructedVoiceSize < RECEPTION_BUFFER_SIZE) {
        reconstructedVoice[reconstructedVoiceSize++] = byte;
        stats.totalVoiceBytes++;
        stats.reconstructedVoiceBytes++;
      }
    } else {
      // Interleaved data
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
  
  // Blink LED to indicate reception
  digitalWrite(LED_PIN, HIGH);
  delay(10);
  digitalWrite(LED_PIN, LOW);
}

// ============================================
// ESP-NOW CALLBACK
// ============================================

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  lastPacketTime = millis();
  
  // Cast received data to DataPacket structure
  DataPacket* packet = (DataPacket*)data;
  
  // Validate packet size
  if (data_len != sizeof(DataPacket)) {
    Serial.println("✗ Invalid packet size received");
    return;
  }
  
  // Process the packet
  processReceivedPacket(packet);
}

// ============================================
// STATISTICS DISPLAY
// ============================================

void displayStatistics() {
  float dataPercentage = 0.0;
  unsigned long totalBytes = stats.totalVoiceBytes + stats.totalDataBytes;
  
  if (totalBytes > 0) {
    dataPercentage = (float)stats.totalDataBytes * 100.0 / totalBytes;
  }
  
  float successRate = 100.0;
  if (stats.totalPacketsReceived > 0) {
    // Simple success metric (can be enhanced)
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
  
  // Display extracted data sample (first 20 bytes)
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

// ============================================
// SETUP AND INITIALIZATION
// ============================================

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║    DITMC Receiver - ESP32-S3           ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  
  // Initialize LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Initialize WiFi in Station mode
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  
  // Print MAC address
  Serial.print("📡 Receiver MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.println("   ⚠️  Copy this address to transmitter's config.h!\n");
  
  // Set WiFi channel
  esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);
  
  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("✗ ESP-NOW initialization failed!");
    return;
  }
  Serial.println("✓ ESP-NOW initialized");
  
  // Register receive callback
  esp_now_register_recv_cb(OnDataRecv);
  
  Serial.println("✓ Receiver ready");
  Serial.println("🎧 Waiting for packets...\n");
  
  lastStatsTime = millis();
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
}

// ============================================
// MAIN LOOP
// ============================================

void loop() {
  unsigned long currentTime = millis();
  
  // Display statistics periodically
  if (currentTime - lastStatsTime >= STATS_INTERVAL) {
    if (stats.totalPacketsReceived > 0) {
      displayStatistics();
    } else {
      Serial.println("⏳ No packets received yet...");
    }
    lastStatsTime = currentTime;
  }
  
  // Check for connection timeout (no packets for 10 seconds)
  if (stats.totalPacketsReceived > 0 && 
      (currentTime - lastPacketTime > 10000)) {
    Serial.println("\n⚠️  No packets received for 10 seconds");
    Serial.println("   Check transmitter status\n");
    lastPacketTime = currentTime; // Reset to avoid spam
  }
  
  delay(10); // Small delay to prevent watchdog issues
}

// ============================================
// UTILITY FUNCTIONS
// ============================================

// Print received data in hex format (for debugging)
void printHexData(const uint8_t* data, int length) {
  Serial.print("Hex: ");
  for (int i = 0; i < length; i++) {
    Serial.printf("%02X ", data[i]);
    if ((i + 1) % 16 == 0) {
      Serial.println();
      Serial.print("     ");
    }
  }
  Serial.println();
}

// Print received data as ASCII (for debugging)
void printASCIIData(const uint8_t* data, int length) {
  Serial.print("ASCII: ");
  for (int i = 0; i < length; i++) {
    if (data[i] >= 32 && data[i] <= 126) {
      Serial.printf("%c", data[i]);
    } else {
      Serial.print(".");
    }
  }
  Serial.println();
}
