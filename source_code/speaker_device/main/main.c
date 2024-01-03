#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_bt.h"
#include "esp_bt_device.h"
#include "esp_bt_main.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"
#include "esp_gap_bt_api.h" 
#include "string.h"
#include "nvs_flash.h"
#include "driver/i2s.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define BUTTON_FORWARD_PIN 5
#define BUTTON_PLAY_PAUSE_PIN 23
#define BUTTON_BACKWARD_PIN 19
#define LED_RED_PIN 17
#define LED_GREEN_PIN 16
#define TAG "MY_BT_SPEAKER"

uint8_t my_bt_speaker_state = ESP_AVRC_PLAYBACK_STOPPED;
const char bt_device_name[] = "ESPEAKER";
bool avrc_conn = false;

static bool ct_button_play_pause_press = false;
static bool ct_button_backward_press = false;
static bool ct_button_forward_press = false;

void my_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    switch (event)
    {
    case ESP_BT_GAP_DISC_RES_EVT:
        break;
    case ESP_BT_GAP_DISC_STATE_CHANGED_EVT:
        break;
    case ESP_BT_GAP_RMT_SRVCS_EVT:
        break;
    case ESP_BT_GAP_RMT_SRVC_REC_EVT:
        break;
    case ESP_BT_GAP_AUTH_CMPL_EVT:
        ESP_LOGI(TAG, "auth: state:%d, remote_name:%s", param->auth_cmpl.stat, param->auth_cmpl.device_name);
        break;
    case ESP_BT_GAP_PIN_REQ_EVT:
        break;
    case ESP_BT_GAP_CFM_REQ_EVT:
        esp_bt_gap_ssp_confirm_reply(param->cfm_req.bda, true);
        break;
    case ESP_BT_GAP_KEY_NOTIF_EVT:
        break;
    case ESP_BT_GAP_MODE_CHG_EVT:
        break;
    default:
        ESP_LOGI(TAG, "GAP EVENT ID:%d", event);
        break;
    }
}

void my_bt_a2d_cb(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param)
{
    switch (event)
    {
    case ESP_A2D_CONNECTION_STATE_EVT:
        if (param->conn_stat.state == ESP_A2D_CONNECTION_STATE_CONNECTED)
        {
            esp_bt_gap_set_scan_mode(ESP_BT_NON_CONNECTABLE, ESP_BT_NON_DISCOVERABLE);
            gpio_set_level(LED_GREEN_PIN, 1);
            gpio_set_level(LED_RED_PIN, 0);
        }
        if (param->conn_stat.state == ESP_A2D_CONNECTION_STATE_DISCONNECTED)
        {
            esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
            gpio_set_level(LED_GREEN_PIN, 0);
            gpio_set_level(LED_RED_PIN, 1);
        }
        break;
    case ESP_A2D_AUDIO_STATE_EVT:
        printf("Audio state event\n");

        break;
    case ESP_A2D_AUDIO_CFG_EVT:
        if (param->audio_cfg.mcc.type == ESP_A2D_MCT_SBC)
        {
            int sample_rate = 16000;
            char oct0 = param->audio_cfg.mcc.cie.sbc[0];
            if (oct0 & (0x01 << 6))
            {
                sample_rate = 32000;
            }
            else if (oct0 & (0x01 << 5))
            {
                sample_rate = 44100;
            }
            else if (oct0 & (0x01 << 4))
            {
                sample_rate = 48000;
            }
            i2s_set_clk(0, sample_rate, 16, 2);
        }

        break;
    case ESP_A2D_MEDIA_CTRL_ACK_EVT:
        break;
    case ESP_A2D_PROF_STATE_EVT:
        break;
    default:
        break;
    }
}

void my_bt_a2d_sink_data_cb(const uint8_t *buf, uint32_t len)
{
    size_t wb;
    i2s_write(0, buf, len, &wb, portMAX_DELAY);
}

