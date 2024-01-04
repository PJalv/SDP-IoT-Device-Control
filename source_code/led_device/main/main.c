#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "led_strip.h"
#include "esp_log.h"
#include "esp_err.h"
#include "../../../utils/mqtt.h"
#include "../../../utils/wifi.c"
#include "led_config.c"
#include "driver/gpio.h"
#include "cJSON.h"

// static const char *TAG = "example";

TaskHandle_t task1Handle = NULL;
TaskHandle_t arrayProcessHandle = NULL;
QueueHandle_t xPowerQueue;
QueueHandle_t xColorQueue;
SemaphoreHandle_t semaphorePower;
SemaphoreHandle_t semaphoreColor;
void init()
{
    nvs_flash_init();
    semaphoreColor = xSemaphoreCreateBinary();

    if (semaphoreColor == NULL)
    {
        /* There was insufficient FreeRTOS heap available for the semaphore to
        be created successfully. */
    }
    else
    {
        printf("Semaphore Active!");
    }
    semaphorePower = xSemaphoreCreateBinary();

    if (semaphorePower == NULL)
    {
        /* There was insufficient FreeRTOS heap available for the semaphore to
        be created successfully. */
    }
    else
    {
        printf("Semaphore Active!");
    }
    xColorQueue = xQueueCreate(5, 3 * sizeof(int));
    topicArray subscribeTopics = {
        .topics = {
            "led/status/power",
            "led/control/color"},
        .numStrings = 2};
    sendStringArray(&subscribeTopics);
}
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
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
void arrayProcess(void *arg)
{
    int txInt, red, green, blue;
    char *red_obj;
    char *green_obj;
    char *blue_obj;
    struct mqttData temp;
    int txRGB[3];
    printf("Waiting for data event...\n");
    while (1)
    {
        if (xSemaphoreTake(dataSemaphore, portTICK_PERIOD_MS) == pdTRUE)
        {
            printf("IN MAIN FUNCTION FOR POPPING.\n");
            temp = pop();
            if (temp.integerPayload.isInteger == 1)
            {
                printf("POPPED DATA HAS INTEGER.\n");
                txInt = temp.integerPayload.intData;
                printf("SET txINT SUCCESSFULLY");
                xQueueSend(xPowerQueue, &txInt, portMAX_DELAY);
                xSemaphoreGive(semaphorePower);
                printf("Data Sent to queue\n");
            }
            else if (temp.jsonPayload.isJson == 1)
            {
                cJSON *root = cJSON_Parse(temp.jsonPayload.jsonData);
                if (root == NULL)
                {
                    printf("Error parsing JSON: %s\n", cJSON_GetErrorPtr());
                    
                }
                cJSON *red_obj = cJSON_GetObjectItem(root, "red");
                cJSON *green_obj = cJSON_GetObjectItem(root, "green");
                cJSON *blue_obj = cJSON_GetObjectItem(root, "blue");
                if (red_obj == NULL || green_obj == NULL || blue_obj == NULL)
                {
                    printf("Error getting JSON values\n");
                    cJSON_Delete(root);
                    
                }
                red = red_obj->valueint;
                green = green_obj->valueint;
                blue = blue_obj->valueint;
                printf("Red: %d\n", red);
                printf("Green: %d\n", green);
                printf("Blue: %d\n", blue);
                cJSON_Delete(root);
                txRGB[0] = red;
                txRGB[1] = green;
                txRGB[2] = blue;
                xQueueSend(xColorQueue, &txRGB, portMAX_DELAY);
                xSemaphoreGive(semaphoreColor);
            }
            else
            {
                printf("Invalid array configuration.");
            }
        }
    };
}
void task1(void *arg)
{

    int rxRGB[3];
    int red, green, blue;
    red = green = blue = 255;
    led_strip_handle_t led_strip = configure_led();
    bool isLedOn = false;
    for (int i = 0; i < LED_STRIP_LED_NUMBERS; i++)
    {
        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, red, green, blue));
    }
    /* Refresh the strip to send data */
    ESP_ERROR_CHECK(led_strip_refresh(led_strip));

    ESP_LOGI(TAG, "Start blinking LED strip");
    struct mqttData temp;
    while (1)
    {
        xQueueReceive(xColorQueue, &rxRGB, portMAX_DELAY);
        if (xSemaphoreTake(semaphoreColor, portTICK_PERIOD_MS) == pdTRUE)
        {

            red = rxRGB[0];
            green = rxRGB[1];
            blue = rxRGB[2];
            for (int i = 0; i < LED_STRIP_LED_NUMBERS; i++)
            {
                ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, red, green, blue));
            }
            ESP_ERROR_CHECK(led_strip_refresh(led_strip));
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    init();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    xTaskCreate(wifiTask, "wifi", 4096, NULL, 10, &wifiTaskHandle);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    xTaskCreate(mqttTask, "mqtt", 4096, NULL, 10, &mqttTaskHandle);
    xTaskCreate(task1, "task1", 4096, NULL, 10, &task1Handle);
    xTaskCreate(arrayProcess, "Event processor", 4096, NULL, 10, &arrayProcessHandle);
}
