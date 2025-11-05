#ifndef RFID_HANDLER_H
#define RFID_HANDLER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PN532.h>
#include "GPIOS.h"

struct CardInfo {
    String uid;
    String type;
    String plate;
    String vehicle;
    String department;
    bool isValid;
};

class RFIDHandler {
private:
    Adafruit_PN532* nfc;
    String lastCardUID;
    unsigned long lastScanTime;
    unsigned long lastSuccessfulRead;
    unsigned long lastInitAttempt;
    bool initialized;
    uint8_t currentUID[7];
    uint8_t currentUIDLength;
    static const unsigned long SCAN_COOLDOWN = 2000;
    static const unsigned long I2C_HEARTBEAT_INTERVAL = 30000; // 30 seconds
    static const unsigned long I2C_RECOVERY_THRESHOLD = 60000; // 1 minute
    static const unsigned long RETRY_INIT_INTERVAL = 30000; // 30 seconds
    
    bool recoverI2C();

public:
    RFIDHandler();
    bool initialize();
    bool isInitialized();
    bool isNewCardPresent();
    CardInfo readCard();
    void readAllBlocks();
    String readBlockAsText(byte blockAddr);
    String getCardType(uint8_t cardType);
    void dumpByteArray(byte *buffer, byte bufferSize);
};

#endif 