#ifndef TASKS_H
#define TASKS_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "../../utils/mqtt.h"
#include "../../utils/wifi.h"
#include "audio.h"
#include "fan.h"
#include "storage_nvs.h"

extern TaskHandle_t currentHandle;
extern TaskHandle_t countTaskHandle;
extern TaskHandle_t heartbeatHandle;
extern TaskHandle_t eventProcessorHandle;
extern SemaphoreHandle_t semaphoreDutyCycle;
extern SemaphoreHandle_t semaphoreAudio;
extern SemaphoreHandle_t terminationSemaphore;

void publish_status();
void play_wav(void *arg);
void breeze_mode(void *arg);
void countTask();
void wifiTask(void *arg);
void mqttTask(void *arg);
void heartbeat(void *arg);
void eventProcessor(void *arg);

#endif /* TASKS_H */
