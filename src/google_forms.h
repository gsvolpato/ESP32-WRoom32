#define DISABLE_ALL_LIBRARY_WARNINGS
#ifndef GOOGLE_FORMS_H
#define GOOGLE_FORMS_H

#include <Arduino.h>
#include <WiFi.h>
#include <GoogleFormPost.h>

class GoogleFormsHandler {
private:
    const char* ssid;
    const char* password;
    const char* formURL;
    const char* uidFieldName;
    const char* typeFieldName;
    const char* userFieldName;
    bool wifiConnected;

public:
    GoogleFormsHandler(const char* wifiSSID, const char* wifiPassword, 
                      const char* googleFormURL, const char* uidField, 
                      const char* typeField, const char* userField);
    
    bool connectToWiFi();
    bool isWiFiConnected();
    bool sendCardData(String uid, String cardType);
    void checkWiFiConnection();
    String getLocalIP();
};

#endif 