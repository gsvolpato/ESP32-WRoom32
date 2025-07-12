#define DISABLE_ALL_LIBRARY_WARNINGS
#include "google_forms.h"
#include "config.h"

GoogleFormsHandler::GoogleFormsHandler(const char* wifiSSID, const char* wifiPassword, 
                                     const char* googleFormURL, const char* uidField, 
                                     const char* typeField, const char* locationField,
                                     const char* plateField, const char* vehicleField,
                                     const char* departmentField) {
    ssid = wifiSSID;
    password = wifiPassword;
    formURL = googleFormURL;
    uidFieldName = uidField;
    typeFieldName = typeField;
    locationFieldName = locationField;
    plateFieldName = plateField;
    vehicleFieldName = vehicleField;
    departmentFieldName = departmentField;
    wifiConnected = false;
}

bool GoogleFormsHandler::connectToWiFi() {
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        Serial.println("\nWiFi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        return true;
    } else {
        wifiConnected = false;
        Serial.println("\nWiFi connection failed!");
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