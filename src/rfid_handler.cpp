#include "rfid_handler.h"

RFIDHandler::RFIDHandler() {
    nfc = new Adafruit_PN532(SDA_PIN, SCL_PIN);
    lastCardUID = "";
    lastScanTime = 0;
    lastSuccessfulRead = 0;
    lastInitAttempt = 0;
    initialized = false;
    currentUIDLength = 0;
    memset(currentUID, 0, sizeof(currentUID));
}

bool RFIDHandler::initialize() {
    delay(500); // Longer initial delay to let everything settle
    
    // Reset I2C bus first
    Wire.end();
    delay(200);
    
    // Initialize I2C bus with longer delays
    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(100000); // Slow down to 100kHz for reliability
    delay(300);
    
    // Try to initialize PN532 with more attempts and longer delays
    uint32_t versiondata = 0;
    for (int attempt = 0; attempt < 5; attempt++) {
        Serial.print("Attempt ");
        Serial.print(attempt + 1);
        Serial.print("/5... ");
        
        nfc->begin();
        delay(500); // Longer delay after begin
        
        // Try getFirmwareVersion with timeout protection
        unsigned long startTime = millis();
        versiondata = nfc->getFirmwareVersion();
        
        if (versiondata) {
            Serial.println("OK");
            break;
        }
        
        Serial.println("failed");
        delay(500); // Longer delay between attempts
    }
    
    if (!versiondata) {
        Serial.println("PN532 not detected. Scanning I2C bus...");
        delay(200);
        
        // Simple I2C scan if PN532 not found
        Serial.print("I2C scan: ");
        int found = 0;
        for (byte addr = 1; addr < 127; addr++) {
            Wire.beginTransmission(addr);
            byte error = Wire.endTransmission();
            if (error == 0) {
                Serial.print("0x");
                if (addr < 16) Serial.print("0");
                Serial.print(addr, HEX);
                Serial.print(" ");
                found++;
            }
            delay(5);
        }
        if (found == 0) {
            Serial.println("no devices");
        } else {
            Serial.println();
        }
        return false;
    }
    
    Serial.print("PN532 found! Firmware: ");
    Serial.print((versiondata>>16) & 0xFF, DEC);
    Serial.print(".");
    Serial.println((versiondata>>8) & 0xFF, DEC);
    
    delay(200);
    nfc->SAMConfig();
    delay(100);
    lastSuccessfulRead = millis();
    initialized = true;
    return true;
}

bool RFIDHandler::recoverI2C() {
    Serial.println("I2C recovery: Re-initializing bus and PN532...");
    
    // Reset I2C bus with longer delays
    Wire.end();
    delay(200);
    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(100000);
    delay(200);
    
    // Try recovery multiple times
    uint32_t versiondata = 0;
    for (int i = 0; i < 3; i++) {
        nfc->begin();
        delay(500);
        
        versiondata = nfc->getFirmwareVersion();
        if (versiondata) {
            break;
        }
        delay(300);
    }
    
    if (!versiondata) {
        Serial.println("I2C recovery: Failed to communicate with PN532");
        return false;
    }
    
    nfc->SAMConfig();
    delay(100);
    lastSuccessfulRead = millis();
    Serial.println("I2C recovery: Success!");
    return true;
}

bool RFIDHandler::isInitialized() {
    return initialized;
}

bool RFIDHandler::isNewCardPresent() {
    if (!nfc || !initialized) {
        return false;
    }
    
    uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
    uint8_t uidLength;
    
    return nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 100);
}

String RFIDHandler::readBlockAsText(byte blockAddr) {
    if (!nfc || !initialized || currentUIDLength == 0) {
        return "";
    }
    
    uint8_t data[16];
    uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    
    uint8_t success = nfc->mifareclassic_AuthenticateBlock(currentUID, currentUIDLength, (uint32_t)blockAddr, 0, keya);
    if (!success) {
        // If auth fails and it's been a while, try recovery
        if ((millis() - lastSuccessfulRead) > 5000) {
            recoverI2C();
        }
        return "";
    }
    
    success = nfc->mifareclassic_ReadDataBlock(blockAddr, data);
    if (!success) {
        return "";
    }
    
    // Update successful read timestamp
    lastSuccessfulRead = millis();
    
    String text = "";
    for (byte i = 0; i < 16; i++) {
        if (data[i] != 0x00 && data[i] >= 32 && data[i] <= 126) {
            text += (char)data[i];
        } else if (data[i] == 0x00) {
            break;
        }
    }
    
    text.trim();
    return text;
}

