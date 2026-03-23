#include "TM1637Helper.h"

TM1637Helper::TM1637Helper(const uint8_t clkPin, const uint8_t dioPin)
    : _display(clkPin, dioPin) {
}

void TM1637Helper::begin(const uint8_t brightness) {
    _display.setBrightness(brightness);
    _display.clear();
}

void TM1637Helper::writeNumber(int number, const bool leadingZeros) {
    const bool negative = number < 0;

    if (negative) {
        number = -number;

        if (number > 999) number = 999;

        uint8_t segments[4];
        segments[0] = SEG_G;
        segments[1] = _display.encodeDigit(number / 100);
        segments[2] = _display.encodeDigit(number / 10 % 10);
        segments[3] = _display.encodeDigit(number % 10);
        _display.setSegments(segments);
    } else {
        if (number > 9999) number = 9999;
        _display.showNumberDec(number, leadingZeros);
    }
}


void TM1637Helper::writeFloat(const float number, uint8_t decimalPlaces) {
    if (decimalPlaces > 2) decimalPlaces = 2;
    if (decimalPlaces == 1) decimalPlaces = 2;

    int multiplier = 1;
    for (uint8_t i = 0; i < decimalPlaces; i++) multiplier *= 10;

    int scaled = static_cast<int>(round(number * multiplier));

    const bool negative = scaled < 0;
    if (negative) scaled = -scaled;

    if (scaled > 9999) scaled = 9999;

    const int d0 = scaled / 1000 % 10;
    const int d1 = scaled / 100 % 10;
    const int d2 = scaled / 10 % 10;
    const int d3 = scaled / 1 % 10;

    uint8_t segments[4];

    if (negative) {
        segments[0] = SEG_G;
        segments[1] = _display.encodeDigit(d1);
        segments[2] = _display.encodeDigit(d2);
        segments[3] = _display.encodeDigit(d3);
    } else {
        segments[0] = _display.encodeDigit(d0);
        segments[1] = _display.encodeDigit(d1);
        segments[2] = _display.encodeDigit(d2);
        segments[3] = _display.encodeDigit(d3);
    }

    const bool showColon = decimalPlaces == 2;
    if (showColon) {
        segments[1] |= 0x80;
    }

    _display.setSegments(segments);
}

void TM1637Helper::writeTime(uint8_t hours, uint8_t minutes) {
    if (hours > 99) hours = 99;
    if (minutes > 59) minutes = 59;

    uint8_t segments[4];
    segments[0] = _display.encodeDigit(hours / 10);
    segments[1] = _display.encodeDigit(hours % 10) | 0x80;
    segments[2] = _display.encodeDigit(minutes / 10);
    segments[3] = _display.encodeDigit(minutes % 10);
    _display.setSegments(segments);
}

void TM1637Helper::clear() {
    _display.clear();
}

void TM1637Helper::setBrightness(const uint8_t brightness) {
    _display.setBrightness(brightness);
}
