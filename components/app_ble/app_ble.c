#include <stdio.h>
#include "app_ble.h"

static const char * TAG = "app_ble";

static void app_ble_notify_task(void *arg)
{
    ESP_LOGI(TAG, "|||||||||||||||||||||||||");
    ESP_LOGI(TAG, "app_ble_notify_task start");
    ESP_LOGI(TAG, "^^^^^^^^^^^^^^^^^^^^^^^^^");
    uint8_t cnt = 0;
    while (1)
    {
        if (s_controller.ble_notify_enable)
        {
            uint8_t notify_data[4] = {0};
            notify_data[0] = 0xAA;
            notify_data[1] = 0xBB;
            notify_data[2] = 0xCC;
            notify_data[3] = cnt;
            esp_ble_dis_send_indication(notify_data, sizeof(notify_data));
            ESP_LOGI(TAG, "BLE notify data sentTTTTTTTTTTTTT");
            cnt++;
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_ble_task_init(void)
{
    esp_err_t ret;
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    ret = esp_bt_controller_init(&bt_cfg);
    if (ret)
    {
        ESP_LOGE(TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret)
    {
        ESP_LOGE(TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }
    
    if (!ble_ota_ringbuf_init(OTA_RINGBUF_SIZE))
    {
        ESP_LOGE(TAG, "%s init ringbuf fail", __func__);
        return;
    }

    if (esp_ble_ota_host_init() != ESP_OK)
    {
        ESP_LOGE(TAG, "%s initialize ble host fail: %s\n", __func__, esp_err_to_name(ret));
        return;
    }
    ESP_LOGE(TAG, "%s initialize ble host success\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++", __func__);
    esp_ble_ota_recv_fw_data_callback(ota_recv_fw_cb);
    xTaskCreatePinnedToCore(app_ble_notify_task, "app_ble_notify_task", 2048, NULL, 6, NULL, 0);
}
