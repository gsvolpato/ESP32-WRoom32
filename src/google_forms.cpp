#define DISABLE_ALL_LIBRARY_WARNINGS
#include "google_forms.h"
#include "config.h"

GoogleFormsHandler::GoogleFormsHandler(WiFiHandler* wifi, const char* googleFormURL, 
                                     const char* uidField, const char* typeField, 
                                     const char* locationField, const char* plateField, 
                                     const char* vehicleField, const char* departmentField) {
    wifiHandler = wifi;
    formURL = googleFormURL;
    uidFieldName = uidField;
    typeFieldName = typeField;
    locationFieldName = locationField;
    plateFieldName = plateField;
    vehicleFieldName = vehicleField;
    departmentFieldName = departmentField;
    
    Serial.println("🔄 Google Forms Handler initialized");
    Serial.print("📝 Form URL: ");
    Serial.println(formURL);
}

bool GoogleFormsHandler::connectToWiFi() {
    Serial.println("🔄 Starting WiFi connection process...");
    Serial.print("📶 WiFi mode: ");
    Serial.println(WiFi.getMode());
    
    WiFi.mode(WIFI_STA);
    Serial.println("📶 WiFi mode set to STA");
    
    Serial.print("🔗 Attempting to connect to: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
        
        if (attempts % 5 == 0) {
            Serial.print(" [" + String(attempts) + "/20] Status: ");
            switch (WiFi.status()) {
                case WL_IDLE_STATUS: Serial.print("IDLE"); break;
                case WL_NO_SSID_AVAIL: Serial.print("NO_SSID_AVAIL"); break;
                case WL_SCAN_COMPLETED: Serial.print("SCAN_COMPLETED"); break;
                case WL_CONNECTED: Serial.print("CONNECTED"); break;
                case WL_CONNECT_FAILED: Serial.print("CONNECT_FAILED"); break;
                case WL_CONNECTION_LOST: Serial.print("CONNECTION_LOST"); break;
                case WL_DISCONNECTED: Serial.print("DISCONNECTED"); break;
                default: Serial.print("UNKNOWN"); break;
            }
            Serial.println();
        }
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        Serial.println("\n✅ WiFi connected successfully!");
        Serial.print("✅ IP address: ");
        Serial.println(WiFi.localIP());
        Serial.print("✅ MAC address: ");
        Serial.println(WiFi.macAddress());
        return true;
    } else {
        wifiConnected = false;
        Serial.println("\n❌ WiFi connection failed after 20 attempts!");
        Serial.print("❌ Final status: ");
        switch (WiFi.status()) {
            case WL_IDLE_STATUS: Serial.println("IDLE"); break;
            case WL_NO_SSID_AVAIL: Serial.println("NO_SSID_AVAIL - Check SSID name"); break;
            case WL_SCAN_COMPLETED: Serial.println("SCAN_COMPLETED"); break;
            case WL_CONNECT_FAILED: Serial.println("CONNECT_FAILED - Check password"); break;
            case WL_CONNECTION_LOST: Serial.println("CONNECTION_LOST"); break;
            case WL_DISCONNECTED: Serial.println("DISCONNECTED"); break;
            default: Serial.println("UNKNOWN"); break;
        }
        return false;
    }
}

bool GoogleFormsHandler::isWiFiConnected() {
    return wifiConnected && (WiFi.status() == WL_CONNECTED);
}

void GoogleFormsHandler::checkWiFiConnection() {
    if (WiFi.status() != WL_CONNECTED) {
        wifiConnected = false;
        Serial.println("WiFi connection lost. Attempting to reconnect...");
        connectToWiFi();
    }
}

String GoogleFormsHandler::getLocalIP() {
    if (isWiFiConnected()) {
        return WiFi.localIP().toString();
    }
    return "Not Connected";
}

bool GoogleFormsHandler::sendCardData(String uid, String cardType, String location, 
                                    String plate, String vehicle, String department) {
    if (!isWiFiConnected()) {
        Serial.println("WiFi not connected, skipping Google Form submission");
        return false;
    }
    
    checkWiFiConnection();
    
    if (!isWiFiConnected()) {
        return false;
    }
    
    Serial.println("\n=== GOOGLE FORM SUBMISSION DEBUG ===");
    Serial.print("Form URL: ");
    Serial.println(formURL);
    Serial.print("UID Field ID: ");
    Serial.println(uidFieldName);
    Serial.print("Type Field ID: ");
    Serial.println(typeFieldName);
    Serial.print("Location Field ID: ");
    Serial.println(locationFieldName);
    Serial.print("Plate Field ID: ");
    Serial.println(plateFieldName);
    Serial.print("Vehicle Field ID: ");
    Serial.println(vehicleFieldName);
    Serial.print("Department Field ID: ");
    Serial.println(departmentFieldName);
    
    Serial.println("\nData being sent:");
    Serial.print("  Card UID: '");
    Serial.print(uid);
    Serial.print("' -> ");
    Serial.println(uidFieldName);
    Serial.print("  Card Type: '");
    Serial.print(cardType);
    Serial.print("' -> ");
    Serial.println(typeFieldName);
    Serial.print("  Location: '");
    Serial.print(location);
    Serial.print("' -> ");
    Serial.println(locationFieldName);
    Serial.print("  Plate: '");
    Serial.print(plate.length() > 0 ? plate : "N/A");
    Serial.print("' -> ");
    Serial.println(plateFieldName);
    Serial.print("  Vehicle: '");
    Serial.print(vehicle.length() > 0 ? vehicle : "N/A");
    Serial.print("' -> ");
    Serial.println(vehicleFieldName);
    Serial.print("  Department: '");
    Serial.print(department.length() > 0 ? department : "N/A");
    Serial.print("' -> ");
    Serial.println(departmentFieldName);
    
    Serial.println("\nCreating fresh Google Form instance...");
    GoogleFormPost freshForm;
    
    freshForm.setFormUrl(formURL);
    Serial.println("Form URL set");
    
    freshForm.addData(uid, uidFieldName);
    Serial.println("UID data added");
    
    freshForm.addData(cardType, typeFieldName);
    Serial.println("Card type data added");
    
    freshForm.addData(location, locationFieldName);
    Serial.println("Location data added");
    
    freshForm.addData(plate.length() > 0 ? plate : "", plateFieldName);
    Serial.println("Plate data added");
    
    freshForm.addData(vehicle.length() > 0 ? vehicle : "", vehicleFieldName);
    Serial.println("Vehicle data added");
    
    freshForm.addData(department.length() > 0 ? department : "", departmentFieldName);
    Serial.println("Department data added");
    
    Serial.println("\nSending form data...");
    bool result = freshForm.send();
    
    if (result) {
        Serial.println("✅ GoogleFormPost.send() returned SUCCESS");
        Serial.println("Data should appear in your Google Spreadsheet");
        Serial.println("Check: https://forms.gle/sX1vyHPxMdNbNhfx9");
    } else {
        Serial.println("❌ GoogleFormPost.send() returned FAILURE");
        Serial.println("Form submission failed at library level");
    }
    
    Serial.println("=== END GOOGLE FORM DEBUG ===\n");
    return result;
} 