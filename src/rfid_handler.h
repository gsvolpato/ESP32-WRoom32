#ifndef RFID_HANDLER_H
#define RFID_HANDLER_H

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include "GPIOS.h"

struct CardInfo {
    String uid;
    String type;
    bool isValid;
};

class RFIDHandler {
private:
    MFRC522* rfid;
    String lastCardUID;
    unsigned long lastScanTime;
    static const unsigned long SCAN_COOLDOWN = 2000;

public:
    RFIDHandler();
    bool initialize();
    bool isNewCardPresent();
    CardInfo readCard();
    void readAllBlocks();
    String getCardType(MFRC522::PICC_Type piccType);
    void dumpByteArray(byte *buffer, byte bufferSize);
};

#endif 