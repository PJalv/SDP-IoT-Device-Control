#ifndef STORAGE_NVS_H
#define STORAGE_NVS_H

#include "nvs_flash.h"

#define LED_NAMESPACE "LED"

#define POWER_KEY "power"
#define RED_KEY "RED"
#define GREEN_KEY "GREEN"
#define BLUE_KEY "BLUE"
// Function prototypes
void setLEDInfo(int power, int red, int green, int blue);
void getLEDInfo(int *power, int *red, int *green, int *blue);

#endif /* FAN_NVS_H */
