#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_I2C_ADDRESS 0x3C
#define OLED_WIDTH  128
#define OLED_HEIGHT  64

class OledDisplay {
public:
    explicit OledDisplay(int8_t resetPin = -1);

    bool begin(uint8_t i2cAddr = OLED_I2C_ADDRESS);

    void show(const int value, const char *label) { show(static_cast<long>(value), label); }

    void show(long value, const char *label);

    void show(float value, const char *label, uint8_t decimals = 1);

    void showRaw(const char *bigText, const char *label);

    void setYellow(const char *text);

    void setDivider(int16_t y);

    void setBrightness(uint8_t level);

    void on();

    void off();

    Adafruit_SSD1306 &driver() { return _display; }

private:
    Adafruit_SSD1306 _display;
    char _yellowText[22]{};
    int16_t _dividerY;

    void _render(const char *bigText, const char *label);

    void _centerText(const char *text, int16_t y, uint8_t textSize);
};
