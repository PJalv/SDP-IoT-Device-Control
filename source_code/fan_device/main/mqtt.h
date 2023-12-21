#ifndef MQTT_H
#define MQTT_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_event.h"
#include "mqtt_client.h"

#define MAX_SIZE 1
#define MAX_STRINGS 10
extern SemaphoreHandle_t dataSemaphore;

struct mqttData
{
    char *topic;
    int data;
};

typedef struct
{
    char *topics[10]; // Assuming a maximum of 10 strings
    int numStrings;
} topicArray;

void sendStringArray(topicArray *array); // Function declaration

extern struct mqttData transfer[MAX_SIZE];

void push(struct mqttData value);
struct mqttData pop();
void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
void mqtt_app_start(void);
void publish_state(char *topic, char *data);
void topic_subscribe(char *topic);

#endif // MQTT_H
