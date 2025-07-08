#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include "GPIOS.h"
#include "config.h"
#include "rfid_handler.h"
#include "google_forms.h"



RFIDHandler rfidHandler;
GoogleFormsHandler googleForms(WIFI_SSID, WIFI_PASSWORD, GOOGLE_FORM_URL, UID_FIELD_ID, TYPE_FIELD_ID, USER_FIELD_ID);

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("ESP32 WROOM 32 RFID Reader Starting...");
    
    if (!rfidHandler.initialize()) {
        Serial.println("RFID Reader initialization failed!");
        Serial.println("Check wiring and connections");
        while(1) {
            delay(1000);
        }
    }
    
    Serial.println("Connecting to WiFi...");
    if (googleForms.connectToWiFi()) {
        Serial.println("WiFi connected successfully!");
        Serial.print("IP address: ");
        Serial.println(googleForms.getLocalIP());
    } else {
        Serial.println("WiFi connection failed - continuing in offline mode");
    }
    
    Serial.println("System ready. Place a card near the reader...");
}

void loop() {
    CardInfo cardInfo = rfidHandler.readCard();
    
    if (cardInfo.isValid) {
        Serial.println("\n=== CARD DETECTED ===");
        Serial.print("UID: ");
        Serial.println(cardInfo.uid);
        Serial.print("Type: ");
        Serial.println(cardInfo.type);
        
        rfidHandler.readAllBlocks();
        
        if (googleForms.isWiFiConnected()) {
            Serial.println("Sending data to Google Forms...");
            bool success = googleForms.sendCardData(cardInfo.uid, cardInfo.type);
            if (success) {
                Serial.println("Data sent successfully!");
            } else {
                Serial.println("Failed to send data to Google Forms");
            }
        } else {
            Serial.println("WiFi not connected - data not sent to Google Forms");
        }
        
        Serial.println("=== END CARD PROCESSING ===\n");
        Serial.println("Ready for next card...");
    }
    
    delay(50);
}