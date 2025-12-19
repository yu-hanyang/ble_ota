
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/ringbuf.h"

#include "esp_log.h"
#include "esp_ota_ops.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "ble_ota.h"
#include "freertos/semphr.h"

#include "app_ota.h"
#include "app_nvs.h"
#include "app_ble.h"



static const char * TAG = "main";

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


void
app_main(void)
{
    app_nvs_init();
    app_ble_task_init();
    app_ota_task_init();//使用该任务前蓝牙和flash得先启动
    print_task_init();
}
