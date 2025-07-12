#include <Arduino.h>
#include <Wire.h>
#include "GPIOS.h"
#include "config.h"
#include "rfid_handler.h"
#include "google_forms.h"

RFIDHandler rfidHandler;
GoogleFormsHandler googleForms(WIFI_SSID, WIFI_PASSWORD, GOOGLE_FORM_URL, 
                              UID_FIELD_ID, TYPE_FIELD_ID, LOCATION_FIELD_ID,
                              PLATE_FIELD_ID, VEHICLE_FIELD_ID, DEPARTMENT_FIELD_ID);

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("ESP32 WROOM 32 Enhanced PN532 NFC/RFID Reader Starting...");
    Serial.print("Device Location: ");
    Serial.println(DEVICE_LOCATION);
    Serial.print("I2C Pins - SDA: ");
    Serial.print(SDA_PIN);
    Serial.print(", SCL: ");
    Serial.println(SCL_PIN);
    
    if (!rfidHandler.initialize()) {
        Serial.println("PN532 NFC/RFID Reader initialization failed!");
        Serial.println("Check I2C wiring and connections");
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
    
    Serial.println("System ready. Place an NFC/RFID card near the reader...");
    Serial.println("Reading blocks 4, 5, 6 for plate, vehicle, and department data");
}

void loop() {
    CardInfo cardInfo = rfidHandler.readCard();
    
    if (cardInfo.isValid) {
        Serial.println("\n=== CARD PROCESSING ===");
        Serial.print("Location: ");
        Serial.println(DEVICE_LOCATION);
        Serial.print("Plate: ");
        Serial.println(cardInfo.plate.length() > 0 ? cardInfo.plate : "N/A");
        Serial.print("Vehicle: ");
        Serial.println(cardInfo.vehicle.length() > 0 ? cardInfo.vehicle : "N/A");
        Serial.print("Department: ");
        Serial.println(cardInfo.department.length() > 0 ? cardInfo.department : "N/A");
        
        rfidHandler.readAllBlocks();
        
        if (googleForms.isWiFiConnected()) {
            Serial.println("Sending enhanced data to Google Forms...");
            bool success = googleForms.sendCardData(cardInfo.uid, cardInfo.type, DEVICE_LOCATION,
                                                   cardInfo.plate, cardInfo.vehicle, cardInfo.department);
            if (success) {
                Serial.println("Enhanced data sent successfully!");
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