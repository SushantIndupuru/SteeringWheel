#pragma once

#include <Arduino.h>
#include <TM1637Display.h>

class TM1637Helper {
public:
    //pass CLK and DIO pins
    TM1637Helper(uint8_t clkPin, uint8_t dioPin);

    //init the display with optional brightness (0x00–0x0f)
    void begin(uint8_t brightness = 0x0f);

    //write an integer to the display (-999 to 9999)
    void writeNumber(int number, bool leadingZeros = false);

    //write a float, with specified decimal places (0 or 2)
    void writeFloat(float number, uint8_t decimalPlaces = 2);

    //write hours and minutes in HH:MM format
    void writeTime(uint8_t hours, uint8_t minutes);

    //clear the display
    void clear();

    //set brightness (0x00–0x0f)
    void setBrightness(uint8_t brightness);


    void writeRaw(const uint8_t segments[4]);
private:
    TM1637Display _display;
};