esp_avrc_rn_evt_cap_mask_t rn_cap_mask = {0};
void my_bt_avrc_cb(esp_avrc_ct_cb_event_t event, esp_avrc_ct_cb_param_t *param)
{
    char *text;
    switch (event)
    {
    case ESP_AVRC_CT_CONNECTION_STATE_EVT:
        if (param->conn_stat.connected)
        {
            ESP_LOGI(TAG, "AVRCP connected");
            esp_avrc_ct_send_get_rn_capabilities_cmd(1);
            avrc_conn = 1;
            esp_avrc_ct_send_register_notification_cmd(1, ESP_AVRC_RN_PLAY_STATUS_CHANGE, 0);
        }
        else
        {
            ESP_LOGI(TAG, "AVRCP disconnected");
            rn_cap_mask.bits = 0;
            avrc_conn = 0;
        }
        break;
    case ESP_AVRC_CT_PASSTHROUGH_RSP_EVT:
        ESP_LOGI(TAG, "AVRCP passthrough response received");
        break;
    case ESP_AVRC_CT_CHANGE_NOTIFY_EVT:
        switch (param->change_ntf.event_id)
        {
        case ESP_AVRC_RN_VOLUME_CHANGE:
            ESP_LOGI(TAG, "AVRCP volume change notification received");
            break;
        case ESP_AVRC_RN_PLAY_STATUS_CHANGE:
            my_bt_speaker_state = param->change_ntf.event_parameter.playback;
            ESP_LOGI(TAG, "AVRCP playback status change: %d", my_bt_speaker_state);
            break;
        default:
            ESP_LOGI(TAG, "AVRCP unknown notification received");
            break;
        }
        break;
    case ESP_AVRC_CT_GET_RN_CAPABILITIES_RSP_EVT:
        rn_cap_mask.bits = param->get_rn_caps_rsp.evt_set.bits;
        ESP_LOGI(TAG, "AVRCP capabilities response received");
        break;
    case ESP_AVRC_CT_SET_ABSOLUTE_VOLUME_RSP_EVT:
        ESP_LOGI(TAG, "AVRCP absolute volume response received");
        break;
    case ESP_AVRC_CT_METADATA_RSP_EVT:
        ESP_LOGI(TAG, "AVRCP metadata response received");
        text = (char *)param->meta_rsp.attr_text;
        text[param->meta_rsp.attr_length] = '\0';
        ESP_LOGI(TAG, "metadata_rsp:%d, %s, %d", param->meta_rsp.attr_id, text, param->meta_rsp.attr_length);
        break;
    default:
        ESP_LOGI(TAG, "AVRCP default case");
        break;
    }
}

void avrc_event_handler(esp_avrc_rn_event_ids_t event_id)
{
    switch (event_id)
    {
    case ESP_AVRC_RN_PLAY_STATUS_CHANGE:
        printf("Event received: ESP_AVRC_RN_PLAY_STATUS_CHANGE\n");
        break;
    case ESP_AVRC_RN_TRACK_CHANGE:
        printf("Event received: ESP_AVRC_RN_TRACK_CHANGE\n");
        break;
    case ESP_AVRC_RN_TRACK_REACHED_END:
        printf("Event received: ESP_AVRC_RN_TRACK_REACHED_END\n");
        break;
    case ESP_AVRC_RN_TRACK_REACHED_START:
        printf("Event received: ESP_AVRC_RN_TRACK_REACHED_START\n");
        break;
    case ESP_AVRC_RN_PLAY_POS_CHANGED:
        printf("Event received: ESP_AVRC_RN_PLAY_POS_CHANGED\n");
        break;
    case ESP_AVRC_RN_BATTERY_STATUS_CHANGE:
        printf("Event received: ESP_AVRC_RN_BATTERY_STATUS_CHANGE\n");
        break;
    case ESP_AVRC_RN_SYSTEM_STATUS_CHANGE:
        printf("Event received: ESP_AVRC_RN_SYSTEM_STATUS_CHANGE\n");
        break;
    case ESP_AVRC_RN_APP_SETTING_CHANGE:
        printf("Event received: ESP_AVRC_RN_APP_SETTING_CHANGE\n");
        break;
    case ESP_AVRC_RN_NOW_PLAYING_CHANGE:
        printf("Event received: ESP_AVRC_RN_NOW_PLAYING_CHANGE\n");
        break;
    case ESP_AVRC_RN_AVAILABLE_PLAYERS_CHANGE:
        printf("Event received: ESP_AVRC_RN_AVAILABLE_PLAYERS_CHANGE\n");
        break;
    case ESP_AVRC_RN_ADDRESSED_PLAYER_CHANGE:
        printf("Event received: ESP_AVRC_RN_ADDRESSED_PLAYER_CHANGE\n");
        break;
    case ESP_AVRC_RN_UIDS_CHANGE:
        printf("Event received: ESP_AVRC_RN_UIDS_CHANGE\n");
        break;
    case ESP_AVRC_RN_VOLUME_CHANGE:
        printf("Event received: ESP_AVRC_RN_VOLUME_CHANGE\n");
        break;
    default:
        printf("Unknown event received\n");
        break;
    }
}

