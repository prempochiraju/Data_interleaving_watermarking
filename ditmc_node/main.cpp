/*
 * DITMC Implementation for ESP32-S3 using PlatformIO
 * 
 * Based on: "Data Interleaving for Congestion Reduction in Mobile Traffic Transmission"
 * by Hemant Purohit et al.
 * 
 * Build with different environments:
 * - platformio run -e get_mac          (Get MAC address)
 * - platformio run -e esp32s3_transmitter (Build transmitter)
 * - platformio run -e esp32s3_receiver    (Build receiver)
 */

#include <Arduino.h>

// Include the appropriate module based on build flag
#if DITMC_MODE == GET_MAC
    #include "get_mac.h"
#elif DITMC_MODE == TRANSMITTER
    #include "transmitter.h"
#elif DITMC_MODE == RECEIVER
    #include "receiver.h"
#else
    #error "DITMC_MODE not defined! Use one of: GET_MAC, TRANSMITTER, RECEIVER"
#endif

void setup() {
    // Call the appropriate setup based on mode
    #if DITMC_MODE == GET_MAC
        setup_get_mac();
    #elif DITMC_MODE == TRANSMITTER
        setup_transmitter();
    #elif DITMC_MODE == RECEIVER
        setup_receiver();
    #endif
}

void loop() {
    // Call the appropriate loop based on mode
    #if DITMC_MODE == GET_MAC
        loop_get_mac();
    #elif DITMC_MODE == TRANSMITTER
        loop_transmitter();
    #elif DITMC_MODE == RECEIVER
        loop_receiver();
    #endif
}
