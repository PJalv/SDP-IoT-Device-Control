#include <stdio.h>
#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "../../utils/wifi.c"
#include "../../utils/mqtt.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "nvs_flash.h"
#include "storage_nvs.h"

TaskHandle_t task1Handle = NULL;
TaskHandle_t countTaskHandle = NULL;
TaskHandle_t arrayProcessHandle = NULL;
QueueHandle_t xDCQueue;
SemaphoreHandle_t semaphoreDutyCycle;
int i, i_fanState = 0;
char *fanState = "OFF";
int dutyCycle = 32;
ledc_timer_config_t timer = {
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .duty_resolution = LEDC_TIMER_10_BIT,
    .timer_num = LEDC_TIMER_0,
    .freq_hz = 25000,
    .clk_cfg = LEDC_AUTO_CLK};
ledc_channel_config_t channel = {
    .gpio_num = 16,
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .channel = LEDC_CHANNEL_0,
    .timer_sel = LEDC_TIMER_0,
    .duty = 3,
    .hpoint = 0};
int counter = 0;
static void IRAM_ATTR intr_handler(void *arg)
{
    counter++;
}

void init()
{

    nvs_flash_init();
    semaphoreDutyCycle = xSemaphoreCreateBinary();

    if (semaphoreDutyCycle == NULL)
    {
        /* There was insufficient FreeRTOS heap available for the semaphore to
        be created successfully. */
    }
    else
    {
        printf("Semaphore Active!");
    }
    xDCQueue = xQueueCreate(5, sizeof(int));

    ledc_timer_config(&timer);
    getFanInfo(&i_fanState, &dutyCycle);
    channel.duty = dutyCycle;
    ledc_channel_config(&channel);
    if (i_fanState == 0) // this should be the first time the relay triggers fan to turn on/off

    {
        gpio_set_level(19, 0);
    }
    else
    {
        gpio_set_level(19, 1);
    }
    setFanInfo(i_fanState, dutyCycle);
    topicArray subscribeTopics = {
        .topics = {
            "fan/status/duty_cycle"},
        .numStrings = 1 // Update this with the actual number of strings (topics)
    };

    sendStringArray(&subscribeTopics);

    gpio_set_intr_type(15, GPIO_INTR_POSEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(15, intr_handler, NULL);
    gpio_set_direction(19, GPIO_MODE_OUTPUT);
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

void arrayProcess(void *arg)
{
    int txInt;
    struct mqttData temp;
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
                xQueueSend(xDCQueue, &txInt, portMAX_DELAY);
                xSemaphoreGive(semaphoreDutyCycle);
                printf("Data Sent to queue\n");
            }
            else if (temp.jsonPayload.isJson == 1)
            {
                /* code */
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
    ledc_channel_config_t channel = {
        .gpio_num = 16,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = dutyCycle,
        .hpoint = 0};
    ledc_channel_config(&channel);

    int dcApply;
    while (1)
    {

        xQueueReceive(xDCQueue, &dcApply, portMAX_DELAY);
        if (xSemaphoreTake(semaphoreDutyCycle, portTICK_PERIOD_MS) == pdTRUE)
        {

            printf("Acting upon receieved data from QUEUE\n");
            switch (dcApply)
            {
            case 0:
                gpio_set_level(19, 0);
                fanState = "ON";
                i_fanState = 0;
                setFanInfo(0, dutyCycle);
                publish_state("fan/status/power", fanState);

                break;
            case 1:
                gpio_set_level(19, 1);
                fanState = "OFF";
                i_fanState = 1;
                publish_state("fan/status/power", fanState);
                setFanInfo(1, dutyCycle);
                break;
            default:
                channel.duty = dcApply;
                dutyCycle = channel.duty;
                ledc_channel_config(&channel);
                printf("FAN DUTY CYCLE CHANGED: NEW D/C = %d \n", dutyCycle);
                setFanInfo(i_fanState, dutyCycle);
            }
        }
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);
}

void app_main(void)
{
    init();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    xTaskCreate(wifiTask, "wifi", 4096, NULL, 10, &wifiTaskHandle);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    xTaskCreate(mqttTask, "mqtt", 4096, NULL, 10, &mqttTaskHandle);
    xTaskCreate(task1, "task1", 4096, NULL, 10, &task1Handle);
    xTaskCreate(countTask, "countTask", 4096, NULL, 10, &countTaskHandle);
    xTaskCreate(arrayProcess, "Event processor", 4096, NULL, 10, &arrayProcessHandle);
}
