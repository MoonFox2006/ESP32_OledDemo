#include <esp_log.h>
#include "Task.h"

static const char TAG[] = "Task";

Task::Task(const char *name, uint32_t stack, uint8_t priority, core_t core) : _task(NULL), _flags(NULL), _mutex(NULL), _name(name),
  _stack(stack), _priority(priority), _core(core) {
  if (! _globalMutex) {
    _globalMutex = xSemaphoreCreateMutex();
    if (! _globalMutex) {
      ESP_LOGE(TAG, "Error creating RTOS global mutex!");
    }
  }
}

Task::~Task() {
  stop();
}

bool Task::start() {
  if (_task)
    return true;

  if (! init())
    return false;

  BaseType_t result;

  if (_core < CORE_ANY)
    result = xTaskCreatePinnedToCore(&Task::run, _name, _stack, this, _priority, &_task, _core);
  else
    result = xTaskCreate(&Task::run, _name, _stack, this, _priority, &_task);
  if (result != pdPASS) {
    _task = NULL;
    ESP_LOGE(TAG, "Error creating RTOS task!");
    return false;
  }
  if ((xEventGroupWaitBits(_flags, (1 << FLAG_DESTROY) | (1 << FLAG_START), pdFALSE, pdFALSE, portMAX_DELAY) & (1 << FLAG_DESTROY)) != 0) {
    deinit();
    return false;
  }
  return true;
}

void Task::stop() {
  if (_task) {
    if (_flags) {
      xEventGroupSetBits(_flags, 1 << FLAG_DESTROY);
    } else {
      vTaskDelete(_task);
    }
    _task = NULL;
    deinit();
  }
}

bool Task::isRunning() {
  if (_task) {
    return eTaskGetState(_task) == eRunning;
  }
  return false;
}

void Task::pause() {
  if (_task) {
    vTaskSuspend(_task);
  }
}

void Task::resume() {
  if (_task) {
    vTaskResume(_task);
  }
}

bool Task::lock(uint32_t timeout) {
  if (_task && _mutex) {
    return xSemaphoreTake(_mutex, timeout == FOREVER ? portMAX_DELAY : pdMS_TO_TICKS(timeout)) == pdTRUE;
  }
  return false;
}

void Task::unlock() {
  if (_task && _mutex) {
    xSemaphoreGive(_mutex);
  }
}

bool Task::globalLock(uint32_t timeout) {
  if (_globalMutex) {
    return xSemaphoreTake(_globalMutex, timeout == FOREVER ? portMAX_DELAY : pdMS_TO_TICKS(timeout)) == pdTRUE;
  }
  return false;
}

void Task::globalUnlock() {
  if (_globalMutex) {
    xSemaphoreGive(_globalMutex);
  }
}

void Task::notify(uint32_t value) {
  if (_task) {
    xTaskNotify(_task, value, eSetValueWithOverwrite);
  }
}

uint32_t Task::getFlags() {
  if (_task && _flags) {
    return xEventGroupGetBits(_flags);
  }
  return 0;
}

bool Task::getFlag(uint8_t flag) {
  if (_task && _flags) {
    return (xEventGroupGetBits(_flags) & (1 << flag)) != 0;
  }
  return false;
}

void Task::setFlag(uint8_t flag) {
  if (_task && _flags) {
    xEventGroupSetBits(_flags, (1 << flag));
  }
}

void Task::clearFlag(uint8_t flag) {
  if (_task && _flags) {
    xEventGroupClearBits(_flags, (1 << flag));
  }
}

bool Task::waitFlags(uint32_t flags, bool all, uint32_t timeout) {
  if (_task && _flags) {
    return (xEventGroupWaitBits(_flags, flags, pdFALSE, all ? pdTRUE : pdFALSE, timeout == FOREVER ? portMAX_DELAY : pdMS_TO_TICKS(timeout)) & flags) != 0;
  }
  return false;
}

void Task::run(void *self) {
  if (((Task*)self)->initLocal()) {
    xEventGroupSetBits(((Task*)self)->_flags, (1 << FLAG_START));
    while (! (xEventGroupWaitBits(((Task*)self)->_flags, (1 << FLAG_DESTROY), pdFALSE, pdTRUE, 0) & (1 << FLAG_DESTROY))) {
      ((Task*)self)->loop();
    }
    xEventGroupClearBits(((Task*)self)->_flags, (1 << FLAG_START));
    ((Task*)self)->deinitLocal();
  } else {
    xEventGroupSetBits(((Task*)self)->_flags, (1 << FLAG_DESTROY));
  }

  TaskHandle_t task = ((Task*)self)->_task;

  ((Task*)self)->_task = NULL;
  vTaskDelete(task);
}

bool Task::waitNotify(uint32_t &value, uint32_t timeout) {
  if (_task) {
    return xTaskNotifyWait(0, 0, &value, timeout == FOREVER ? portMAX_DELAY : pdMS_TO_TICKS(timeout)) == pdTRUE;
  }
  return false;
}

bool Task::init() {
  if (! _mutex) {
    _mutex = xSemaphoreCreateMutex();
    if (! _mutex) {
      ESP_LOGE(TAG, "Error creating RTOS mutex!");
      return false;
    }
  }
  if (! _flags) {
    _flags = xEventGroupCreate();
    if (! _flags) {
      vSemaphoreDelete(_mutex);
      _mutex = NULL;
      ESP_LOGE(TAG, "Error creating RTOS event group!");
      return false;
    }
  }
  return true;
}

void Task::deinit() {
  if (_flags) {
    vEventGroupDelete(_flags);
    _flags = NULL;
  }
  if (_mutex) {
    vSemaphoreDelete(_mutex);
    _mutex = NULL;
  }
}

SemaphoreHandle_t Task::_globalMutex = NULL;
