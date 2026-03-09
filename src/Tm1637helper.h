#ifndef TM1637_HELPER_H
#define TM1637_HELPER_H

#include <Arduino.h>
#include <TM1637Display.h>

class TM1637Helper {
public:
    // Constructor: pass CLK and DIO pins
    TM1637Helper(uint8_t clkPin, uint8_t dioPin);

    // Initialize the display with optional brightness (0x00–0x0f)
    void begin(uint8_t brightness = 0x0f);

    // Write an integer to the display (-999 to 9999)
    void writeNumber(int number, bool leadingZeros = false);

    // Write a float, with specified decimal places (0–3)
    void writeFloat(float number, uint8_t decimalPlaces = 1);

    // Clear the display
    void clear();

    // Set brightness (0x00–0x0f)
    void setBrightness(uint8_t brightness);

private:
    TM1637Display _display;
};

#endif // TM1637_HELPER_H