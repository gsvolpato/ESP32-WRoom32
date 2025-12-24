#ifndef WEB_APP_H
#define WEB_APP_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "rfid_handler.h"

class WebAppHandler {
private:
    WebServer* server;
    RFIDHandler* rfidHandler;
    int serverPort;
    String currentOperation;
    String lastCardData;
    String statusMessage;
    
    void handleRoot();
    void handleReadCard();
    void handleWriteCard();
    void handleFormatCard();
    void handleGetStatus();
    void handleWriteData();
    void handleNotFound();
    
    String generateHTML();
    String getStatusJSON();
    bool writeStringToBlock(int blockNumber, String data);
    String buildCardDataJSON(CardInfo cardInfo);
    bool performWriteOperation(String plate, String vehicle, String department);
    bool performFormatOperation();
    
public:
    WebAppHandler(int port = 80);
    void initialize(RFIDHandler* rfid);
    void handleClients();
    void setStatus(String message);
    String getServerIP();
    bool isRunning();
};

#endif
