#include "mqtt.h"
#include "esp_log.h"
#include "../../credentials.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "mqtt_client.h"


bool hasConnected = false;
static const char *TAG = "MQTT";
struct mqttData temp = {
    .topic = "",
    .data = 0};

SemaphoreHandle_t dataSemaphore = NULL;

struct mqttData transfer[MAX_SIZE];
int transfer_size = 0;

topicArray rxTopics;

void sendStringArray(topicArray *array)
{
    printf("Received String Array with %d strings...\n", array->numStrings);
    for (int i = 0; i < array->numStrings; ++i)
    {
        printf("%s\n", array->topics[i]);
    }
    rxTopics = *array;
}

void push(struct mqttData value)
{

    if (transfer_size < MAX_SIZE)
    {
        transfer[transfer_size++] = value;
        ESP_LOGI(TAG, "Pushed to the array.");
    }
    else
    {
        ESP_LOGI(TAG, "Array is full, cannot push.");
    }
}

struct mqttData pop()
{
    struct mqttData defaultValue = {NULL, 0};
    if (transfer_size > 0)
    {
        ESP_LOGI(TAG, "Popped from the array.");
        return transfer[--transfer_size];
    }
    else
    {
        ESP_LOGI(TAG, "Array is empty, cannot pop.");
        return defaultValue;
    }
}
static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}
esp_mqtt_client_handle_t mqtt_client = NULL;
void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    mqtt_client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        for (int i = 0; i < rxTopics.numStrings; i++)
        {
            topic_subscribe(rxTopics.topics[i]);
        }
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "SUBSCRIBED TO TOPIC");
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        printf(" , data length: %d\n", event->data_len);

        char *result = malloc(((event->data_len) + 1) * sizeof(char));

        if (result == NULL)
        {
            fprintf(stderr, "Memory allocation failed\n");
        }
        strncpy(result, event->data, event->data_len);
        result[event->data_len] = '\0';

        temp.data = atoi(result);
        temp.topic = event->topic;
        printf("TEMP DATA = %d\n", temp.data);
        push(temp);
        printf("\nPushed %d into the transfer array.\n", transfer[0].data);

        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        free(result);
        xSemaphoreGive(dataSemaphore);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}
void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = BROKER_URI,
    };
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
    dataSemaphore = xSemaphoreCreateBinary();

    if (dataSemaphore == NULL)
    {
        /* There was insufficient FreeRTOS heap available for the semaphore to
        be created successfully. */
    }
    else
    {
        printf("Semaphore Active!");
    }
    vTaskSuspend(NULL);
}
void publish_state(char *topic, char *data)
{
    char *txData = data;
    char *txTopic = topic;
    if (mqtt_client != NULL)
    {
        // Publish message using the stored client handle
        esp_mqtt_client_publish(mqtt_client, txTopic, txData, 0, 1, 0);
    }
    else
    {
        ESP_LOGE(TAG, "MQTT Client Handle is NULL");
    }
}

void topic_subscribe(char *topic)
{
    // char *txData = data;
    char *txTopic = topic;
    if (mqtt_client != NULL)
    {
        while (1)
        {
            if (esp_mqtt_client_subscribe_single(mqtt_client, txTopic, 1) < 0)
            {
                printf("COULD NOT SUBSCRIBE, RECONNECTING...");
                vTaskDelay(2000 / portTICK_PERIOD_MS);
            }
            else
            {

                break;
            }
        }
    }
    else
    {
        ESP_LOGE(TAG, "MQTT Client Handle is NULL");
    }
}
