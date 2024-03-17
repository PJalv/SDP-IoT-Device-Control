#ifndef STORAGE_NVS_H
#define STORAGE_NVS_H

#include "nvs_flash.h"

#define FAN_NAMESPACE "fan"
#define POWER_KEY "power"
#define RPM_KEY "rpm"

// Function prototypes
void setFanInfo(int power, int function, int dutyCycle);
void getFanInfo(int *power, int *function, int *dutyCycle);

#endif /* FAN_NVS_H */
