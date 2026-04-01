#include "OledDisplay.h"

static constexpr int16_t Y_YELLOW = 4;
static constexpr int16_t Y_BLUE_TOP = 16;
static constexpr int16_t DIVIDER_GAP = 2;

static constexpr int16_t DIVIDER_DEFAULT = 45;

OledDisplay::OledDisplay(const int8_t resetPin)
    : _display(OLED_WIDTH, OLED_HEIGHT, &Wire, resetPin)
      , _dividerY(DIVIDER_DEFAULT) {
    _yellowText[0] = '\0';
}

bool OledDisplay::begin(const uint8_t i2cAddr) {
    if (!_display.begin(SSD1306_SWITCHCAPVCC, i2cAddr))
        return false;
    _display.clearDisplay();
    _display.setTextColor(SSD1306_WHITE);
    _display.display();
    return true;
}

void OledDisplay::setDivider(const int16_t y) {
    _dividerY = constrain(y, Y_BLUE_TOP + 8 + DIVIDER_GAP, OLED_HEIGHT - 8 - 1);
}

void OledDisplay::show(const long value, const char *label) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%ld", value);
    _render(buf, label);
}

void OledDisplay::show(const float value, const char *label, const uint8_t decimals) {
    char fmt[8], buf[16];
    snprintf(fmt, sizeof(fmt), "%%.%uf", static_cast<unsigned>(decimals));
    snprintf(buf, sizeof(buf), fmt, static_cast<double>(value));
    _render(buf, label);
}

void OledDisplay::showRaw(const char *bigText, const char *label) {
    _render(bigText, label);
}

void OledDisplay::setYellow(const char *text) {
    if (text == nullptr) {
        _yellowText[0] = '\0';
    } else {
        strncpy(_yellowText, text, sizeof(_yellowText) - 1);
        _yellowText[sizeof(_yellowText) - 1] = '\0';
    }
    _display.fillRect(0, 0, OLED_WIDTH, 16, SSD1306_BLACK);
    if (_yellowText[0] != '\0') {
        _centerText(_yellowText, Y_YELLOW, 1);
    }
    _display.display();
}

void OledDisplay::setBrightness(const uint8_t level) {
    _display.ssd1306_command(SSD1306_SETCONTRAST);
    _display.ssd1306_command(static_cast<uint8_t>((level & 0x0F) * 17));
}

void OledDisplay::on() { _display.ssd1306_command(SSD1306_DISPLAYON); }

void OledDisplay::off() { _display.ssd1306_command(SSD1306_DISPLAYOFF); }

void OledDisplay::_centerText(const char *text, const int16_t y, const uint8_t textSize) {
    _display.setTextSize(textSize);
    int16_t x1, y1;
    uint16_t w, h;
    _display.getTextBounds(text, 0, y, &x1, &y1, &w, &h);
    int16_t x = (static_cast<int16_t>(OLED_WIDTH) - static_cast<int16_t>(w)) / 2;
    if (x < 0) x = 0;
    _display.setCursor(x, y);
    _display.print(text);
}

static uint8_t pickSize(Adafruit_SSD1306 &disp, const char *text, const int16_t startY, const int16_t maxWidth,
                        const int16_t maxHeight, uint16_t &outH) {
    for (uint8_t s = 4; s >= 1; s--) {
        int16_t x1, y1;
        uint16_t w, h;
        disp.setTextSize(s);
        disp.getTextBounds(text, 0, startY, &x1, &y1, &w, &h);
        if (static_cast<int16_t>(w) <= maxWidth && static_cast<int16_t>(h) <= maxHeight) {
            outH = h;
            return s;
        }
    }
    //fallback: size 1 even if it overflows
    int16_t x1, y1;
    uint16_t w, h;
    disp.setTextSize(1);
    disp.getTextBounds(text, 0, startY, &x1, &y1, &w, &h);
    outH = h;
    return 1;
}

void OledDisplay::_render(const char *bigText, const char *label) {
    _display.clearDisplay();

    if (_yellowText[0] != '\0') {
        _centerText(_yellowText, Y_YELLOW, 1);
    }

    const int16_t numberZoneH = _dividerY - DIVIDER_GAP - Y_BLUE_TOP;

    uint16_t numH = 0;
    const uint8_t numSize = pickSize(_display, bigText,
                                     Y_BLUE_TOP, OLED_WIDTH, numberZoneH, numH);

    int16_t numberY = Y_BLUE_TOP + (numberZoneH - static_cast<int16_t>(numH)) / 2;
    if (numberY < Y_BLUE_TOP) numberY = Y_BLUE_TOP;
    _centerText(bigText, numberY, numSize);

    _display.drawFastHLine(0, _dividerY, OLED_WIDTH, SSD1306_WHITE);

    const int16_t labelTop = _dividerY + 1;
    const int16_t labelZoneH = OLED_HEIGHT - labelTop; // e.g. 63 - dividerY

    uint16_t lblH = 0;
    const uint8_t lblSize = pickSize(_display, label,
                                     labelTop, OLED_WIDTH, labelZoneH, lblH);

    int16_t labelY = labelTop + (labelZoneH - static_cast<int16_t>(lblH)) / 2;
    if (labelY < labelTop) labelY = labelTop;
    _centerText(label, labelY, lblSize);

    _display.display();
}
