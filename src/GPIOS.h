#define DISABLE_ALL_LIBRARY_WARNINGS
#ifndef GPIOS_H
#define GPIOS_H

//| RF Modules SPI Pins for ESP32 WROOM 32
#define PIN_RF_MISO 26  // MISO pin for RFID
#define PIN_RF_MOSI 27  // MOSI pin for RFID
#define PIN_RF_SCK 14   // SCK pin for RFID

//| RC522 RFID Module Pins
#define RST_PIN 32      // RC522 RST pin
#define SS_PIN 12       // RC522 SDA (SS) pin
#define IRQ_PIN 25      // RC522 IRQ pin

#endif // GPIOS_H