#include "TM1637Helper.h"

TM1637Helper::TM1637Helper(uint8_t clkPin, uint8_t dioPin)
    : _display(clkPin, dioPin) {
}

void TM1637Helper::begin(uint8_t brightness) {
    _display.setBrightness(brightness);
    _display.clear();
}

void TM1637Helper::writeNumber(int number, bool leadingZeros) {
    bool negative = number < 0;

    if (negative) {
        number = -number;

        // Clamp: max displayable with minus sign is -999
        if (number > 999) number = 999;

        // Build segments manually: [-, hundreds, tens, ones]
        uint8_t segments[4];
        segments[0] = SEG_G; // minus sign
        segments[1] = _display.encodeDigit(number / 100); // hundreds
        segments[2] = _display.encodeDigit((number / 10) % 10); // tens
        segments[3] = _display.encodeDigit(number % 10); // ones
        _display.setSegments(segments);
    } else {
        // Clamp to 4-digit max
        if (number > 9999) number = 9999;
        _display.showNumberDec(number, leadingZeros);
    }
}

void TM1637Helper::writeFloat(float number, uint8_t decimalPlaces) {
    // Clamp decimal places to valid range for a 4-digit display
    if (decimalPlaces > 3) decimalPlaces = 3;

    // Scale float to integer representation
    int multiplier = 1;
    for (uint8_t i = 0; i < decimalPlaces; i++) multiplier *= 10;

    int scaled = (int) round(number * multiplier);

    // Clamp to displayable range
    int maxVal = 9999;
    int minVal = -999;
    if (scaled > maxVal) scaled = maxVal;
    if (scaled < minVal) scaled = minVal;

    // Use showNumberDecEx to place the decimal point
    // Dot position bitmask: 0x08=pos0, 0x04=pos1, 0x02=pos2, 0x01=pos3
    uint8_t dotPos = 0;
    if (decimalPlaces == 1) dotPos = 0x02; // dot after digit 2 (X X . X)
    if (decimalPlaces == 2) dotPos = 0x04; // dot after digit 1 (X . X X)
    if (decimalPlaces == 3) dotPos = 0x08; // dot after digit 0 (X . X X X — leftmost)

    _display.showNumberDecEx(scaled, dotPos, false);
}

void TM1637Helper::clear() {
    _display.clear();
}

void TM1637Helper::setBrightness(uint8_t brightness) {
    _display.setBrightness(brightness);
}
