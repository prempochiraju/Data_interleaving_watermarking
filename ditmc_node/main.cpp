/*
 * DITMC Implementation for ESP32-S3 using PlatformIO
 */

#include <Arduino.h>

// WORKAROUND: Define numeric values for modes
#ifndef GET_MAC
  #define GET_MAC 0
#endif
#ifndef TRANSMITTER  
  #define TRANSMITTER 1
#endif
#ifndef RECEIVER
  #define RECEIVER 2
#endif

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
    #if DITMC_MODE == GET_MAC
        setup_get_mac();
    #elif DITMC_MODE == TRANSMITTER
        setup_transmitter();
    #elif DITMC_MODE == RECEIVER
        setup_receiver();
    #endif
}

void loop() {
    #if DITMC_MODE == GET_MAC
        loop_get_mac();
    #elif DITMC_MODE == TRANSMITTER
        loop_transmitter();
    #elif DITMC_MODE == RECEIVER
        loop_receiver();
    #endif
}