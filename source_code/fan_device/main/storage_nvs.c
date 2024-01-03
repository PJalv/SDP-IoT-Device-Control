#include <stdio.h>
#include "nvs_flash.h"

#define FAN_NAMESPACE "fan"
#define POWER_KEY "power"
#define RPM_KEY "rpm"

void setFanInfo(int power, int dutyCycle)
{
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err != ESP_OK)
    {
        printf("Error (%s) initializing NVS.\n", esp_err_to_name(err));
        return;
    }

    nvs_handle_t my_handle;
    // Open NVS namespace
    err = nvs_open(FAN_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return;
    }

    // Store values in NVS
    err = nvs_set_i32(my_handle, POWER_KEY, power);
    if (err != ESP_OK)
    {
        printf("Error (%s) setting power value to NVS!\n", esp_err_to_name(err));
    }
    else
    {
        printf("Power value stored in NVS: %d\n", power);
    }

    err |= nvs_set_i32(my_handle, RPM_KEY, dutyCycle);
    if (err != ESP_OK)
    {
        printf("Error (%s) setting duty cycle value to NVS!\n", esp_err_to_name(err));
    }
    else
    {
        printf("Duty cycle value stored in NVS: %d\n", dutyCycle);
    }

    // Commit changes to NVS
    err = nvs_commit(my_handle);
    if (err != ESP_OK)
    {
        printf("Error (%s) committing data to NVS!\n", esp_err_to_name(err));
    }
    else
    {
        printf("Values successfully committed to NVS!\n");
    }

    // Close NVS
    nvs_close(my_handle);
}

void getFanInfo(int *power, int *dutyCycle)
{
    nvs_handle_t my_handle;
    // Open NVS namespace
    esp_err_t err = nvs_open(FAN_NAMESPACE, NVS_READONLY, &my_handle);
    if (err != ESP_OK)
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return;
    }

    // Retrieve values from NVS
    err = nvs_get_i32(my_handle, POWER_KEY, power);
    if (err != ESP_OK)
    {
        printf("Error (%s) reading power value from NVS!\n", esp_err_to_name(err));
    }
    else
    {
        printf("Retrieved power value from NVS: %d\n", *power);
    }

    err |= nvs_get_i32(my_handle, RPM_KEY, dutyCycle);
    if (err != ESP_OK)
    {
        printf("Error (%s) reading duty cycle value from NVS!\n", esp_err_to_name(err));
    }
    else
    {
        printf("Retrieved duty cycle value from NVS: %d\n", *dutyCycle);
    }

    // Close NVS
    nvs_close(my_handle);
}
