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
        segments[1] = _display.encodeDigit(number / 100);
        segments[2] = _display.encodeDigit((number / 10) % 10);
        segments[3] = _display.encodeDigit(number % 10);
        _display.setSegments(segments);
    } else {
        // Clamp to 4-digit max
        if (number > 9999) number = 9999;
        _display.showNumberDec(number, leadingZeros);
    }
}

// Show a float using the colon as a separator instead of a decimal point.
// The colon sits between digit 1 and digit 2 (i.e. XX:XX).
// decimalPlaces must be 2 — the colon acts as the decimal point for ##.##
// For decimalPlaces == 1, the value is shown as  X:XXX (colon after first digit)
// For decimalPlaces == 3, value is shown as XXX:X
// Pass showColon=true to light the colon, false to hide it (plain integer layout)
void TM1637Helper::writeFloat(float number, uint8_t decimalPlaces) {
    if (decimalPlaces > 3) decimalPlaces = 3;

    // Scale to integer
    int multiplier = 1;
    for (uint8_t i = 0; i < decimalPlaces; i++) multiplier *= 10;

    int scaled = (int) round(number * multiplier);

    bool negative = scaled < 0;
    if (negative) scaled = -scaled;

    // Clamp
    if (scaled > 9999) scaled = 9999;

    int d0 = (scaled / 1000) % 10;
    int d1 = (scaled / 100)  % 10;
    int d2 = (scaled / 10)   % 10;
    int d3 = (scaled / 1)    % 10;

    uint8_t segments[4];

    if (negative) {
        // Show as  -X:XX  (minus, one digit, colon, two digits) for decimalPlaces==2
        // Best effort for other decimal places
        segments[0] = SEG_G; // minus
        segments[1] = _display.encodeDigit(d1);
        segments[2] = _display.encodeDigit(d2);
        segments[3] = _display.encodeDigit(d3);
    } else {
        segments[0] = _display.encodeDigit(d0);
        segments[1] = _display.encodeDigit(d1);
        segments[2] = _display.encodeDigit(d2);
        segments[3] = _display.encodeDigit(d3);
    }

    // The colon on TM1637 4-digit displays is toggled via the dot bit
    // on segment index 1 (second digit). 0x80 on that segment lights the colon.
    // decimalPlaces==2 → colon sits at XX:XX which matches the physical colon position
    bool showColon = (decimalPlaces == 2);
    if (showColon) {
        segments[1] |= 0x80; // enable colon
    }

    _display.setSegments(segments);
}

// Convenience: show time as HH:MM with colon always on
void TM1637Helper::writeTime(uint8_t hours, uint8_t minutes) {
    if (hours   > 99) hours   = 99;
    if (minutes > 59) minutes = 59;

    uint8_t segments[4];
    segments[0] = _display.encodeDigit(hours / 10);
    segments[1] = _display.encodeDigit(hours % 10) | 0x80; // colon ON
    segments[2] = _display.encodeDigit(minutes / 10);
    segments[3] = _display.encodeDigit(minutes % 10);
    _display.setSegments(segments);
}

void TM1637Helper::clear() {
    _display.clear();
}

void TM1637Helper::setBrightness(uint8_t brightness) {
    _display.setBrightness(brightness);
}