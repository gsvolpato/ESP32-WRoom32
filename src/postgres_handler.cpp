#define DISABLE_ALL_LIBRARY_WARNINGS
#include "postgres_handler.h"
#include "config.h"

PostgresHandler::PostgresHandler(const char* wifiSSID, const char* wifiPassword, const char* apiURL) {
    ssid = wifiSSID;
    password = wifiPassword;
    apiUrl = apiURL;
    wifiConnected = false;
}

bool PostgresHandler::connectToWiFi() {
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

bool PostgresHandler::isWiFiConnected() {
    return wifiConnected && (WiFi.status() == WL_CONNECTED);
}

void PostgresHandler::checkWiFiConnection() {
    if (WiFi.status() != WL_CONNECTED) {
        wifiConnected = false;
        Serial.println("WiFi connection lost. Attempting to reconnect...");
        connectToWiFi();
    }
}

String PostgresHandler::getLocalIP() {
    if (isWiFiConnected()) {
        return WiFi.localIP().toString();
    }
    return "Not Connected";
}

bool PostgresHandler::sendCardData(String uid, String cardType, String location, 
                                   String plate, String vehicle, String department) {
    if (!isWiFiConnected()) {
        return false;
    }
    
    checkWiFiConnection();
    if (!isWiFiConnected()) {
        return false;
    }
    
    HTTPClient http;
    http.begin(apiUrl);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(5000);
    
    // Create JSON payload
    StaticJsonDocument<512> doc;
    doc["card_uid"] = uid;
    doc["card_type"] = cardType.length() > 0 ? cardType : "";
    doc["reader_name"] = location.length() > 0 ? location : "PN532";
    
    JsonObject metadata = doc.createNestedObject("metadata");
    if (location.length() > 0) metadata["location"] = location;
    if (plate.length() > 0) metadata["plate"] = plate;
    if (vehicle.length() > 0) metadata["vehicle"] = vehicle;
    if (department.length() > 0) metadata["department"] = department;
    
    String jsonPayload;
    serializeJson(doc, jsonPayload);
    
    int httpResponseCode = http.POST(jsonPayload);
    
    bool success = (httpResponseCode == 200 || httpResponseCode == 201);
    
    if (!success) {
        Serial.print("HTTP Error: ");
        Serial.print(httpResponseCode);
        if (httpResponseCode == -1) {
            Serial.println(" (Connection failed - check if server is running and IP is correct)");
        } else if (httpResponseCode == -11) {
            Serial.println(" (Timeout)");
        } else {
            String response = http.getString();
            if (response.length() > 0) {
                Serial.print(" - Response: ");
                Serial.println(response);
            } else {
                Serial.println();
            }
        }
    }
    
    http.end();
    return success;
}

