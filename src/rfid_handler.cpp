#include "rfid_handler.h"

RFIDHandler::RFIDHandler() {
    rfid = new MFRC522(SS_PIN, RST_PIN);
    lastCardUID = "";
    lastScanTime = 0;
}

bool RFIDHandler::initialize() {
    SPI.begin(PIN_RF_SCK, PIN_RF_MISO, PIN_RF_MOSI, SS_PIN);
    rfid->PCD_Init();
    delay(10);
    
    byte v = rfid->PCD_ReadRegister(rfid->VersionReg);
    Serial.print(F("MFRC522 Software Version: 0x"));
    Serial.println(v, HEX);
    
    if (v == 0x00 || v == 0xFF) {
        Serial.println(F("Warning: Communication failure, is the MFRC522 properly connected?"));
        return false;
    } else {
        rfid->PCD_SetAntennaGain(rfid->RxGain_max);
        Serial.println(F("RFID Reader initialized successfully."));
        return true;
    }
}

bool RFIDHandler::isNewCardPresent() {
    return rfid->PICC_IsNewCardPresent() && rfid->PICC_ReadCardSerial();
}

CardInfo RFIDHandler::readCard() {
    CardInfo cardInfo;
    cardInfo.isValid = false;
    
    if (!isNewCardPresent()) {
        return cardInfo;
    }
    
    String uid = "";
    for (byte i = 0; i < rfid->uid.size; i++) {
        uid += (rfid->uid.uidByte[i] < 0x10 ? "0" : "");
        uid += String(rfid->uid.uidByte[i], HEX);
        if (i < rfid->uid.size - 1) {
            uid += ":";
        }
    }
    uid.toUpperCase();
    
    if (uid == lastCardUID && (millis() - lastScanTime < SCAN_COOLDOWN)) {
        return cardInfo;
    }
    
    lastCardUID = uid;
    lastScanTime = millis();
    
    cardInfo.uid = uid;
    cardInfo.type = getCardType(rfid->PICC_GetType(rfid->uid.sak));
    cardInfo.isValid = true;
    
    return cardInfo;
}

void RFIDHandler::readAllBlocks() {
    MFRC522::MIFARE_Key key;
    for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = 0xFF;
    }

    Serial.println(F("\n===================="));
    Serial.println(F("Card Detected:"));
    Serial.print(F("Card UID: "));
    for (byte i = 0; i < rfid->uid.size; i++) {
        Serial.print(rfid->uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(rfid->uid.uidByte[i], HEX);
    }
    Serial.println();
    
    MFRC522::PICC_Type piccType = rfid->PICC_GetType(rfid->uid.sak);
    Serial.print(F("PICC Type: "));
    Serial.println(rfid->PICC_GetTypeName(piccType));
    Serial.print(F("SAK: 0x"));
    Serial.println(rfid->uid.sak, HEX);
    Serial.println();

    if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
        piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
        piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
        Serial.println(F("This sample only works with MIFARE Classic cards."));
        Serial.println(F("====================\n"));
        return;
    }

    Serial.println(F("Reading all accessible sectors..."));
    
    byte numSectors = 16;
    if (piccType == MFRC522::PICC_TYPE_MIFARE_4K) {
        numSectors = 40;
    }

    for (byte sector = 0; sector < numSectors; sector++) {
        byte trailerBlock = (sector < 32) ? (sector * 4 + 3) : (32 * 4 + (sector - 32) * 16 + 15);
        
        MFRC522::StatusCode status = rfid->PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(rfid->uid));
        if (status != MFRC522::STATUS_OK) {
            Serial.print(F("Sector "));
            Serial.print(sector);
            Serial.print(F(" - Auth failed: "));
            Serial.println(rfid->GetStatusCodeName(status));
            continue;
        }

        Serial.print(F("Sector "));
        Serial.print(sector);
        Serial.println(F(":"));

        byte firstBlock = (sector < 32) ? (sector * 4) : (32 * 4 + (sector - 32) * 16);
        byte lastBlock = (sector < 32) ? (sector * 4 + 2) : (32 * 4 + (sector - 32) * 16 + 14);

        for (byte blockAddr = firstBlock; blockAddr <= lastBlock; blockAddr++) {
            byte buffer[18];
            byte size = sizeof(buffer);
            
            status = rfid->MIFARE_Read(blockAddr, buffer, &size);
            if (status != MFRC522::STATUS_OK) {
                Serial.print(F("  Block "));
                Serial.print(blockAddr);
                Serial.print(F(" read failed: "));
                Serial.println(rfid->GetStatusCodeName(status));
                continue;
            }

            Serial.print(F("  Block "));
            Serial.print(blockAddr);
            Serial.print(F(": "));
            
            for (byte i = 0; i < 16; i++) {
                Serial.print(buffer[i] < 0x10 ? " 0" : " ");
                Serial.print(buffer[i], HEX);
            }
            
            Serial.print(F(" | "));
            for (byte i = 0; i < 16; i++) {
                char c = buffer[i];
                Serial.print((c >= 32 && c <= 126) ? c : '.');
            }
            Serial.println();
        }

        Serial.print(F("  Trailer Block "));
        Serial.print(trailerBlock);
        Serial.print(F(": "));
        byte buffer[18];
        byte size = sizeof(buffer);
        status = rfid->MIFARE_Read(trailerBlock, buffer, &size);
        if (status == MFRC522::STATUS_OK) {
            for (byte i = 0; i < 16; i++) {
                Serial.print(buffer[i] < 0x10 ? " 0" : " ");
                Serial.print(buffer[i], HEX);
            }
            Serial.println(F(" [ACCESS BITS]"));
        } else {
            Serial.println(F("Failed to read trailer"));
        }
        Serial.println();
    }

    Serial.println(F("====================\n"));
    rfid->PICC_HaltA();
    rfid->PCD_StopCrypto1();
}

String RFIDHandler::getCardType(MFRC522::PICC_Type piccType) {
    switch (piccType) {
        case MFRC522::PICC_TYPE_MIFARE_MINI:  return "MIFARE Mini";
        case MFRC522::PICC_TYPE_MIFARE_1K:    return "MIFARE 1K";
        case MFRC522::PICC_TYPE_MIFARE_4K:    return "MIFARE 4K";
        case MFRC522::PICC_TYPE_MIFARE_UL:    return "MIFARE Ultralight";
        case MFRC522::PICC_TYPE_ISO_14443_4:  return "ISO 14443-4";
        case MFRC522::PICC_TYPE_ISO_18092:    return "ISO 18092";
        default:                               return "Unknown";
    }
}

void RFIDHandler::dumpByteArray(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
} 