#ifndef DITMC_CONFIG_H
#define DITMC_CONFIG_H

// ============================================
// DITMC CONFIGURATION
// ============================================

// IMPORTANT: Replace with your RECEIVER's MAC address!
// Get it by uploading with env:get_mac first
#define RECEIVER_MAC_0 0xFF
#define RECEIVER_MAC_1 0xFF
#define RECEIVER_MAC_2 0xFF
#define RECEIVER_MAC_3 0xFF
#define RECEIVER_MAC_4 0xFF
#define RECEIVER_MAC_5 0xFF

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

// Battery Codes (Data markers)
#define BATTERY_OFF_CODE 0xAA   // Marks start of interleaved data
#define BATTERY_ON_CODE  0x55   // Marks end of interleaved data

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
