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
    uint8_t currentUID[7];
    uint8_t currentUIDLength;
    static const unsigned long SCAN_COOLDOWN = 2000;

public:
    RFIDHandler();
    bool initialize();
    bool isNewCardPresent();
    CardInfo readCard();
    void readAllBlocks();
    String readBlockAsText(byte blockAddr);
    bool writeBlockAsText(byte blockAddr, String text);
    String getCardType(uint8_t cardType);
    void dumpByteArray(byte *buffer, byte bufferSize);
};

#endif 