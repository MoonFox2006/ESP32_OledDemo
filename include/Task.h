#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <freertos/semphr.h>
#include <freertos/portmacro.h>

class Task {
public:
  enum core_t { CORE_0 = 0, CORE_1, CORE_ANY };
  enum flags_t : uint8_t { FLAG_DESTROY = 0, FLAG_START, FLAG_USER };

  static const uint32_t FOREVER = (uint32_t)-1;

  Task(const char *name, uint32_t stack, uint8_t priority = 1, core_t core = CORE_ANY);
  virtual ~Task();

  operator bool() const {
    return _task != NULL;
  }

  bool start();
  void stop();
  bool isRunning();
  void pause();
  void resume();

  bool lock(uint32_t timeout = FOREVER);
  void unlock();

  static bool globalLock(uint32_t timeout = FOREVER);
  static void globalUnlock();

  void notify(uint32_t value);
  uint32_t getFlags();
  bool getFlag(uint8_t flag);
  void setFlag(uint8_t flag);
  void clearFlag(uint8_t flag);
  bool waitFlags(uint32_t flags, bool all = true, uint32_t timeout = FOREVER);

protected:
  static void run(void *self);

  bool waitNotify(uint32_t &value, uint32_t timeout = FOREVER);

  virtual bool init();
  virtual void deinit();
  virtual bool initLocal() {
    return true;
  }
  virtual void deinitLocal() {}
  virtual void loop() = 0;

  TaskHandle_t _task;
  EventGroupHandle_t _flags;
  SemaphoreHandle_t _mutex;

  const char *_name;
  uint32_t _stack : 24;
  uint8_t _priority : 4;
  core_t _core : 2;

  static SemaphoreHandle_t _globalMutex;
};
