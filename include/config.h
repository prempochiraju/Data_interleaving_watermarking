#include <cstdint>
#include <cstddef>
#ifndef DITMC_CONFIG_H
#define DITMC_CONFIG_H

// ============================================
// DITMC CONFIGURATION
// ============================================

// IMPORTANT: Replace with your RECEIVER's MAC address!
// Get it by uploading with env:get_mac first
#define RECEIVER_MAC_0 0x10
#define RECEIVER_MAC_1 0xB4
#define RECEIVER_MAC_2 0x1D
#define RECEIVER_MAC_3 0xEB
#define RECEIVER_MAC_4 0x2D
#define RECEIVER_MAC_5 0xA4

// Create MAC array from defines
uint8_t receiverMAC[] = {
    RECEIVER_MAC_0, 
    RECEIVER_MAC_1, 
    RECEIVER_MAC_2, 
    RECEIVER_MAC_3, 
    RECEIVER_MAC_4, 
    RECEIVER_MAC_5
};

// Communication Settings
#define WIFI_CHANNEL 1        // WiFi channel (1-13)
#define SEND_INTERVAL_MS 50   // Time between packet transmissions

// DITMC Algorithm Parameters
#define REPETITION_THRESHOLD 4  // Minimum repetitions to trigger interleaving
#define SAMPLE_BUFFER_SIZE 8    // Number of samples to accumulate
#define MAX_PACKET_SIZE 250     // Maximum ESP-NOW packet size
#define INTERLEAVED_DATA_BLOCK_SIZE 4 // Current DITMC demo data block length

// First watermarked DITMC block sent at transmitter startup.
// The receiver uses this block to reset counters and align logs.
static const uint8_t EXPERIMENT_SYNC_DATA[] = {'S', 'Y', 'N', 'C'};
static const size_t EXPERIMENT_SYNC_DATA_LENGTH = sizeof(EXPERIMENT_SYNC_DATA);
#define EXPERIMENT_SYNC_REPETITIONS 5
#define EXPERIMENT_SYNC_DELAY_MS 500
#define EXPERIMENT_START_DELAY_MS 3000

// Battery Codes (Data markers)
#define BATTERY_OFF_CODE 0xAA   // Marks start of interleaved data
#define BATTERY_ON_CODE  0x55   // Marks end of interleaved data

// Payload-level watermarking.
// Both transmitter and receiver must use the same key.
static const uint8_t SECRET_KEY[] = {
    'D', 'I', 'T', 'M', 'C', '_', 'W', 'M', '_', '2', '0', '2', '6'
};
static const size_t SECRET_KEY_LENGTH = sizeof(SECRET_KEY);

// Debug Settings
#define ENABLE_DEBUG true       // Enable detailed debug output
#define STATS_INTERVAL 5000     // Statistics display interval (ms)

// LED Pin (optional - for status indication)
#define LED_PIN 2               // Built-in LED on most ESP32-S3

// Test Data Configuration
#define USE_SIMULATED_VOICE true  // Use simulated voice data for testing
#define SIMULATION_DURATION 60000 // Simulation duration in ms (60 seconds)

// Reception buffer
#define RECEPTION_BUFFER_SIZE 1024

#endif // DITMC_CONFIG_H
