#define DISABLE_ALL_LIBRARY_WARNINGS
#ifndef POSTGRES_HANDLER_H
#define POSTGRES_HANDLER_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

class PostgresHandler {
private:
    const char* ssid;
    const char* password;
    const char* apiUrl;
    bool wifiConnected;

public:
    PostgresHandler(const char* wifiSSID, const char* wifiPassword, const char* apiURL);
    
    bool connectToWiFi();
    bool isWiFiConnected();
    bool sendCardData(String uid, String cardType, String location, 
                     String plate, String vehicle, String department);
    void checkWiFiConnection();
    String getLocalIP();
};

#endif

