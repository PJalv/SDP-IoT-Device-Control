#ifndef WIFI_H
#define WIFI_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "../credentials.h"

extern TaskHandle_t wifiTaskHandle;
extern const char *ssid;
extern const char *pass;
extern bool isWifiConnected;
extern char *wifiTag;

void wifi_connection();
void wifi_main();

#endif /* WIFI_H */
