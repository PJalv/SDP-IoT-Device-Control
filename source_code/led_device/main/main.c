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
#include "storage_nvs.h"
#include "led_config.c"
#include "driver/gpio.h"
#include "cJSON.h"
#include "esp_random.h"

#define DEBOUNCE_DELAY pdMS_TO_TICKS(180)

TaskHandle_t arrayProcessHandle = NULL;
TaskHandle_t heartbeatHandle = NULL;
TaskHandle_t currentHandle = NULL;

QueueHandle_t xPowerQueue;
QueueHandle_t xColorQueue;
SemaphoreHandle_t semaphorePower;
SemaphoreHandle_t terminationSemaphore;
led_strip_handle_t led_strip;
int red = 0, green = 0, blue = 0, power = 0;
int function = 0;
TickType_t currentInterrupt = 0;
TickType_t lastInterrupt = 0;
int timeBetweenPresses;

char stringValue[10];

struct mqttData isrStruct = {
    .topic = "",
    .integerPayload = {
        .isInteger = 0,
        .intData = 0},
    .jsonPayload = {.isJson = 0, .jsonData = ""}};

double generateRandomNumber()
{
    // Get a random 32-bit word
    uint32_t randomValue = esp_random();

    // Map the 32-bit random value to the range [0, 255]
    double randomNumber = (double)(randomValue % 256);

    return randomNumber;
}

void publish_status()
{
    cJSON *ledDeviceObject = cJSON_CreateObject();

    // Add "power" key-value pair to the outer object
    cJSON_AddItemToObject(ledDeviceObject, "power", cJSON_CreateNumber(power));
    cJSON_AddItemToObject(ledDeviceObject, "function", cJSON_CreateNumber(function));

    // Create the "color" object
    cJSON *colorObject = cJSON_CreateObject();

    // Add "red", "green", and "blue" key-value pairs to the "color" object
    cJSON_AddItemToObject(colorObject, "red", cJSON_CreateNumber(red));
    cJSON_AddItemToObject(colorObject, "green", cJSON_CreateNumber(green));
    cJSON_AddItemToObject(colorObject, "blue", cJSON_CreateNumber(blue));

    // Add the "color" object to the outer object
    cJSON_AddItemToObject(ledDeviceObject, "color", colorObject);
    char *payload = cJSON_Print(ledDeviceObject);
    publish_state("led/status", payload);
    cJSON_free(payload);
    cJSON_Delete(ledDeviceObject);
}
static TickType_t last_interrupt_time = 0;
static void power_button(void *arg)
{
    // Read button state
    int button_state = gpio_get_level(12); // Replace BUTTON_GPIO with your button's GPIO

    // Variables to track debounce
    TickType_t current_interrupt_time = xTaskGetTickCountFromISR();
    TickType_t time_since_last_interrupt = current_interrupt_time - last_interrupt_time;

    if (button_state == 0)
    { // Button pressed (assuming low is pressed)

        if (time_since_last_interrupt > DEBOUNCE_DELAY && time_since_last_interrupt != 0)
        {
            // Debounced button press detected
            if (power == 1)
            {
                isrStruct.integerPayload.isInteger = 1;
                isrStruct.integerPayload.intData = 0;
            }
            else
            {
                isrStruct.integerPayload.isInteger = 1;
                isrStruct.integerPayload.intData = 1;
            }
            push(isrStruct);
            // Process the power change
            isrStruct.integerPayload.isInteger = 0;
            isrStruct.integerPayload.intData = 0;
            xSemaphoreGiveFromISR(dataSemaphore, pdFALSE);
        }

        last_interrupt_time = current_interrupt_time;
    }
}

