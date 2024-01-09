#include <stdio.h>
#include "nvs_flash.h"

#define LED_NAMESPACE "LED"

#define POWER_KEY "power"
#define RED_KEY "RED"
#define GREEN_KEY "GREEN"
#define BLUE_KEY "BLUE"

void setLEDInfo(int power, int red, int green, int blue)
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
    err = nvs_open(LED_NAMESPACE, NVS_READWRITE, &my_handle);
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

    err |= nvs_set_i32(my_handle, RED_KEY, red);
    if (err != ESP_OK)
    {
        printf("Error (%s) setting red value to NVS!\n", esp_err_to_name(err));
    }
    else
    {
        printf("Red value stored in NVS: %d\n", red);
    }

    err |= nvs_set_i32(my_handle, GREEN_KEY, green);
    if (err != ESP_OK)
    {
        printf("Error (%s) setting green value to NVS!\n", esp_err_to_name(err));
    }
    else
    {
        printf("Green value stored in NVS: %d\n", green);
    }
    err |= nvs_set_i32(my_handle, BLUE_KEY, blue);
    if (err != ESP_OK)
    {
        printf("Error (%s) setting blue value to NVS!\n", esp_err_to_name(err));
    }
    else
    {
        printf("Blue value stored in NVS: %d\n", blue);
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

void getLEDInfo(int *power, int *red, int *green, int *blue)
{
    nvs_handle_t my_handle;
    // Open NVS namespace
    esp_err_t err = nvs_open(LED_NAMESPACE, NVS_READONLY, &my_handle);
    if (err != ESP_OK)
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return;
    }

    // Retrieve values from NVS
    err = nvs_get_i32(my_handle, POWER_KEY, power);
    if (err != ESP_OK)
    {
        printf("Error (%s) getting power value from NVS!\n", esp_err_to_name(err));
    }
    else
    {
        printf("Power value retrieved from NVS: %d\n", *power);
    }

    err = nvs_get_i32(my_handle, RED_KEY, red);
    if (err != ESP_OK)
    {
        printf("Error (%s) getting red value from NVS!\n", esp_err_to_name(err));
    }
    else
    {
        printf("Red value retrieved from NVS: %d\n", *red);
    }
    err = nvs_get_i32(my_handle, BLUE_KEY, blue);
    if (err != ESP_OK)
    {
        printf("Error (%s) getting blue value from NVS!\n", esp_err_to_name(err));
    }
    else
    {
        printf("Blue value retrieved from NVS: %d\n", *blue);
    }
    err = nvs_get_i32(my_handle, GREEN_KEY, green);
    if (err != ESP_OK)
    {
        printf("Error (%s) getting green value from NVS!\n", esp_err_to_name(err));
    }
    else
    {
        printf("Green value retrieved from NVS: %d\n", *green);
    }

    // Close NVS
    nvs_close(my_handle);
}
