#pragma once

#include <u8g2lib.h>
#include "Task.h"

class OledTask : public Task {
public:
  enum brightness_t : uint8_t { BRIGHT_OFF, BRIGHT_DIM, BRIGHT_NORMAL };
  enum symbol_t : uint8_t { SYMBOL_NONE = 0, SYMBOL_OK, SYMBOL_FAIL, SYMBOL_PROGRESS, SYMBOL_HARDWARE, SYMBOL_WIFI, SYMBOL_CLOCK, SYMBOL_CLOUD, SYMBOL_SLEEP, SYMBOL_BLINK = 0x80 };

  OledTask();

  void sleep();
  void wakeup();
  void setBrightness(brightness_t value);
  void setSymbols(uint8_t firstSymbol = SYMBOL_NONE, uint8_t secondSymbol = SYMBOL_NONE, uint8_t thirdSymbol = SYMBOL_NONE, uint8_t fourthSymbol = SYMBOL_NONE);

protected:
  static const uint8_t CONTRAST_DIM = 7;
  static const uint8_t CONTRAST_NORMAL = 31;

  static const uint32_t DIM_TIMEOUT = 5000; // 5 sec.
  static const uint32_t OFF_TIMEOUT = 5000; // 5 sec.

  bool initLocal();
  void deinitLocal();
  void loop();

  U8G2 *_u8g2;
  uint32_t _lastTime;
  uint32_t _symbols;
  uint8_t _frames;
  brightness_t _bright;
};