CardInfo RFIDHandler::readCard() {
    CardInfo cardInfo;
    cardInfo.isValid = false;
    
    if (!nfc) {
        return cardInfo;
    }
    
    // If not initialized, try to initialize periodically
    if (!initialized) {
        unsigned long timeSinceLastAttempt = millis() - lastInitAttempt;
        if (timeSinceLastAttempt > RETRY_INIT_INTERVAL) {
            lastInitAttempt = millis();
            Serial.println("Retrying PN532 initialization...");
            if (initialize()) {
                Serial.println("PN532 initialized successfully!");
            }
        }
        return cardInfo;
    }
    
    // Check if I2C needs heartbeat (no activity for too long)
    unsigned long timeSinceLastSuccess = millis() - lastSuccessfulRead;
    if (timeSinceLastSuccess > I2C_HEARTBEAT_INTERVAL) {
        // Try a simple read to wake up I2C
        uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
        uint8_t uidLength;
        nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 50);
        // Don't care about result, just wake up the bus
    }
    
    // If I2C has been down for too long, try recovery
    if (timeSinceLastSuccess > I2C_RECOVERY_THRESHOLD) {
        if (!recoverI2C()) {
            return cardInfo; // Recovery failed
        }
    }
    
    uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
    uint8_t uidLength;
    
    bool success = nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 100);
    
    // If read fails, try recovery once
    if (!success) {
        // Check if error is likely I2C related (not just no card)
        if (timeSinceLastSuccess > 5000) { // No successful read in 5 seconds
            recoverI2C();
            // Try once more
            success = nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 100);
        }
    }
    
    if (!success || uidLength == 0) {
        return cardInfo;
    }
    
    // Successful read - update timestamp
    lastSuccessfulRead = millis();
    
    memcpy(currentUID, uid, uidLength);
    currentUIDLength = uidLength;
    
    String uidString = "";
    for (uint8_t i = 0; i < uidLength; i++) {
        uidString += (uid[i] < 0x10 ? "0" : "");
        uidString += String(uid[i], HEX);
        if (i < uidLength - 1) {
            uidString += ":";
        }
    }
    uidString.toUpperCase();
    
    if (uidString == lastCardUID && (millis() - lastScanTime < SCAN_COOLDOWN)) {
        return cardInfo;
    }
    
    lastCardUID = uidString;
    lastScanTime = millis();
    
    cardInfo.uid = uidString;
    cardInfo.type = getCardType(uidLength);
    
    Serial.println(F("\n=== Reading Card Data ==="));
    Serial.print(F("UID: "));
    Serial.println(cardInfo.uid);
    Serial.print(F("Type: "));
    Serial.println(cardInfo.type);

    cardInfo.plate = readBlockAsText(4);
    Serial.print(F("Plate (Block 4): '"));
    Serial.print(cardInfo.plate);
    Serial.println(F("'"));

    cardInfo.vehicle = readBlockAsText(5);
    Serial.print(F("Vehicle (Block 5): '"));
    Serial.print(cardInfo.vehicle);
    Serial.println(F("'"));

    cardInfo.department = readBlockAsText(6);
    Serial.print(F("Department (Block 6): '"));
    Serial.print(cardInfo.department);
    Serial.println(F("'"));

    Serial.println(F("=========================\n"));
    
    cardInfo.isValid = true;
    
    return cardInfo;
}

void RFIDHandler::readAllBlocks() {
    uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

    Serial.println(F("\n===================="));
    Serial.println(F("Reading all accessible sectors..."));
    
    for (uint8_t sector = 0; sector < 16; sector++) {
        uint8_t firstBlock = sector * 4;
        uint8_t trailerBlock = sector * 4 + 3;
        
        uint8_t success = nfc->mifareclassic_AuthenticateBlock(currentUID, currentUIDLength, (uint32_t)trailerBlock, 0, keya);
        if (!success) {
            Serial.print(F("Sector "));
            Serial.print(sector);
            Serial.println(F(" - Auth failed"));
            continue;
        }

        Serial.print(F("Sector "));
        Serial.print(sector);
        Serial.println(F(":"));

        for (uint8_t blockAddr = firstBlock; blockAddr < trailerBlock; blockAddr++) {
            uint8_t data[16];
            success = nfc->mifareclassic_ReadDataBlock(blockAddr, data);
            if (!success) {
                Serial.print(F("  Block "));
                Serial.print(blockAddr);
                Serial.println(F(" read failed"));
                continue;
            }

            Serial.print(F("  Block "));
            Serial.print(blockAddr);
            Serial.print(F(": "));
            
            for (uint8_t i = 0; i < 16; i++) {
                Serial.print(data[i] < 0x10 ? " 0" : " ");
                Serial.print(data[i], HEX);
            }
            
            Serial.print(F(" | "));
            for (uint8_t i = 0; i < 16; i++) {
                char c = data[i];
                Serial.print((c >= 32 && c <= 126) ? c : '.');
            }
            Serial.println();
        }

        Serial.print(F("  Trailer Block "));
        Serial.print(trailerBlock);
        Serial.print(F(": "));
        uint8_t data[16];
        success = nfc->mifareclassic_ReadDataBlock(trailerBlock, data);
        if (success) {
            for (uint8_t i = 0; i < 16; i++) {
                Serial.print(data[i] < 0x10 ? " 0" : " ");
                Serial.print(data[i], HEX);
            }
            Serial.println(F(" [ACCESS BITS]"));
        } else {
            Serial.println(F("Failed to read trailer"));
        }
        Serial.println();
    }

    Serial.println(F("====================\n"));
}

String RFIDHandler::getCardType(uint8_t uidLength) {
    switch (uidLength) {
        case 4:  return "MIFARE Classic 1K";
        case 7:  return "MIFARE Classic 4K";
        case 10: return "MIFARE DESFire";
        default: return "Unknown NFC/RFID";
    }
}

void RFIDHandler::dumpByteArray(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
} 