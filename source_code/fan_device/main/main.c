#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "../../utils/wifi.c"
#include "mqtt.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

TaskHandle_t wifiTaskHandle = NULL;
TaskHandle_t task1Handle = NULL;
TaskHandle_t mqttTaskHandle = NULL;
TaskHandle_t countTaskHandle = NULL;
int counter = 0;
static void IRAM_ATTR intr_handler(void *arg)
{
    counter++;
}

void countTask()
{
    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("Current RPM = %d\n...", counter * 30);
        counter = 0;
    }
    vTaskDelete(NULL);
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

void task1(void *arg)
{
    int i;
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 25000,
        .clk_cfg = LEDC_AUTO_CLK};
    ledc_timer_config(&timer);
    ledc_channel_config_t channel = {
        .gpio_num = 16,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 32,
        .hpoint = 0};

    gpio_set_level(19, 1);

    ledc_channel_config(&channel);
    struct mqttData temp;
    while (1)
    {
        if (xSemaphoreTake(dataSemaphore, portTICK_PERIOD_MS) == pdTRUE)
        {
            temp = pop();
            i = temp.data;
            channel.duty = i;
            ledc_channel_config(&channel);
            printf("FAN DUTY CYCLE CHANGED: NEW D/C = %d \n", i);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Some delay between iterations
    }
}

void app_main(void)
{

    topicArray myStrings = {
        .topics = {
            "fan/status/duty_cycle"},
        .numStrings = 1 // Update this with the actual number of strings
    };
    sendStringArray(&myStrings);

    gpio_set_intr_type(15, GPIO_INTR_POSEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(15, intr_handler, NULL);
    gpio_set_direction(19, GPIO_MODE_OUTPUT);

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    // vTaskDelay(5000 / portTICK_PERIOD_MS);
    // gpio_set_level(19, 0);
    xTaskCreate(countTask, "countTask", 4096, NULL, 10, &countTaskHandle);
    xTaskCreate(wifiTask, "wifi", 4096, NULL, 10, &wifiTaskHandle);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    xTaskCreate(mqttTask, "mqtt", 4096, NULL, 10, &mqttTaskHandle);
    vTaskDelay(10000 / portTICK_PERIOD_MS);
    xTaskCreate(task1, "task1", 4096, NULL, 10, &task1Handle);
    // printf("Waiting for 10 secs...");
    // vTaskDelay(10000 / portTICK_PERIOD_MS);
    // // publish_state("test/status", "Hello from MQTT!");
}
