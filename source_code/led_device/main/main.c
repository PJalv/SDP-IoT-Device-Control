#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "led_strip.h"
#include "esp_log.h"
#include "esp_err.h"
#include "../../../utils/mqtt.h"
#include "../../../utils/wifi.c"
#include "led_config.c"
#include "driver/gpio.h"

static const char *TAG = "example";

TaskHandle_t task1Handle = NULL;

void init()
{
    nvs_flash_init();

    topicArray subscribeTopics = {
        .topics = {
            "led/status/power"},
        .numStrings = 1};
    sendStringArray(&subscribeTopics);
}

void task1(void *arg)
{
    int i;
    led_strip_handle_t led_strip = configure_led();
    bool isLedOn = false;

    for (int i = 0; i < LED_STRIP_LED_NUMBERS; i++)
    {
        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, 28, 75, 169));
    }
    /* Refresh the strip to send data */
    ESP_ERROR_CHECK(led_strip_refresh(led_strip));

    ESP_LOGI(TAG, "Start blinking LED strip");
    struct mqttData temp;
    while (1)
    {
        if (xSemaphoreTake(dataSemaphore, portTICK_PERIOD_MS) == pdTRUE)
        {
            temp = pop();
            i = temp.data;
            switch (i)
            {
            case 0:
                led_strip_clear(led_strip);
                break;
            case 1:
                for (int i = 0; i < LED_STRIP_LED_NUMBERS; i++)
                {
                    ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, 28, 75, 169));
                }
                ESP_ERROR_CHECK(led_strip_refresh(led_strip));
            default:
                printf("not valid parameter at the moment\n");
                break;
            }
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    init();
    xTaskCreate(task1, "task1", 4096, NULL, 10, &task1Handle);
}
