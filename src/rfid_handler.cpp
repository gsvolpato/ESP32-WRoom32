#include "rfid_handler.h"

RFIDHandler::RFIDHandler() {
    Serial.println("🔄 Creating PN532 handler...");
    Serial.print("📌 Using I2C mode with SDA=");
    Serial.print(SDA_PIN);
    Serial.print(", SCL=");
    Serial.println(SCL_PIN);
    
    nfc = new Adafruit_PN532(SDA_PIN, SCL_PIN);
    lastCardUID = "";
    lastScanTime = 0;
    currentUIDLength = 0;
    memset(currentUID, 0, sizeof(currentUID));
    
    Serial.println("✅ PN532 handler created successfully");
}

bool RFIDHandler::initialize() {
    Serial.println("🔄 Starting PN532 initialization...");
    Serial.print("📌 I2C SDA Pin: ");
    Serial.println(SDA_PIN);
    Serial.print("📌 I2C SCL Pin: ");
    Serial.println(SCL_PIN);
    
    Serial.println("🔄 Initializing I2C communication...");
    nfc->begin();
    
    Serial.println("🔄 Attempting to get firmware version...");
    uint32_t versiondata = nfc->getFirmwareVersion();
    if (!versiondata) {
        Serial.println("❌ Warning: Didn't find PN53x board");
        Serial.println("❌ Possible causes:");
        Serial.println("   - I2C wiring incorrect (SDA/SCL swapped?)");
        Serial.println("   - PN532 not powered (check 3.3V/5V)");
        Serial.println("   - I2C address conflict");
        Serial.println("   - Faulty PN532 module");
        Serial.println("   - I2C pull-up resistors missing");
        return false;
    }
    
    Serial.print("✅ Found chip PN5"); 
    Serial.println((versiondata>>24) & 0xFF, HEX); 
    Serial.print("✅ Firmware ver. "); 
    Serial.print((versiondata>>16) & 0xFF, DEC); 
    Serial.print('.'); 
    Serial.println((versiondata>>8) & 0xFF, DEC);
    
    Serial.println("🔄 Configuring SAM (Security Access Module)...");
    nfc->SAMConfig();
    Serial.println("✅ PN532 NFC/RFID Reader initialized successfully.");
    return true;
}

bool RFIDHandler::isNewCardPresent() {
    uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
    uint8_t uidLength;
    
    return nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 100);
}

String RFIDHandler::readBlockAsText(byte blockAddr) {
    uint8_t data[16];
    uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    
    uint8_t success = nfc->mifareclassic_AuthenticateBlock(currentUID, currentUIDLength, (uint32_t)blockAddr, 0, keya);
    if (!success) {
        Serial.print(F("Auth failed for block "));
        Serial.println(blockAddr);
        return "";
    }
    
    success = nfc->mifareclassic_ReadDataBlock(blockAddr, data);
    if (!success) {
        Serial.print(F("Read failed for block "));
        Serial.println(blockAddr);
        return "";
    }
    
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

bool RFIDHandler::writeBlockAsText(byte blockAddr, String text) {
    uint8_t data[16];
    uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    
    uint8_t success = nfc->mifareclassic_AuthenticateBlock(currentUID, currentUIDLength, (uint32_t)blockAddr, 0, keya);
    if (!success) {
        Serial.print(F("Auth failed for block "));
        Serial.println(blockAddr);
        return false;
    }
    
    memset(data, 0, 16);
    
    if (text.length() > 0) {
        int len = (text.length() > 15) ? 15 : text.length();
        for (int i = 0; i < len; i++) {
            data[i] = text.charAt(i);
        }
    }
    
    success = nfc->mifareclassic_WriteDataBlock(blockAddr, data);
    if (!success) {
        Serial.print(F("Write failed for block "));
        Serial.println(blockAddr);
        return false;
    }
    
    Serial.print(F("Successfully wrote to block "));
    Serial.print(blockAddr);
    Serial.print(F(": '"));
    Serial.print(text);
    Serial.println(F("'"));
    
    return true;
}

CardInfo RFIDHandler::readCard() {
    CardInfo cardInfo;
    cardInfo.isValid = false;
    
    uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
    uint8_t uidLength;
    
    bool success = nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 100);
    if (!success) {
        return cardInfo;
    }
    
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