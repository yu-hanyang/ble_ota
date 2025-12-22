#include <stdio.h>
#include "app_nvs.h"


#define USER_PARTITION_TYPE   0x40
#define USER_PARTITION_SUBTYPE 0x01

#define EEP_MAX_SIZE            0x4000

static const esp_partition_t *pt_partion = NULL;
bool app_nvs_update_flag = false;

static const char * TAG = "app_nvs";

SemaphoreHandle_t nvs_semaphore;

static void app_nvs_erase_partition(void)
{
    esp_partition_erase_range(pt_partion, 0, EEP_MAX_SIZE);
}

static void app_nvs_write_usercfg(void)
{
    app_nvs_erase_partition();
    char buffer[sizeof(user_cfg_t)] = { 0 };
    memcpy(buffer, &s_controller.userCfg, sizeof(user_cfg_t));
    esp_partition_write(pt_partion, 0,
        buffer, sizeof(user_cfg_t));
}

static void app_nvs_read_usercfg(void)
{
    char buffer[sizeof(user_cfg_t)] = {0};
    esp_partition_read(pt_partion, 0, buffer, sizeof(user_cfg_t));
    memcpy(&s_controller.userCfg, buffer, sizeof(user_cfg_t));
}

static void app_nvs_update_usercfg(void)
{
    xSemaphoreTake(nvs_semaphore, portMAX_DELAY); //加锁
    app_nvs_write_usercfg();
    app_nvs_read_usercfg();
    xSemaphoreGive(nvs_semaphore);
}

static void app_nvs_task(void * pvParameter)
{
    ESP_LOGI(TAG, "Initializing NVS");
    esp_err_t ret;
    // Initialize NVS
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    pt_partion = esp_partition_find_first(USER_PARTITION_TYPE, USER_PARTITION_SUBTYPE, NULL);

    if (pt_partion == NULL)
    {
        ESP_LOGE(TAG, "Can't find partition");
        return;
    }

    nvs_semaphore = xSemaphoreCreateMutex();

    app_nvs_read_usercfg();//初始化的时候读取usercfg
    while (1)
    {
        if (app_nvs_update_flag == true)
        {
            app_nvs_update_usercfg();
            app_nvs_update_flag = false;
        }
        
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        
        ESP_LOGI(TAG, "-----------------------------------Serial number: %lu", s_controller.userCfg.product_serial_number);
    }
    
    
}

void app_nvs_init(void)
{
    xTaskCreatePinnedToCore(app_nvs_task, "app_nvs_task", 2048, NULL, 5, NULL, 0);
}