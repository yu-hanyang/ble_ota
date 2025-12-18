/*
 * SPDX-FileCopyrightText: 2019-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/ringbuf.h"

#include "esp_log.h"
#include "esp_ota_ops.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "ble_ota.h"
#include "freertos/semphr.h"

#define OTA_RINGBUF_SIZE                    8192
#define OTA_TASK_SIZE                       8192

static const char * TAG = "ESP_BLE_OTA";
static esp_ota_handle_t out_handle;
SemaphoreHandle_t notify_sem;




static RingbufHandle_t s_ringbuf = NULL;

bool
ble_ota_ringbuf_init(uint32_t ringbuf_size)
{
    s_ringbuf = xRingbufferCreate(ringbuf_size, RINGBUF_TYPE_BYTEBUF);
    if (s_ringbuf == NULL)
    {
        return false;
    }

    return true;
}

size_t
write_to_ringbuf(const uint8_t * data, size_t size)
{
    BaseType_t done = xRingbufferSend(s_ringbuf, (void *)data, size, (TickType_t)portMAX_DELAY);
    if (done)
    {
        return size;
    }
    else
    {
        return 0;
    }
}

void
ota_task(void * arg)
{
    esp_partition_t * partition_ptr = NULL;
    esp_partition_t partition;
    const esp_partition_t * next_partition = NULL;

    uint32_t recv_len = 0;
    uint8_t * data = NULL;
    size_t item_size = 0;
    ESP_LOGI(TAG, "ota_task start");

    notify_sem = xSemaphoreCreateCounting(100, 0);
    xSemaphoreGive(notify_sem);
    partition_ptr = (esp_partition_t *)esp_ota_get_boot_partition();
    if (partition_ptr == NULL)
    {
        ESP_LOGE(TAG, "boot partition NULL!\r\n");
        goto OTA_ERROR;
    }
    if (partition_ptr->type != ESP_PARTITION_TYPE_APP)
    {
        ESP_LOGE(TAG, "esp_current_partition->type != ESP_PARTITION_TYPE_APP\r\n");
        goto OTA_ERROR;
    }

    if (partition_ptr->subtype == ESP_PARTITION_SUBTYPE_APP_FACTORY)
    {
        partition.subtype = ESP_PARTITION_SUBTYPE_APP_OTA_0;
    }
    else
    {
        next_partition = esp_ota_get_next_update_partition(partition_ptr);
        if (next_partition)
        {
            partition.subtype = next_partition->subtype;
        }
        else
        {
            partition.subtype = ESP_PARTITION_SUBTYPE_APP_OTA_0;
        }
    }
    partition.type = ESP_PARTITION_TYPE_APP;

    partition_ptr = (esp_partition_t *)esp_partition_find_first(partition.type, partition.subtype, NULL);
    if (partition_ptr == NULL)
    {
        ESP_LOGE(TAG, "partition NULL!\r\n");
        goto OTA_ERROR;
    }

    memcpy(&partition, partition_ptr, sizeof(esp_partition_t));
    if (esp_ota_begin(&partition, OTA_SIZE_UNKNOWN, &out_handle) != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_ota_begin failed!\r\n");
        goto OTA_ERROR;
    }

    ESP_LOGI(TAG, "wait for data from ringbuf! fw_len = %u", esp_ble_ota_get_fw_length());
    /*deal with all receive packet*/
    for (;;)
    {
        data = (uint8_t *)xRingbufferReceive(s_ringbuf, &item_size, (TickType_t)portMAX_DELAY);

        xSemaphoreTake(notify_sem, portMAX_DELAY);


        ESP_LOGI(TAG, "recv: %u, recv_total:%"PRIu32"\n", item_size, recv_len + item_size);

        if (item_size != 0)
        {

            if (esp_ota_write(out_handle, (const void *)data, item_size) != ESP_OK)
            {
                ESP_LOGE(TAG, "esp_ota_write failed!\r\n");
                goto OTA_ERROR;
            }

            recv_len += item_size;
            vRingbufferReturnItem(s_ringbuf, (void *)data);

            if (recv_len >= esp_ble_ota_get_fw_length())
            {
                xSemaphoreGive(notify_sem);
                break;
            }
        }
        xSemaphoreGive(notify_sem);
    }



    if (esp_ota_end(out_handle) != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_ota_end failed!\r\n");
        goto OTA_ERROR;
    }


    if (esp_ota_set_boot_partition(&partition) != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed!\r\n");
        goto OTA_ERROR;
    }


    vSemaphoreDelete(notify_sem);
    esp_restart();

OTA_ERROR:
    ESP_LOGE(TAG, "OTA failed");
    vTaskDelete(NULL);
}

void
ota_recv_fw_cb(uint8_t * buf, uint32_t length)
{
    write_to_ringbuf(buf, length);
}

static void
ota_task_init(void)
{
    xTaskCreate(&ota_task, "ota_task", OTA_TASK_SIZE, NULL, 5, NULL);
    return;
}

static void print_task(void * arg)
{
    int cnt = 0;
    while (1)
    {
        ESP_LOGI(TAG, "Free heap size: %d", cnt++);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

static void print_task_init(void)
{
    xTaskCreate(&print_task, "print_task", 2048, NULL, 5, NULL);
}

void app_ble_ota_task_init(void)
{
    esp_err_t ret;
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    // Initialize NVS
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    if (!ble_ota_ringbuf_init(OTA_RINGBUF_SIZE))
    {
        ESP_LOGE(TAG, "%s init ringbuf fail", __func__);
        return;
    }


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
    if (esp_ble_ota_host_init() != ESP_OK)
    {
        ESP_LOGE(TAG, "%s initialize ble host fail: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    esp_ble_ota_recv_fw_data_callback(ota_recv_fw_cb);

    ota_task_init();
}

void
app_main(void)
{
    app_ble_ota_task_init();
    print_task_init();
}
