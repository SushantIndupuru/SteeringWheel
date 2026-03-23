#pragma once
#include <Arduino.h>
#include <new>

class Button {
public:
  Button() = delete;

  explicit Button(uint8_t pin, uint8_t mode = INPUT_PULLUP, uint16_t debounceMs = 50);

  void begin();

  void update();

  bool wasPressed() const;

  bool wasReleased() const;

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
  uint64_t _currentMs{};

  bool _read() const;
};

template<uint8_t N>
class ButtonArray {
public:
  explicit ButtonArray(const uint8_t pins[N], uint8_t mode = INPUT_PULLUP) {
    for (uint8_t i = 0; i < N; i++) {
      new(&_storage[i]) Button(pins[i], mode);
    }
  }

  void begin() {
    for (uint8_t i = 0; i < N; i++) btn(i).begin();
  }

  void update() {
    for (uint8_t i = 0; i < N; i++) btn(i).update();
  }

  Button &operator[](const uint8_t i) { return btn(i); }
  static uint8_t size() { return N; }

private:
  alignas(Button) uint8_t _storage[N][sizeof(Button)]{};

  Button &btn(uint8_t i) {
    return *reinterpret_cast<Button *>(&_storage[i]);
  }
};
