#include <Arduino.h>
#include "OledTask.h"

#define LED_PIN 25

static void halt(const char *msg) {
  Serial.println(msg);
  Serial.flush();
  esp_deep_sleep_start();
}

static void pwmDelay(uint32_t ms, uint32_t step = 10) {
  static int16_t pwm = 0;

  while (ms) {
    ledcWrite(0, abs(pwm));
    if (++pwm > 255)
      pwm = -254;
    if (ms >= step) {
      delay(step);
      ms -= step;
    } else {
      delay(ms);
      ms = 0;
    }
  }
}

OledTask *oledTask;

void setup() {
  Serial.begin(115200);
  Serial.println();

  ledcSetup(0, 1000, 8);
  ledcAttachPin(LED_PIN, 0);

  oledTask = new OledTask();
  if ((! oledTask) || (! oledTask->start()))
    halt("Error creating OLED task!");
}

void loop() {
  oledTask->setSymbols(OledTask::SYMBOL_WIFI | OledTask::SYMBOL_BLINK);
  pwmDelay(8000); // 8 sec.
  oledTask->setSymbols(OledTask::SYMBOL_WIFI, OledTask::SYMBOL_CLOUD | OledTask::SYMBOL_BLINK);
  pwmDelay(8000); // 8 sec.
  oledTask->setSymbols(OledTask::SYMBOL_WIFI, OledTask::SYMBOL_CLOUD);
  pwmDelay(12000); // 12 sec.
}
