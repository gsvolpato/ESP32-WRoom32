#include <Arduino.h>
#include <Wire.h>
#include "GPIOS.h"
#include "config.h"
#include "rfid_handler.h"
#include "postgres_handler.h"

RFIDHandler rfidHandler;
PostgresHandler postgresHandler(WIFI_SSID, WIFI_PASSWORD, POSTGRES_API_URL);

void setup() {
    Serial.begin(115200);
    
    // Initialize onboard LED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW); // Start with LED off (waiting state)
    
    Serial.println("\n========================================");
    Serial.println("ESP32 WROOM 32 - PN532 NFC/RFID Reader");
    Serial.println("========================================");
    Serial.print("Location: ");
    Serial.println(DEVICE_LOCATION);
    
    // Step 1: Connect to WiFi first
    Serial.println("\n[1/2] Connecting to WiFi...");
    if (postgresHandler.connectToWiFi()) {
        Serial.print("WiFi connected! IP: ");
        Serial.println(postgresHandler.getLocalIP());
    } else {
        Serial.println("WiFi failed - continuing in offline mode");
    }
    
    // Step 2: Initialize RFID reader
    Serial.println("\n[2/2] Initializing PN532 RFID reader...");
    Serial.println("(This may take a few seconds, errors during init are normal)");
    
    if (!rfidHandler.initialize()) {
        Serial.println("\nPN532 initialization failed!");
        Serial.println("System will continue but RFID will not work.");
        Serial.print("Check wiring: SDA=GPIO"); Serial.print(SDA_PIN);
        Serial.print(", SCL=GPIO"); Serial.print(SCL_PIN);
        Serial.println(", VCC=3.3V, GND=GND");
        Serial.println("Will retry automatically in loop if needed.");
    }
    
    Serial.println("\nSystem ready. Waiting for RFID cards...");
}

void loop() {
    // Try to initialize if not already done
    if (!rfidHandler.isInitialized()) {
        digitalWrite(LED_PIN, LOW); // LED off while waiting for initialization
        return;
    }
    
    CardInfo cardInfo = rfidHandler.readCard();
    
    if (cardInfo.isValid) {
        // Turn LED ON when card is detected
        digitalWrite(LED_PIN, HIGH);
        
        // Send to PostgreSQL IMMEDIATELY after reading (before Serial prints)
        bool sendSuccess = false;
        if (postgresHandler.isWiFiConnected()) {
            sendSuccess = postgresHandler.sendCardData(cardInfo.uid, cardInfo.type, DEVICE_LOCATION,
                                                     cardInfo.plate, cardInfo.vehicle, cardInfo.department);
        }
        
        // Print to Serial after sending (non-blocking)
        Serial.println("\n--- Card Detected ---");
        Serial.print("UID: ");
        Serial.println(cardInfo.uid);
        Serial.print("Type: ");
        Serial.println(cardInfo.type);
        
        if (cardInfo.plate.length() > 0) {
            Serial.print("Plate: ");
            Serial.println(cardInfo.plate);
        }
        if (cardInfo.vehicle.length() > 0) {
            Serial.print("Vehicle: ");
            Serial.println(cardInfo.vehicle);
        }
        if (cardInfo.department.length() > 0) {
            Serial.print("Department: ");
            Serial.println(cardInfo.department);
        }
        
        if (postgresHandler.isWiFiConnected()) {
            Serial.print("PostgreSQL: ");
            Serial.println(sendSuccess ? "OK" : "FAILED");
        } else {
            Serial.println("WiFi not connected - data not sent");
        }
        
        Serial.println("Ready for next card...\n");
        
        // Turn LED OFF immediately after sending
        digitalWrite(LED_PIN, LOW);
    } else {
        // No card detected - LED OFF (waiting state)
        digitalWrite(LED_PIN, LOW);
    }
    
    delay(50); // Reduced delay for faster scanning
}