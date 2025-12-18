#ifndef APP_OTA_H
#define APP_OTA_H

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

void app_ble_ota_task_init();
#endif
