#ifndef WIFI_SETTINGS_H
#define WIFI_SETTINGS_H

#include <Arduino.h>
#include <WiFi.h>

class WiFiHandler {
private:
    const char* ssid;
    const char* password;
    bool wifiConnected;
    unsigned long lastConnectionAttempt;
    int connectionAttempts;
    static const unsigned long RECONNECT_INTERVAL = 30000;
    static const int MAX_CONNECTION_ATTEMPTS = 20;

    void printWiFiStatus();
    void printConnectionDetails();

public:
    WiFiHandler(const char* wifiSSID, const char* wifiPassword);
    
    bool connect();
    bool isConnected();
    void checkConnection();
    void reconnect();
    void disconnect();
    
    String getLocalIP();
    String getGatewayIP();
    String getSubnetMask();
    String getDNSIP();
    String getMACAddress();
    int getSignalStrength();
    
    void printNetworkInfo();
    void scanNetworks();
    
    wl_status_t getStatus();
    String getStatusString();
    String getStatusString(wl_status_t status);
};

#endif
