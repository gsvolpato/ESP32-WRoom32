#define DISABLE_ALL_LIBRARY_WARNINGS
#include "google_forms.h"
#include "config.h"

GoogleFormsHandler::GoogleFormsHandler(const char* wifiSSID, const char* wifiPassword, 
                                     const char* googleFormURL, const char* uidField, 
                                     const char* typeField, const char* userField) {
    ssid = wifiSSID;
    password = wifiPassword;
    formURL = googleFormURL;
    uidFieldName = uidField;
    typeFieldName = typeField;
    userFieldName = userField;
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

bool GoogleFormsHandler::sendCardData(String uid, String cardType) {
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
    Serial.print("User Field ID: ");
    Serial.println(userFieldName);
    
    String userName = DEVICE_USER_NAME;
    
    Serial.println("\nData being sent:");
    Serial.print("  Card UID: '");
    Serial.print(uid);
    Serial.print("' -> ");
    Serial.println(uidFieldName);
    Serial.print("  Card Type: '");
    Serial.print(cardType);
    Serial.print("' -> ");
    Serial.println(typeFieldName);
    Serial.print("  User: '");
    Serial.print(userName);
    Serial.print("' -> ");
    Serial.println(userFieldName);
    
    Serial.println("\nCreating fresh Google Form instance...");
    GoogleFormPost freshForm;
    
    freshForm.setFormUrl(formURL);
    Serial.println("Form URL set");
    
    freshForm.addData(uid, uidFieldName);
    Serial.println("UID data added");
    
    freshForm.addData(cardType, typeFieldName);
    Serial.println("Card type data added");
    
    freshForm.addData(userName, userFieldName);
    Serial.println("User data added");
    
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