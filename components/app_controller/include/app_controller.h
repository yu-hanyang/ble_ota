#ifndef APP_CONTROLLER_H
#define APP_CONTROLLER_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define SOFTWARE_MAJOR_VERSION_NUMBER       (0)//软件主版本号
#define SOFTWARE_MINOR_VERSION_NUMBER       (0)//软件次版本号
#define SOFTWARE_REVISION_VERSION_NUMBER    (2)//软件修订版本号

typedef struct{

    
    uint32_t                    product_serial_number;//产品序列号
    
}user_cfg_t;

typedef struct
{


    /*控制台参数，需要记录在EEPROM页1中 */
    user_cfg_t  userCfg;


    uint16_t software_version[3];//软件版本号
    uint32_t build_date;//编译日期 格式为年月日，如2025 年 3 月 10 日发布，显示为20250310
    uint32_t build_time;//编译时间 格式为hhmmss
}controller_t;

extern controller_t s_controller;

void app_controller_param_clear(void);

void app_userCfg_init(void);

void app_controller_init(void);

void app_controller_task_init(void);

#endif //APP_CONTROLLER_H

