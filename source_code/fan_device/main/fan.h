#ifndef FAN_H
#define FAN_H

#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#define DEBOUNCE_DELAY pdMS_TO_TICKS(180)

extern TickType_t currentInterrupt;
extern TickType_t lastInterrupt;

extern int i, i_fanState;
extern int function;
extern char *fanState;
extern int dutyCycle;
extern int rpm;

extern struct mqttData isrStruct;

extern ledc_timer_config_t timer;
extern ledc_channel_config_t channel;

extern int counter;
extern void rpm_handler(void *arg);
extern void power_button(void *arg);
extern void cycle_dc(void *arg);

#endif /* FAN_H */
