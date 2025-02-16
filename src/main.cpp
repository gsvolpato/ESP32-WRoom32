#include <TFT_eSPI.h>
#include <SPI.h>
#include "skull_logo.h"  // Include the header file for the logo

// Pin definitions
#define TFT_CS     17
#define TFT_RST    4
#define TFT_DC     16
#define TFT_SCLK   15
#define TFT_MOSI   2
#define TFT_BLK    5
#define BUTTON_PIN 0  // Assuming boot button is connected to GPIO 0

// Screen dimensions
#define SCREEN_WIDTH  160
#define SCREEN_HEIGHT 80

// Create display object
TFT_eSPI tft = TFT_eSPI();

// Define rainbow colors
#define TFT_INDIGO 0x4810  // Define a custom color for indigo
uint16_t rainbowColors[] = {TFT_WHITE, TFT_RED, TFT_ORANGE, TFT_YELLOW, TFT_GREEN, TFT_BLUE, TFT_INDIGO, TFT_VIOLET};
const char* colorNames[] = {"White", "Red", "Orange", "Yellow", "Green", "Blue", "Indigo", "Violet"};
int colorIndex = 0;

// Forward declaration of drawScreen
void drawScreen();

void setup() {
  Serial.begin(115200);  // Initialize serial communication
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  tft.init();
  tft.setRotation(3);  // Rotate display 270 degrees
  tft.fillScreen(TFT_BLACK);
  drawScreen();
}

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(200);  // Debounce delay
    colorIndex = (colorIndex + 1) % (sizeof(rainbowColors) / sizeof(rainbowColors[0]));
    drawScreen();
  }
  delay(200);
}

void drawScreen() {
  uint16_t currentColor = rainbowColors[colorIndex];

  tft.fillScreen(TFT_BLACK);

  // Draw the skull logo
  tft.drawBitmap((SCREEN_WIDTH - 160) / 2, (SCREEN_HEIGHT - 80) / 2, skull_logo, 160, 80, currentColor);
  
  // Print the current color name to serial
  Serial.print("Current color: ");
  Serial.println(colorNames[colorIndex]);
}