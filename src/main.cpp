#include <Arduino.h>
#include <Wire.h>
#include "GPIOS.h"
#include "config.h"
#include "rfid_handler.h"
#include "google_forms.h"
#include "web_app.h"

RFIDHandler rfidHandler;
GoogleFormsHandler googleForms(WIFI_SSID, WIFI_PASSWORD, GOOGLE_FORM_URL, 
                              UID_FIELD_ID, TYPE_FIELD_ID, LOCATION_FIELD_ID,
                              PLATE_FIELD_ID, VEHICLE_FIELD_ID, DEPARTMENT_FIELD_ID);
WebAppHandler webApp(80);

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("========================================");
    Serial.println("ESP32 WROOM 32 Enhanced PN532 NFC/RFID Reader Starting...");
    Serial.println("========================================");
    Serial.print("Device Location: ");
    Serial.println(DEVICE_LOCATION);
    Serial.print("I2C Pins - SDA: ");
    Serial.print(SDA_PIN);
    Serial.print(", SCL: ");
    Serial.println(SCL_PIN);
    Serial.println("Free Heap: " + String(ESP.getFreeHeap()) + " bytes");
    Serial.println("========================================");
    
    Serial.println("Step 1: Initializing PN532 NFC/RFID Reader...");
    if (!rfidHandler.initialize()) {
        Serial.println("❌ PN532 NFC/RFID Reader initialization failed!");
        Serial.println("❌ Check I2C wiring and connections");
        Serial.println("❌ Possible issues:");
        Serial.println("   - SDA/SCL pins not connected properly");
        Serial.println("   - PN532 not powered");
        Serial.println("   - Wrong I2C address");
        Serial.println("   - Faulty PN532 module");
        Serial.println("❌ System halted. Fix hardware and restart.");
        while(1) {
            delay(1000);
        }
    }
    Serial.println("✅ PN532 initialized successfully!");
    Serial.println("========================================");
    
    Serial.println("Step 2: Connecting to WiFi...");
    Serial.print("SSID: ");
    Serial.println(WIFI_SSID);
    Serial.print("Attempting connection");
    
    if (googleForms.connectToWiFi()) {
        Serial.println("\n✅ WiFi connected successfully!");
        Serial.print("✅ IP address: ");
        Serial.println(googleForms.getLocalIP());
        Serial.print("✅ Gateway: ");
        Serial.println(WiFi.gatewayIP());
        Serial.print("✅ Subnet: ");
        Serial.println(WiFi.subnetMask());
        Serial.print("✅ DNS: ");
        Serial.println(WiFi.dnsIP());
        Serial.print("✅ Signal strength: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
        Serial.println("========================================");
        
        Serial.println("Step 3: Starting web interface...");
        webApp.initialize(&rfidHandler);
        Serial.println("✅ Web interface started!");
        Serial.println("🌐 Access the web interface at: http://" + googleForms.getLocalIP());
        Serial.println("========================================");
    } else {
        Serial.println("\n❌ WiFi connection failed - continuing in offline mode");
        Serial.println("❌ Check WiFi credentials in config.h");
        Serial.println("❌ Make sure WiFi network is 2.4GHz");
        Serial.println("❌ Web interface will not be available");
        Serial.println("========================================");
    }
    
    Serial.println("Step 4: System initialization complete!");
    Serial.println("✅ System ready. Place an NFC/RFID card near the reader...");
    Serial.println("📖 Reading blocks 4, 5, 6 for plate, vehicle, and department data");
    Serial.println("🔄 Entering main loop...");
    Serial.println("========================================");
}

void loop() {
    static unsigned long lastHeartbeat = 0;
    static unsigned long cardCount = 0;
    
    webApp.handleClients();
    
    if (millis() - lastHeartbeat > 30000) {
        Serial.println("💓 System heartbeat - Free heap: " + String(ESP.getFreeHeap()) + " bytes");
        Serial.println("📶 WiFi status: " + String(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected"));
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("🌐 IP: " + WiFi.localIP().toString() + " | Signal: " + String(WiFi.RSSI()) + " dBm");
        }
        Serial.println("📊 Cards processed: " + String(cardCount));
        lastHeartbeat = millis();
    }
    
    CardInfo cardInfo = rfidHandler.readCard();
    
    if (cardInfo.isValid) {
        cardCount++;
        Serial.println("\n🎯 === CARD PROCESSING #" + String(cardCount) + " ===");
        Serial.print("📍 Location: ");
        Serial.println(DEVICE_LOCATION);
        Serial.print("🏷️  UID: ");
        Serial.println(cardInfo.uid);
        Serial.print("📋 Type: ");
        Serial.println(cardInfo.type);
        Serial.print("🚗 Plate: ");
        Serial.println(cardInfo.plate.length() > 0 ? cardInfo.plate : "N/A");
        Serial.print("🚙 Vehicle: ");
        Serial.println(cardInfo.vehicle.length() > 0 ? cardInfo.vehicle : "N/A");
        Serial.print("🏢 Department: ");
        Serial.println(cardInfo.department.length() > 0 ? cardInfo.department : "N/A");
        
        Serial.println("📊 Reading all blocks for debugging...");
        rfidHandler.readAllBlocks();
        
        if (googleForms.isWiFiConnected()) {
            Serial.println("📡 Sending enhanced data to Google Forms...");
            bool success = googleForms.sendCardData(cardInfo.uid, cardInfo.type, DEVICE_LOCATION,
                                                   cardInfo.plate, cardInfo.vehicle, cardInfo.department);
            if (success) {
                Serial.println("✅ Enhanced data sent successfully!");
            } else {
                Serial.println("❌ Failed to send data to Google Forms");
            }
        } else {
            Serial.println("❌ WiFi not connected - data not sent to Google Forms");
        }
        
        Serial.println("🎯 === END CARD PROCESSING #" + String(cardCount) + " ===\n");
        Serial.println("⏳ Ready for next card...");
    }
    
    delay(50);
}