#include <stdio.h>             //for basic printf commands
#include <string.h>            //for handling strings
#include "freertos/FreeRTOS.h" //for delay,mutexs,semphrs rtos operations
#include "esp_system.h"        //esp_init funtions esp_err_t
#include "esp_wifi.h"          //esp_wifi_init functions and wifi operations
#include "esp_log.h"           //for showing logs
#include "esp_event.h"         //for wifi event
#include "nvs_flash.h"         //non volatile storage
#include "lwip/err.h"          //light weight ip packets error handling
#include "lwip/sys.h"
#include "../credentials.h" //system applications for light weight ip apps
#include "freertos/task.h"

extern TaskHandle_t wifiTaskHandle = NULL;

const char *ssid = WIFI_SSID;
const char *pass = WIFI_PASSWORD;

extern bool isWifiConnected = false;
extern 
char *wifiTag = "WiFi";
static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_id == WIFI_EVENT_STA_START)
    {
        printf("WIFI CONNECTING....\n");
    }
    else if (event_id == WIFI_EVENT_STA_CONNECTED)
    {
        printf("WiFi CONNECTED\n");
    }
    else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        isWifiConnected = false;
        ESP_LOGW(wifiTag, "WiFi is down!");
        esp_wifi_connect();
        ESP_LOGI(wifiTag, "Reconnecting...\n");
    }
    else if (event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        char ip_addr[16];
        sprintf(ip_addr, IPSTR, IP2STR(&event->ip_info.ip));

        ESP_LOGI("WiFi", "Got IP: %s", ip_addr);
        isWifiConnected = true;
    }
}

void wifi_connection()
{
    //                          s1.4
    // 2 - Wi-Fi Configuration Phase
    esp_netif_init();
    esp_event_loop_create_default();     // event loop                    s1.2
    esp_netif_create_default_wifi_sta(); // WiFi station                      s1.3
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation); //
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = "",
            .password = "",

        }

    };
    strcpy((char *)wifi_configuration.sta.ssid, ssid);
    strcpy((char *)wifi_configuration.sta.password, pass);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    // 3 - Wi-Fi Start Phase
    esp_wifi_start();
    esp_wifi_set_mode(WIFI_MODE_STA);
    // 4- Wi-Fi Connect Phase
    esp_wifi_connect();
    printf("wifi_init_softap finished. SSID:%s  password:%s\n", ssid, pass);
}

void wifi_main(void)
{
    // nvs_flash_init();
    wifi_connection();

    // Add a loop to keep the task suspended after a successful connection
    while (!isWifiConnected)
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    printf("Task suspended\n");
    vTaskSuspend(NULL); // Suspend the task after a successful connection
}
