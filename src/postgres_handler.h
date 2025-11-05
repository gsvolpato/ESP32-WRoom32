#define DISABLE_ALL_LIBRARY_WARNINGS
#ifndef POSTGRES_HANDLER_H
#define POSTGRES_HANDLER_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

class PostgresHandler {
private:
    const char* ssid;
    const char* password;
    const char* apiUrl;
    const char* dbUrl;
    bool wifiConnected;
    bool useDirectConnection;
    WiFiClient client;
    
    bool parseDatabaseUrl(const char* url, String& host, int& port, String& database, String& user, String& password);
    bool connectToDatabase(String host, int port, String database, String user, String password);
    String escapeString(String str);
    bool sendViaDirectConnection(String uid, String cardType, String location, String plate, String vehicle, String department);

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

