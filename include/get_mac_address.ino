/*
 * ESP32 MAC Address Detection
 * 
 * Upload this sketch to BOTH ESP32 boards to get their MAC addresses.
 * You'll need these addresses to configure the transmitter and receiver.
 * 
 * STEPS:
 * 1. Upload this to Transmitter ESP32, open Serial Monitor, record MAC
 * 2. Upload this to Receiver ESP32, open Serial Monitor, record MAC
 * 3. Update transmitter/config.h with Receiver's MAC address
 */

#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║     ESP32-S3 MAC Address Detector      ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  
  // Set WiFi to station mode
  WiFi.mode(WIFI_STA);
  delay(100);
  
  // Get MAC address
  String macAddress = WiFi.macAddress();
  
  Serial.println("📡 Your ESP32 MAC Address:");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println(macAddress);
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
  
  // Convert to array format for easy copying
  Serial.println("For config.h, use this format:");
  Serial.print("uint8_t MAC[] = {");
  
  uint8_t mac[6];
  WiFi.macAddress(mac);
  
  for (int i = 0; i < 6; i++) {
    Serial.printf("0x%02X", mac[i]);
    if (i < 5) Serial.print(", ");
  }
  Serial.println("};\n");
  
  // Instructions
  Serial.println("📝 NEXT STEPS:");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("1. WRITE DOWN this MAC address");
  Serial.println("2. Label this ESP32 (e.g., 'Transmitter' or 'Receiver')");
  Serial.println("3. Upload this sketch to your OTHER ESP32");
  Serial.println("4. Record that MAC address too");
  Serial.println("5. Update transmitter/config.h with RECEIVER's MAC\n");
  
  Serial.println("✓ Done! You can now upload the main sketches.");
}

void loop() {
  // Blink LED to show it's running
  static unsigned long lastBlink = 0;
  static bool ledState = false;
  
  if (millis() - lastBlink > 1000) {
    ledState = !ledState;
    digitalWrite(2, ledState); // LED on pin 2
    lastBlink = millis();
  }
}
