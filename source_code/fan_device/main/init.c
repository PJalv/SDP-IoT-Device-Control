#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "nvs_flash.h"
#include "storage_nvs.h"
#include "esp_spiffs.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/gpio.h"

#include "fan.h"
#include "audio.h"
#include "../../utils/mqtt.h"

SemaphoreHandle_t semaphoreDutyCycle;
SemaphoreHandle_t semaphoreAudio;
SemaphoreHandle_t terminationSemaphore;

gpio_isr_t rpm_handler_handle = &rpm_handler;

/**
 * @brief Initializes internal ESP processes, this includes nvs storage, task semaphores, audio, and fan pwm channels
 * as well as GPIO.
 *
 */
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
    else
    {
        printf("Semaphore Active!");
    }
    semaphoreAudio = xSemaphoreCreateBinary();

    if (semaphoreAudio == NULL)
    {
        /* There was insufficient FreeRTOS heap available for the semaphore to
        be created successfully. */
    }
    else
    {
        printf("Semaphore Active!");
        xSemaphoreGive(semaphoreAudio);
    }
    ledc_timer_config(&timer);
    getFanInfo(&i_fanState, &function, &dutyCycle);
    switch (i_fanState)
    {
    case 0:
        gpio_set_level(19, 1);
        break;
    case 1:
        gpio_set_level(19, 0);
        printf("FAN DC FROM NVS:%d", dutyCycle);
        channel.duty = dutyCycle;
        ledc_channel_config(&channel);

    default:
        break;
    }

    i2s_setup();
    esp_vfs_spiffs_conf_t spiffsConfig = {
        .base_path = "/storage",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = false};

    esp_err_t ret = esp_vfs_spiffs_register(&spiffsConfig);
    if (ret != ESP_OK)
    {
        ESP_LOGE("SPIFFS", "Failed to initialize SPIFFS. Error %d", ret);
    }
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

void endInit()
{
    gpio_isr_handler_add(33, cycle_dc, NULL);
    gpio_isr_handler_add(32, power_button, NULL);
}