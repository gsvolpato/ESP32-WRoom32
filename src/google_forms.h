#define DISABLE_ALL_LIBRARY_WARNINGS
#ifndef GOOGLE_FORMS_H
#define GOOGLE_FORMS_H

#include <Arduino.h>
#include <GoogleFormPost.h>
#include "wifi_settings.h"

class GoogleFormsHandler {
private:
    WiFiHandler* wifiHandler;
    const char* formURL;
    const char* uidFieldName;
    const char* typeFieldName;
    const char* locationFieldName;
    const char* plateFieldName;
    const char* vehicleFieldName;
    const char* departmentFieldName;

public:
    GoogleFormsHandler(WiFiHandler* wifi, const char* googleFormURL, 
                      const char* uidField, const char* typeField, 
                      const char* locationField, const char* plateField, 
                      const char* vehicleField, const char* departmentField);
    
    bool connectToWiFi();
    bool isWiFiConnected();
    bool sendCardData(String uid, String cardType, String location, 
                     String plate, String vehicle, String department);
    void checkWiFiConnection();
    String getLocalIP();
};

#endif 