static void ct_play_forward(void *param)
{
    if (!ct_button_forward_press)
    {
        ct_button_forward_press = true;
    }
}
static void ct_play_play_pause(void *param)
{
    if (!ct_button_play_pause_press)
    {
        ct_button_play_pause_press = true;
    }
}
static void ct_play_backward(void *param)
{
    if (!ct_button_backward_press)
    {
        ct_button_backward_press = true;
    }
}
void my_bt_set_gpio_pins()
{
    gpio_set_direction(BUTTON_BACKWARD_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(BUTTON_FORWARD_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(BUTTON_PLAY_PAUSE_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(LED_GREEN_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_RED_PIN, GPIO_MODE_OUTPUT);

    gpio_set_intr_type(BUTTON_BACKWARD_PIN, GPIO_INTR_POSEDGE);
    gpio_set_intr_type(BUTTON_FORWARD_PIN, GPIO_INTR_POSEDGE);
    gpio_set_intr_type(BUTTON_PLAY_PAUSE_PIN, GPIO_INTR_POSEDGE);

    gpio_install_isr_service(ESP_INTR_FLAG_EDGE);
    gpio_isr_handler_add(BUTTON_FORWARD_PIN, ct_play_forward, 0);
    gpio_isr_handler_add(BUTTON_PLAY_PAUSE_PIN, ct_play_play_pause, 0);
    gpio_isr_handler_add(BUTTON_BACKWARD_PIN, ct_play_backward, 0);

    gpio_set_level(LED_GREEN_PIN, 0);
    gpio_set_level(LED_RED_PIN, 1);
}
void app_main(void)
{
    esp_err_t err;
    err = nvs_flash_init();
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "nvs init error");
        return;
    }

    my_bt_set_gpio_pins();

    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX, // Only TX
        .sample_rate = 44100,
        .bits_per_sample = 16,
        .channel_format = I2S_CHANNEL_MONO,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S, // MAX98357A I2S Philips
        .dma_buf_count = 6,
        .dma_buf_len = 60,
        .intr_alloc_flags = 0,     // Default interrupt priority
        .tx_desc_auto_clear = true // Auto clear tx descriptor on underflow
    };
    i2s_driver_install(0, &i2s_config, 0, NULL);

    i2s_pin_config_t pin_config = {
        .bck_io_num = 26,
        .ws_io_num = 25,
        .data_out_num = 22,
        .data_in_num = -1 // Not used
    };

    i2s_set_pin(0, &pin_config);

    esp_bt_controller_config_t bt_controller = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    err = esp_bt_controller_init(&bt_controller);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Bluetooth controller initialize failed: %d", err);
    }
    else
    {
        ESP_LOGI(TAG, "Bluetooth controller initialized");
    }

    err = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Bluetooth controller enable failed: %d", err);
    }
    else
    {
        ESP_LOGI(TAG, "Bluetooth controller enabled");
    }

    err = esp_bluedroid_init();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Bluedroid stack initialize failed: %d", err);
    }
    else
    {
        ESP_LOGI(TAG, "Bluedroid stack initialized");
    }

    err = esp_bluedroid_enable();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Bluedroid stack enable failed: %d", err);
    }
    else
    {
        ESP_LOGI(TAG, "Bluedroid stack enabled");
    }

    err = esp_avrc_ct_register_callback(my_bt_avrc_cb);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "AVRCP callback registration failed: %d", err);
    }
    else
    {
        ESP_LOGI(TAG, "AVRCP callback registered");
    }

    err = esp_avrc_ct_register_callback(avrc_event_handler);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "AVRCP target failed: %d", err);
    }
    else
    {
        ESP_LOGI(TAG, "AVRCP target initialized");
    }

    err = esp_avrc_ct_init();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "AVRCP initialization failed: %d", err);
    }
    else
    {
        ESP_LOGI(TAG, "AVRCP initialized");
    }

    err = esp_a2d_register_callback(my_bt_a2d_cb);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "A2DP callback registration failed: %d", err);
    }
    else
    {
        ESP_LOGI(TAG, "A2DP callback registered");
    }

    err = esp_a2d_sink_register_data_callback(my_bt_a2d_sink_data_cb);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "A2DP data callback registration failed: %d", err);
    }
    else
    {
        ESP_LOGI(TAG, "A2DP data callback registered");
    }

    err = esp_a2d_sink_init();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "A2DP sink initialization failed: %d", err);
    }
    else
    {
        ESP_LOGI(TAG, "A2DP sink initialized");
    }

    err = esp_bt_gap_register_callback(my_bt_gap_cb);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "GAP callback registration failed: %d", err);
    }
    else
    {
        ESP_LOGI(TAG, "GAP callback registered");
    }

    err = esp_bt_dev_set_device_name(bt_device_name);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Setting device name failed: %d", err);
    }
    else
    {
        ESP_LOGI(TAG, "Device name set");
    }

    esp_bt_io_cap_t io_cap = ESP_BT_IO_CAP_IO;
    err = esp_bt_gap_set_security_param(ESP_BT_SP_IOCAP_MODE, &io_cap, sizeof(io_cap));
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Setting security parameters failed: %d", err);
    }
    else
    {
        ESP_LOGI(TAG, "Security parameters set");
    }

    err = esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Setting scan mode failed: %d", err);
    }
    else
    {
        ESP_LOGI(TAG, "Scan mode set");
    }

    while (1)
    {
        if (avrc_conn)
        {
            // esp_avrc_ct_send_metadata_cmd(1, ESP_AVRC_MD_ATTR_TITLE);
            // esp_avrc_ct_send_metadata_cmd(1, ESP_AVRC_MD_ATTR_ARTIST);
            // esp_avrc_ct_send_metadata_cmd(1, ESP_AVRC_MD_ATTR_ALBUM);
            // esp_avrc_ct_send_metadata_cmd(1, ESP_AVRC_MD_ATTR_PLAYING_TIME);
            // esp_avrc_ct_send_metadata_cmd(1, ESP_AVRC_MD_ATTR_TRACK_NUM);
            vTaskDelay(500 / portTICK_PERIOD_MS);
            if (ct_button_backward_press)
            {
                esp_avrc_ct_send_passthrough_cmd(1, ESP_AVRC_PT_CMD_BACKWARD, ESP_AVRC_PT_CMD_STATE_PRESSED);
                esp_avrc_ct_send_metadata_cmd(1, ESP_AVRC_MD_ATTR_TITLE);
                vTaskDelay(300 / portTICK_PERIOD_MS);
                ct_button_backward_press = false;
            }
            if (ct_button_forward_press)
            {
                esp_avrc_ct_send_passthrough_cmd(1, ESP_AVRC_PT_CMD_FORWARD, ESP_AVRC_PT_CMD_STATE_PRESSED);
                esp_avrc_ct_send_metadata_cmd(1, ESP_AVRC_MD_ATTR_TITLE);
                vTaskDelay(300 / portTICK_PERIOD_MS);
                ct_button_forward_press = false;
            }
            if (ct_button_play_pause_press)
            {
                if (my_bt_speaker_state == ESP_AVRC_PLAYBACK_STOPPED || my_bt_speaker_state == ESP_AVRC_PLAYBACK_PAUSED)
                {
                    esp_avrc_ct_send_passthrough_cmd(1, ESP_AVRC_PT_CMD_PLAY, ESP_AVRC_PT_CMD_STATE_PRESSED);
                    my_bt_speaker_state = ESP_AVRC_PLAYBACK_PLAYING;
                }
                else if (my_bt_speaker_state == ESP_AVRC_PLAYBACK_PLAYING)
                {
                    esp_avrc_ct_send_passthrough_cmd(1, ESP_AVRC_PT_CMD_PAUSE, ESP_AVRC_PT_CMD_STATE_PRESSED);
                    my_bt_speaker_state = ESP_AVRC_PLAYBACK_PAUSED;
                }

                vTaskDelay(300 / portTICK_PERIOD_MS);
                ct_button_play_pause_press = false;
            }
        }
        else
        {
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
    }
}
