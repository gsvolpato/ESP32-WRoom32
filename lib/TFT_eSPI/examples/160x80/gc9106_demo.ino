#include <TFT_eSPI.h>
TFT_eSPI tft;

// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define GRAY    0x8410

//#define TFT_INVERSION_ON

void setup()
{
    Serial.begin(115200);
    Serial.println("Start");
    tft.begin();
}

void loop()
{
    static uint8_t aspect = 0;
    const char *aspectname[] = {
        "PORTRAIT", "LANDSCAPE", "PORTRAIT_REV", "LANDSCAPE_REV"
    };
    const char *colorname[] = { "BLUE", "GREEN", "RED", "GRAY" };
    uint16_t colormask[] = { BLUE, GREEN, RED, GRAY };
    //uint16_t ID = tft.readID(); // MCUFRIEND_kbv style
    uint16_t ID = 0x7735;
    tft.setRotation(aspect);
    int width = tft.width();
    int height = tft.height();
    int sz = (width <= 130 || height <= 130) ? 1 : 2;
    int mgn = (sz < 2) ? 8 : 32;
    int x = mgn + 4 * sz, y = x;
    tft.fillScreen(colormask[aspect]);
    tft.drawRect(0, 0, width, height, WHITE);
    tft.drawRect(mgn, mgn, width - mgn * 2, height - mgn * 2, WHITE);
    tft.setTextSize(sz);
    tft.setTextColor(BLACK);
    tft.setCursor(x, y);
    tft.print("ID=0x");
    tft.print(ID, HEX);
    if (ID == 0xD3D3) tft.print(" w/o");
    tft.setCursor(x, y += 10 * sz);
    tft.print(aspectname[aspect]);
    tft.setCursor(x, y += 10 * sz);
    tft.print(width);
    tft.print(" x ");
    tft.print(height);
    tft.setTextColor(WHITE);
    tft.setCursor(x, y += 10 * sz);
    tft.setTextSize(sz * 2);
    tft.print(colorname[aspect]);
    if (++aspect > 3) aspect = 0;
    Serial.print("LOOP");
    delay(5000);
}