static void change_color(void *arg)
{
    // Read button state
    int button_state = gpio_get_level(14); // Replace BUTTON_GPIO with your button's GPIO

    // Variables to track debounce
    TickType_t current_interrupt_time = xTaskGetTickCountFromISR();
    TickType_t time_since_last_interrupt = current_interrupt_time - last_interrupt_time;

    if (button_state == 0)
    { // Button pressed (assuming low is pressed)

        if (time_since_last_interrupt > DEBOUNCE_DELAY && time_since_last_interrupt != 0)
        {
            // Debounced button press detected
            cJSON *root = cJSON_CreateObject();

            // Set values for keys
            cJSON_AddItemToObject(root, "red", cJSON_CreateNumber(generateRandomNumber()));
            cJSON_AddItemToObject(root, "green", cJSON_CreateNumber(generateRandomNumber()));
            cJSON_AddItemToObject(root, "blue", cJSON_CreateNumber(generateRandomNumber()));

            // Convert the JSON object to a string
            char *json_data = cJSON_Print(root);
            isrStruct.jsonPayload.isJson = 1;
            isrStruct.jsonPayload.jsonData = json_data;
            // Release the cJSON object
            cJSON_Delete(root);
            push(isrStruct);
            // Process the power change
            xSemaphoreGiveFromISR(dataSemaphore, pdFALSE);
        }

        last_interrupt_time = current_interrupt_time;
    }
}

void trailing_rainbow(void *arg)
{
    const TickType_t delay = 10 / portTICK_PERIOD_MS;

    while (1)
    {
        if (xSemaphoreTake(terminationSemaphore, 0) == pdTRUE)
        {
            break; // Terminate the task
        }
        // Iterate through the LED strip pixels
        for (uint32_t i = 0; i < LED_STRIP_LED_NUMBERS; i++)
        {
            // Calculate hue based on the pixel index and a time variable
            uint16_t hue = ((i * 360) / LED_STRIP_LED_NUMBERS + (pdTICKS_TO_MS(xTaskGetTickCount()) / (int)(arg))) % 360;

            // Set the pixel color using the HSV values
            led_strip_set_pixel_hsv(led_strip, i, hue, 255, 255);
        }

        // Update the LED strip to apply the changes
        if (led_strip_refresh(led_strip) != ESP_OK)
        {
            continue;
        }
        // Introduce a delay for the trailing effect
        vTaskDelay(delay);
    }
    vTaskDelete(NULL);
}

void static_rainbow(void *arg)
{
    const TickType_t delay = 10 / portTICK_PERIOD_MS;
    while (1)
    {
        if (xSemaphoreTake(terminationSemaphore, 0) == pdTRUE)
        {
            break; // Terminate the task
        }

        // Calculate hue based on time
        uint16_t hue = (pdTICKS_TO_MS(xTaskGetTickCount()) / (int)(arg)) % 360;

        // Set the same color for all LEDs using the HSV values
        for (uint32_t i = 0; i < LED_STRIP_LED_NUMBERS; i++)
        {
            led_strip_set_pixel_hsv(led_strip, i, hue, 255, 255);
        }

        // Update the LED strip to apply the changes
        if (led_strip_refresh(led_strip) != ESP_OK)
        {
            continue;
        }
        vTaskDelay(delay);
        // Introduce a delay for the trailing effect
        // vTaskDelay(delay);
    }

    vTaskDelete(NULL);
}

