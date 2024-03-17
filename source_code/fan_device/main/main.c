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
#include "cJSON.h"
#include "esp_spiffs.h"
#include "driver/i2s_std.h"

#define DEBOUNCE_DELAY pdMS_TO_TICKS(180)
#define BUFFER_SIZE 4096
#define REBOOT_WAIT 5000 // reboot after 5 seconds
#define AUDIO_BUFFER 2048
TaskHandle_t task1Handle = NULL;
TaskHandle_t countTaskHandle = NULL;
TaskHandle_t heartbeatHandle = NULL;
TaskHandle_t arrayProcessHandle = NULL;
QueueHandle_t xDCQueue;
SemaphoreHandle_t semaphoreDutyCycle;
SemaphoreHandle_t terminationSemaphore;

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

TickType_t lastInterrupt = 0;
TickType_t currentInterrupt = 0;
int i, i_fanState = 0;
int function;
char *fanState = "OFF";
int dutyCycle = 32;
int counter = 0;
static int rpm;
struct mqttData isrStruct = {
    .topic = "",
    .integerPayload = {
        .isInteger = 0,
        .intData = 0},
    .jsonPayload = {.isJson = 0, .jsonData = ""}};

void publish_status()
{
    cJSON *fanDeviceObject = cJSON_CreateObject();

    cJSON_AddItemToObject(fanDeviceObject, "power", cJSON_CreateNumber(i_fanState));
    cJSON_AddItemToObject(fanDeviceObject, "dutyCycle", cJSON_CreateNumber(dutyCycle));
    cJSON_AddItemToObject(fanDeviceObject, "rpm", cJSON_CreateNumber(rpm));
    cJSON_AddItemToObject(fanDeviceObject, "function", cJSON_CreateNumber(function));

    char *payload = cJSON_Print(fanDeviceObject);
    publish_state("fan/status", payload);
    cJSON_free(payload);
    cJSON_Delete(fanDeviceObject);
}

