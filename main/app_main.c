
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

#include "driver/gpio.h"
#include "app_Led.h"
#include "sdkconfig.h"
#include "app_controller.h"



static const char * TAG = "main";


void app_main(void)
{
    ESP_LOGI(TAG, "++++++++++++++++++++++++++++app_main start+++++++++++++++++++++");
    app_controller_task_init();

    app_nvs_init();
    app_ble_task_init();
    app_ota_task_init();//使用该任务前蓝牙和flash得先启动


    app_led_task_init();
}
