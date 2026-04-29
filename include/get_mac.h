#ifndef GET_MAC_H
#define GET_MAC_H

#include <WiFi.h>

void setup_get_mac() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n╔════════════════════════════════════════╗");
    Serial.println("║     ESP32-S3 MAC Address Detector      ║");
    Serial.println("╚════════════════════════════════════════╝\n");
    
    WiFi.mode(WIFI_STA);
    delay(100);
    
    String macAddress = WiFi.macAddress();
    
    Serial.println("📡 Your ESP32 MAC Address:");
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    Serial.println(macAddress);
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    
    uint8_t mac[6];
    WiFi.macAddress(mac);
    
    Serial.println("For include/config.h, update these defines:");
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    Serial.printf("#define RECEIVER_MAC_0 0x%02X\n", mac[0]);
    Serial.printf("#define RECEIVER_MAC_1 0x%02X\n", mac[1]);
    Serial.printf("#define RECEIVER_MAC_2 0x%02X\n", mac[2]);
    Serial.printf("#define RECEIVER_MAC_3 0x%02X\n", mac[3]);
    Serial.printf("#define RECEIVER_MAC_4 0x%02X\n", mac[4]);
    Serial.printf("#define RECEIVER_MAC_5 0x%02X\n", mac[5]);
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    
    Serial.println("📝 NEXT STEPS:");
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    Serial.println("1. WRITE DOWN this MAC address");
    Serial.println("2. Label this ESP32 (e.g., 'Transmitter' or 'Receiver')");
    Serial.println("3. Upload to your OTHER ESP32 with:");
    Serial.println("   pio run -e get_mac -t upload -t monitor");
    Serial.println("4. Record that MAC address too");
    Serial.println("5. Update include/config.h with RECEIVER's MAC\n");
    
    Serial.println("✓ Done! You can now build the main project.");
    
    pinMode(2, OUTPUT);
}

void loop_get_mac() {
    static unsigned long lastBlink = 0;
    static bool ledState = false;
    
    if (millis() - lastBlink > 1000) {
        ledState = !ledState;
        digitalWrite(2, ledState);
        lastBlink = millis();
    }
}

#endif // GET_MAC_H
