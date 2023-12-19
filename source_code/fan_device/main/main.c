#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../../utils/wifi.c"

void app_main(void)
{
    wifi_main();
    while (!wifi_connected)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    int i = 0;
    while (1)
    {
        printf("[%d] Hello world!\n", i);
        i++;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
