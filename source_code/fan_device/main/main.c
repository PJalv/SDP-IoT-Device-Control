#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "../../utils/wifi.c"
#include "mqtt.h"

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
        vTaskDelay(100000 / portTICK_PERIOD_MS);
    }
}

void task1(void *arg)
{

    int i = 800;
    struct mqttData temp;
    while (1)
    {
        if (xSemaphoreTake(dataSemaphore, portTICK_PERIOD_MS) == pdTRUE)
        {
            temp = pop();
            if (temp.data != i)
            {
                i = temp.data;
                printf("FAN RPM CHANGED: NEW RPM = %d \n", i);
            }
        }
        printf("Fan RPM: %d \n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Some delay between iterations
    }
}

void app_main(void)
{
    topicArray myStrings = {
        .topics = {
            "fan/status/rpm",
            "fan/status/temp",
            "fan/status/duty_cycle"},
        .numStrings = 3 // Update this with the actual number of strings
    };

    sendStringArray(&myStrings);
    xTaskCreate(wifiTask, "wifi", 4096, NULL, 10, &wifiTaskHandle);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    xTaskCreate(mqttTask, "mqtt", 4096, NULL, 10, &mqttTaskHandle);
    vTaskDelay(10000 / portTICK_PERIOD_MS);
    xTaskCreate(task1, "task1", 4096, NULL, 10, &task1Handle);
    // printf("Waiting for 10 secs...");
    // vTaskDelay(10000 / portTICK_PERIOD_MS);
    // // publish_state("test/status", "Hello from MQTT!");
}
