#ifndef APP_BLE_H
#define APP_BLE_H

#include "app_ota.h"
#include "app_nvs.h"
#include "nvs_flash.h"

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/ringbuf.h"

#include "esp_log.h"
#include "esp_ota_ops.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "ble_ota.h"
#include "freertos/semphr.h"

void app_ble_task_init(void);

#endif // APP_BLE_H
