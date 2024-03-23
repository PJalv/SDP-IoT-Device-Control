
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "init.h"
#include "tasks.h"

#define BUFFER_SIZE 4096

void app_main(void)
{
    init();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    xTaskCreate(wifiTask, "wifi", BUFFER_SIZE, NULL, 10, &wifiTaskHandle);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    xTaskCreate(mqttTask, "mqtt", BUFFER_SIZE, NULL, 10, &mqttTaskHandle);
    xTaskCreate(eventProcessor, "Event processor", BUFFER_SIZE, NULL, 10, &eventProcessorHandle);
    xTaskCreate(countTask, "countTask", BUFFER_SIZE, NULL, 10, &countTaskHandle);
    xTaskCreate(heartbeat, "Heartbeat", BUFFER_SIZE, NULL, 10, &heartbeatHandle);
    endInit();
}
