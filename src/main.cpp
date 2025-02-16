#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <FS.h>
#include <SPIFFS.h>
#include <chameleonUltra.h>

// MFRC522 Pin configuration for ESP32
#define RST_PIN     25    // Reset pin
#define SS_PIN      26    // SDA (SS) pin
#define SCK_PIN     13    // SCK pin
#define MOSI_PIN    12    // MOSI pin 
#define MISO_PIN    14    // MISO pin
#define IRQ_PIN     27    // IRQ pin

// Initialize MFRC522
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Initialize ChameleonUltra
ChameleonUltra chmUltra = ChameleonUltra(true);

void setup() {
  Serial.begin(115200);
  while (!Serial);    // Do nothing if no serial port is opened
  
  // Initialize SPI
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  
  // Initialize MFRC522
  mfrc522.PCD_Init();
  delay(4);        // Optional delay
  mfrc522.PCD_DumpVersionToSerial();  // Show version of PCD - MFRC522 Card Reader
  
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));

  // Initialize filesystem
  if(!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
}

void loop() {
  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Show UID on serial monitor
  Serial.print(F("Card UID:"));
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();

  // Show card type
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.print(F("PICC type: "));
  Serial.println(mfrc522.PICC_GetTypeName(piccType));

  // Dump debug info about the card
  mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid));

  // Try to connect to Chameleon Ultra
  if (chmUltra.connect()) {
    Serial.println("Connected to Chameleon Ultra");
    
    // Read card data and send to Chameleon
    byte buffer[18];
    byte size = sizeof(buffer);
    
    // Get card data
    mfrc522.MIFARE_Read(4, buffer, &size);
    
    // Send to Chameleon (example - you'll need to adjust based on your needs)
    // chmUltra.writeBlock(4, buffer, size);
    
    chmUltra.disconnect();
  }

  // Halt PICC and stop encryption
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  delay(1000);
}