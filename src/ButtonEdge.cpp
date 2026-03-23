#include "ButtonEdge.h"

Button::Button(const uint8_t pin, const uint8_t mode, const uint16_t debounceMs)
    : _pin(pin), _mode(mode), _current(false), _last(false), _toggle(false),
      _debounceMs(debounceMs), _lastChangeMs(0) {
}

void Button::begin() {
    pinMode(_pin, _mode);
    _current = _read();
    _last = _current;
}

void Button::update() {
    const bool raw = _read();
    const uint32_t now = millis();

    if (raw != _current && now - _lastChangeMs >= _debounceMs) {
        _last = _current;
        _current = raw;
        _lastChangeMs = now;
    } else {
        _last = _current;
    }
}

bool Button::wasPressed() const { return _current && !_last; }
bool Button::wasReleased() const { return !_current && _last; }
bool Button::held() const { return _current; }
bool Button::rising() const { return wasPressed(); }
bool Button::falling() const { return wasReleased(); }

bool Button::toggle() {
    if (wasPressed()) _toggle = !_toggle;
    return _toggle;
}

bool Button::toggleState() const { return _toggle; }
void Button::resetToggle() { _toggle = false; }

bool Button::_read() const {
    const bool raw = digitalRead(_pin);
    return _mode == INPUT_PULLUP ? !raw : raw;
}
