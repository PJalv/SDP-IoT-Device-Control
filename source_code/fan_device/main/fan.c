#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "../../utils/mqtt.h"

#define DEBOUNCE_DELAY pdMS_TO_TICKS(180)

TickType_t currentInterrupt = 0;
TickType_t lastInterrupt = 0;

int i, i_fanState = 0;
int function;
char *fanState = "OFF";
int dutyCycle = 32;
int rpm;

struct mqttData isrStruct = {
    .topic = "",
    .integerPayload = {
        .isInteger = 0,
        .intData = 0},
    .jsonPayload = {.isJson = 0, .jsonData = ""}};
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
void IRAM_ATTR rpm_handler(void *arg)
{
    counter++;
}

static TickType_t last_interrupt_time = 0;
void power_button(void *arg)
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

void cycle_dc(void *arg)
{
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