static void IRAM_ATTR rpm_handler(void *arg)
{
    counter++;
}
static TickType_t last_interrupt_time = 0;
static void power_button(void *arg)
{
    int button_state = gpio_get_level(32); // Replace BUTTON_GPIO with your button's GPIO

    // Variables to track debounce
    TickType_t current_interrupt_time = xTaskGetTickCountFromISR();
    TickType_t time_since_last_interrupt = current_interrupt_time - last_interrupt_time;

    if (button_state == 0)
    { // Button pressed (assuming low is pressed)

        if (time_since_last_interrupt > DEBOUNCE_DELAY && time_since_last_interrupt != 0)
        {
            // Debounced button press detected
            if (i_fanState == 1)
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

static void cycle_dc(void *arg)
{
    static int cycle_speed;
    int button_state = gpio_get_level(33); // Replace BUTTON_GPIO with your button's GPIO

    // Variables to track debounce
    TickType_t current_interrupt_time = xTaskGetTickCountFromISR();
    TickType_t time_since_last_interrupt = current_interrupt_time - last_interrupt_time;

    if (button_state == 0)
    { // Button pressed (assuming low is pressed)

        if (time_since_last_interrupt > DEBOUNCE_DELAY && time_since_last_interrupt != 0)
        {
            if (0 <= dutyCycle && dutyCycle <= 250)
            {
                dutyCycle = 400;
            }
            else if (251 <= dutyCycle && dutyCycle <= 550)
            {
                dutyCycle = 700;
            }
            else if (551 <= dutyCycle && dutyCycle <= 800)
            {
                dutyCycle = 1000;
            }
            else if (801 <= dutyCycle && dutyCycle <= 1024)
            {
                dutyCycle = 96;
            }
            isrStruct.integerPayload.isInteger = 1;
            isrStruct.integerPayload.intData = dutyCycle;
            push(isrStruct);
            isrStruct.integerPayload.isInteger = 0;
            isrStruct.integerPayload.intData = 0;
            xSemaphoreGiveFromISR(dataSemaphore, pdFALSE);
        }

        last_interrupt_time = current_interrupt_time;
    }
}
i2s_chan_handle_t tx_handle;
esp_err_t i2s_setup(void)
{
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_handle, NULL));

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(24000),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = 26,
            .ws = 25,
            .dout = 22,
            .din = I2S_GPIO_UNUSED,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };
    return i2s_channel_init_std_mode(tx_handle, &std_cfg);
}
esp_err_t play_wav(char *fp)
{
    FILE *fh = fopen(fp, "rb");
    if (fh == NULL)
    {
        ESP_LOGE("AUDIO", "Failed to open file");
        return ESP_ERR_INVALID_ARG;
    }

    // skip the header...
    fseek(fh, 44, SEEK_SET);

    // create a writer buffer
    int16_t *buf = calloc(AUDIO_BUFFER, sizeof(int16_t));
    size_t bytes_read = 0;
    size_t bytes_written = 0;

    bytes_read = fread(buf, sizeof(int16_t), AUDIO_BUFFER, fh);

    i2s_channel_enable(tx_handle);

    while (bytes_read > 0)
    {
        // write the buffer to the i2s
        i2s_channel_write(tx_handle, buf, bytes_read * sizeof(int16_t), &bytes_written, portMAX_DELAY);
        bytes_read = fread(buf, sizeof(int16_t), AUDIO_BUFFER, fh);
        ESP_LOGV("AUDIO", "Bytes read: %d", bytes_read);
    }

    i2s_channel_disable(tx_handle);
    free(buf);

    return ESP_OK;
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
    terminationSemaphore = xSemaphoreCreateBinary();

    if (terminationSemaphore == NULL)
    {
        /* There was insufficient FreeRTOS heap available for the semaphore to
        be created successfully. */
    }
    xDCQueue = xQueueCreate(5, sizeof(int));

    ledc_timer_config(&timer);
    getFanInfo(&i_fanState, &dutyCycle);
    printf("FAN DC FROM NVS:%d", dutyCycle);
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
            "fan/control"},
        .numStrings = 1 // Update this with the actual number of strings (topics)
    };

    sendStringArray(&subscribeTopics);

    gpio_install_isr_service(0);
    gpio_set_intr_type(15, GPIO_INTR_POSEDGE);
    gpio_isr_handler_add(15, rpm_handler, NULL);
    gpio_set_direction(19, GPIO_MODE_OUTPUT);
    gpio_set_direction(32, GPIO_MODE_INPUT);
    gpio_set_direction(33, GPIO_MODE_INPUT);
    gpio_set_direction(27, GPIO_MODE_INPUT);
    gpio_set_intr_type(32, GPIO_INTR_NEGEDGE);
    gpio_set_pull_mode(32, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(33, GPIO_INTR_NEGEDGE);
    gpio_set_intr_type(27, GPIO_INTR_NEGEDGE);
    // gpio_isr_handler_add(27, increase_dc, NULL);
    gpio_set_pull_mode(33, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(27, GPIO_PULLUP_ONLY);
}

void countTask()
{
    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("Current RPM = %d\n...", counter * 30);
        printf("Current DC = %d\n...", dutyCycle);
        rpm = counter * 30;
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

void heartbeat(void *arg)
{

    while (1)
    {
        publish_state("device_heartbeat", "fan");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        publish_status();
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
                printf("TXINT: %d\n", txInt);
                switch (txInt)
                {
                case 0:
                    gpio_set_level(19, 0);
                    i_fanState = 0;
                    break;
                case 1:
                    gpio_set_level(19, 1);
                    i_fanState = 1;

                    break;
                default:
                    dutyCycle = txInt;
                    channel.duty = dutyCycle;
                    ledc_channel_config(&channel);
                    break;
                }

                publish_status();
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
                        break;
                    case 2:
                        function = 2;
                        break;
                    default:
                        break;
                    }
                }
                publish_status();
            }
            else
            {
                printf("Invalid array configuration.");
            }
        }
    };
}

void app_main(void)
{
    init();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    xTaskCreate(wifiTask, "wifi", 4096, NULL, 10, &wifiTaskHandle);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    xTaskCreate(mqttTask, "mqtt", 4096, NULL, 10, &mqttTaskHandle);
    xTaskCreate(arrayProcess, "Event processor", 4096, NULL, 10, &arrayProcessHandle);
    xTaskCreate(countTask, "countTask", 4096, NULL, 10, &countTaskHandle);
    xTaskCreate(heartbeat, "Heartbeat", 4096, NULL, 10, &heartbeatHandle);
    gpio_isr_handler_add(33, cycle_dc, NULL);
    gpio_isr_handler_add(32, power_button, NULL);
    i2s_setup();
    esp_vfs_spiffs_conf_t config = {
        .base_path = "/storage",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = false};
    esp_vfs_spiffs_register(&config);
    play_wav("/storage/output.wav");
}
