#pragma once
#include <Arduino.h>

class Button {
public:
  Button() : _pin(0), _mode(INPUT_PULLUP), _current(false), _last(false), _toggle(false), _debounceMs(20),
             _lastChangeMs(0) {
  }

  Button(uint8_t pin, uint8_t mode = INPUT_PULLUP, uint16_t debounceMs = 50);

  void begin();

  void update();

  bool rose() const;

  bool fell() const;

  bool held() const;

  bool rising() const;

  bool falling() const;

  bool toggle();

  bool toggleState() const;

  void resetToggle();

private:
  uint8_t _pin;
  uint8_t _mode;
  bool _current;
  bool _last;
  bool _toggle;
  uint16_t _debounceMs;
  uint32_t _lastChangeMs;

  bool _read() const;
};

template<uint8_t N>
class ButtonArray {
public:
  ButtonArray(const uint8_t pins[N], uint8_t mode = INPUT_PULLUP) {
    for (uint8_t i = 0; i < N; i++)
      _btns[i] = Button(pins[i], mode);
  }

  void begin() {
    for (uint8_t i = 0; i < N; i++) _btns[i].begin();
  }

  void update() {
    for (uint8_t i = 0; i < N; i++) _btns[i].update();
  }

  Button &operator[](uint8_t i) { return _btns[i]; }
  uint8_t size() const { return N; }

private:
  Button _btns[N];
};
