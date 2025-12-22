#ifndef APP_NVS_H
#define APP_NVS_H

#include "nvs_flash.h"
#include "esp_log.h"
#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "app_controller.h"

extern bool app_nvs_update_flag;

void app_nvs_init(void);


#endif