void init()
{
    nvs_flash_init();
    semaphorePower = xSemaphoreCreateBinary();

    if (semaphorePower == NULL)
    {
    }
    else
    {
        printf("Semaphore Active!");
    }
    xColorQueue = xQueueCreate(5, 3 * sizeof(int));
    xPowerQueue = xQueueCreate(5, sizeof(int));

    getLEDInfo(&power, &function, &red, &green, &blue);
    if (red == 0 && green == 0 && blue == 0)
    {
        red = green = blue = 255;
    }
    terminationSemaphore = xSemaphoreCreateBinary();

    if (terminationSemaphore == NULL)
    {
        /* There was insufficient FreeRTOS heap available for the semaphore to
        be created successfully. */
    }
    led_strip = configure_led();
    switch (power)
    {
    case 0:
        led_strip_clear(led_strip);
        break;
    case 1:
        if (function != 0)
        {
            printf("Retrieving function...\n");
            switch (function)
            {
            case 1:
                xTaskCreate(static_rainbow, "static rainbow Effect", 1024, (void *)10, 8, &currentHandle);
                break;
            case 2:
                xTaskCreate(trailing_rainbow, "trailing rainbow Effect", 1024, (void *)10, 8, &currentHandle);
                break;
            default:
                break;
            }
        } // add more for more efefcts
        else
        {
            for (int i = 0; i < LED_STRIP_LED_NUMBERS; i++)
            {
                ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, red, green, blue));
            }
            /* Refresh the strip to send data */
            ESP_ERROR_CHECK(led_strip_refresh(led_strip));
        }
    default:
        break;
    }
    setLEDInfo(power, function, red, green, blue);

    topicArray subscribeTopics = {
        .topics = {
            "led/control/power",
            "led/control/color"},
        .numStrings = 2};
    sendStringArray(&subscribeTopics);

    gpio_install_isr_service(0);
    gpio_set_direction(12, GPIO_MODE_INPUT);
    gpio_set_pull_mode(12, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(12, GPIO_INTR_NEGEDGE);
    gpio_set_direction(14, GPIO_MODE_INPUT);
    gpio_set_pull_mode(14, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(14, GPIO_INTR_NEGEDGE);
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
void heartbeat(void *arg)
{
    while (1)
    {
        publish_state("device_heartbeat", "led");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}
void arrayProcess(void *arg)
{
    int txInt;
    txInt = 0;
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
            if (currentHandle != NULL)
            {
                printf("TASK DELETED\n");
                xSemaphoreGive(terminationSemaphore);
                currentHandle = NULL;

                while (led_strip_del(led_strip) != ESP_OK)
                {
                    vTaskDelay(50 / portTICK_PERIOD_MS);
                }
                led_strip = configure_led();
                printf("AFTER TASK DELETED\n");
                // led_strip_del(led_strip);
                // continue;
            }
            printf("IN MAIN FUNCTION FOR POPPING.\n");
            temp = pop();
            if (temp.integerPayload.isInteger == 1)
            {
                printf("POPPED DATA HAS INTEGER.\n");
                txInt = temp.integerPayload.intData;
                printf("SET txINT SUCCESSFULLY\n");
                // xQueueSend(xPowerQueue, &txInt, portMAX_DELAY);
                // xSemaphoreGive(semaphorePower);
                switch (txInt)
                {
                case 0:
                    led_strip_clear(led_strip);
                    power = 0;
                    break;
                case 1:
                    power = 1;
                    for (int i = 0; i < LED_STRIP_LED_NUMBERS; i++)
                    {
                        led_strip_set_pixel(led_strip, i, red, green, blue);
                    }
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                    led_strip_refresh(led_strip);
                    printf("Refreshed\n");
                default:
                    break;
                }
                setLEDInfo(power, function, red, green, blue);
                publish_status();
                printf("Data Sent to queue\n");
            }
            else if (temp.jsonPayload.isJson == 1)
            {

                cJSON *root = cJSON_Parse(temp.jsonPayload.jsonData);
                if (root == NULL)
                {
                    printf("Error parsing JSON: %s\n", cJSON_GetErrorPtr());
                }
                cJSON *functionItem = cJSON_GetObjectItem(root, "function");

                if (functionItem != NULL)
                {
                    switch (functionItem->valueint)
                    {
                    case 1:
                        function = 1;
                        xTaskCreate(static_rainbow, "rainbow Effect", 1024, (void *)10, 8, &currentHandle);
                        break;
                    case 2:
                        function = 2;
                        xTaskCreate(trailing_rainbow, "trailing rainbow Effect", 1024, (void *)10, 8, &currentHandle);
                        break;
                    default:
                        break;
                    }
                }
                else
                {
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
                    if (power != 0)
                    {
                        for (int i = 0; i < LED_STRIP_LED_NUMBERS; i++)
                        {
                            ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, red, green, blue));
                        }
                        ESP_ERROR_CHECK(led_strip_refresh(led_strip));
                    }
                }
                function = 0;
                setLEDInfo(power, function, red, green, blue);
                publish_status();
            }
            else
            {
                printf("Invalid array configuration.");
            }
        }
    }
};

void app_main(void)
{
    init();
    vTaskDelay(500 / portTICK_PERIOD_MS);
    xTaskCreate(wifiTask, "wifi", 4096, NULL, 10, &wifiTaskHandle);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    xTaskCreate(mqttTask, "mqtt", 4096, NULL, 10, &mqttTaskHandle);
    xTaskCreate(arrayProcess, "Event processor", 4096, NULL, 10, &arrayProcessHandle);
    xTaskCreate(heartbeat, "Heartbeat", 4096, NULL, 10, &heartbeatHandle);
    gpio_isr_handler_add(14, change_color, NULL);
    gpio_isr_handler_add(12, power_button, NULL);
}
