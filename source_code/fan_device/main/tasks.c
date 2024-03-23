#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "cJSON.h"

#include "audio.h"
#include "fan.h"
#include "../../utils/mqtt.h"
#include "../../utils/wifi.h"
#include "storage_nvs.h"
TaskHandle_t currentHandle = NULL;
TaskHandle_t countTaskHandle = NULL;
TaskHandle_t heartbeatHandle = NULL;
TaskHandle_t eventProcessorHandle = NULL;

extern SemaphoreHandle_t semaphoreDutyCycle;
extern SemaphoreHandle_t terminationSemaphore;
extern SemaphoreHandle_t semaphoreAudio;
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

void play_wav(void *arg)
{
    if (xSemaphoreTake(semaphoreAudio, 0) == pdTRUE)
    {
        FILE *fh = fopen((char *)arg, "rb");
        if (fh == NULL)
        {
            ESP_LOGE("AUDIO", "Failed to open file");
            xSemaphoreGive(semaphoreAudio); // Release semaphore if file open failed
            vTaskDelete(NULL); // Delete task and exit
            return;
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
        fclose(fh); // Close the file

        xSemaphoreGive(semaphoreAudio);
    }
    vTaskDelete(NULL);
}
void breeze_mode(void *arg)
{
    const int minSpeed = 150;
    const int maxSpeed = 1000;
    const int increment = 50;
    printf("Entered breeze mode\n");
    int targetSpeedPast = 0;
    while (1)
    {
        if (xSemaphoreTake(terminationSemaphore, 0) == pdTRUE)
        {
            break; // Terminate the task
        }
        int targetSpeed = minSpeed + esp_random() % (maxSpeed - minSpeed);
        printf("Target speed: %d\n", targetSpeed);
        int direction = (targetSpeed > targetSpeedPast) ? increment : -increment;

        for (int i = targetSpeedPast; (direction > 0 && i < targetSpeed) || (direction < 0 && i > targetSpeed); i += direction)
        {
            printf("Duty cycle: %d\n", i);
            channel.duty = i;
            dutyCycle = i;
            ledc_channel_config(&channel);
            printf("waiting in loop...\n");
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }

        channel.duty = targetSpeed;
        dutyCycle = targetSpeed;
        ledc_channel_config(&channel);
        targetSpeedPast = targetSpeed;
        printf("waiting...\n");
        vTaskDelay((3 + esp_random() % (6 - 3)) * 1000 / portTICK_PERIOD_MS);
    }
    printf("Exited breeze mode\n");

    vTaskDelete(NULL);
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

void eventProcessor(void *arg)
{
    int txInt;
    struct mqttData temp;
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
            }
            temp = pop();
            if (temp.integerPayload.isInteger == 1)
            {

                printf("POPPED DATA HAS INTEGER.\n");
                txInt = temp.integerPayload.intData;
                printf("SET txINT SUCCESSFULLY");
                // xQueueSend(xDCQueue, &txInt, portMAX_DELAY);
                // xSemaphoreGive(semaphoreDutyCycle);
                // printf("Data Sent to queue\n");
                printf("TXINT: %d\n", txInt);
                switch (txInt)
                {
                case 0:
                    gpio_set_level(19, 1);
                    i_fanState = 0;
                    break;
                case 1:
                    gpio_set_level(19, 0);
                    i_fanState = 1;

                    break;
                default:
                    dutyCycle = txInt;
                    channel.duty = dutyCycle;
                    ledc_channel_config(&channel);
                    break;
                }
                function = 0;
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
                        xTaskCreate(breeze_mode, "Breeze Mode", 4096, NULL, 10, &currentHandle);
                        xTaskCreate(play_wav, "Play Audio", 4096, (void *)"/storage/breeze.wav", 10, &currentHandle);
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
            setFanInfo(i_fanState, function, dutyCycle);
        }
    };
}