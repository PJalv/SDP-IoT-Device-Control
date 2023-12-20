#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../../utils/wifi.c"
#include "mqtt.c"

TaskHandle_t wifiTaskHandle = NULL;
TaskHandle_t task1Handle = NULL;
TaskHandle_t mqttTaskHandle = NULL;

void wifiTask(void *arg)
{

    wifi_main();
    while (1)
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void mqttTask(void *arg)
{
    mqtt_app_start();
    while (1)
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void task1(void *arg)
{
    int i = 0;
    while (1)
    {

        // Normal task1 logic (including WiFi reconnection)
        printf("[%d] Hello world!\n", i);
        i++;
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Some delay between iterations
    }
}

void app_main(void)
{
    xTaskCreate(wifiTask, "wifi", 4096, NULL, 10, &wifiTaskHandle);
    // xTaskCreate(task1, "task1", 4096, NULL, 10, &task1Handle);;
    vTaskDelay(10000 / portTICK_PERIOD_MS);
    xTaskCreate(mqttTask, "mqtt", 4096, NULL, 10, &mqttTaskHandle);
    printf("Waiting for 10 secs to publish...");
    vTaskDelay(10000 / portTICK_PERIOD_MS);
    publish_state("Hello from MQTT!");
}
