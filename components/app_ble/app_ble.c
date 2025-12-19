#include <stdio.h>
#include "app_ble.h"

static const char *TAG = "app_ble";
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
    esp_ble_ota_recv_fw_data_callback(ota_recv_fw_cb);
}
