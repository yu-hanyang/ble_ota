#include <stdio.h>
#include <string.h>
#include "app_controller.h"

#define BUILD_DAY ((__DATE__ [4] == ' ' ? 0 : __DATE__ [4] - '0') * 10 \
                  + (__DATE__ [5] - '0'))

static const char * TAG = "app_controller";
controller_t s_controller = { 0 };
static bool s_controller_build_date_init(void)
{
    /* 软件版本号初始化 */
    s_controller.software_version[0] = SOFTWARE_MAJOR_VERSION_NUMBER;
    s_controller.software_version[1] = SOFTWARE_MINOR_VERSION_NUMBER;
    s_controller.software_version[2] = SOFTWARE_REVISION_VERSION_NUMBER;

    /* 编译日期拼接 */
    s_controller.build_date = 0;
    //年
    s_controller.build_date += __DATE__[7] - '0';
    s_controller.build_date *= 10;
    s_controller.build_date += __DATE__[8] - '0';
    s_controller.build_date *= 10;
    s_controller.build_date += __DATE__[9] - '0';
    s_controller.build_date *= 10;
    s_controller.build_date += __DATE__[10] - '0';
    //月
    s_controller.build_date *= 10;
    s_controller.build_date *= 10;
    char month_abbr[4];
    strncpy(month_abbr, __DATE__, 3); // 提取前3字符（如"Jun"）
    month_abbr[3] = '\0';         // 添加字符串结束符

    const char * months[] =
    { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct",
            "Nov", "Dec" };

    uint16_t month_num = 0;
    for (int i = 0; i < 12; i++)
    {
        if (strcmp(month_abbr, months[i]) == 0)
        {
            month_num = i + 1; // 匹配成功，返回1-12
            break;
        }
    }
    s_controller.build_date += month_num;

    //日
    s_controller.build_date *= 100;
    s_controller.build_date += BUILD_DAY;

    /* 编译时间拼接 */
    s_controller.build_time = 0;
    //时
    s_controller.build_time += __TIME__[0] - '0';
    s_controller.build_time *= 10;
    s_controller.build_time += __TIME__[1] - '0';
    s_controller.build_time *= 10;
    //分
    s_controller.build_time += __TIME__[3] - '0';
    s_controller.build_time *= 10;
    s_controller.build_time += __TIME__[4] - '0';
    s_controller.build_time *= 10;
    //秒
    s_controller.build_time += __TIME__[6] - '0';
    s_controller.build_time *= 10;
    s_controller.build_time += __TIME__[7] - '0';
    return true;
}

static void app_controller_task(void * pvParameter)
{
    s_controller_build_date_init();
    char build_info[128];
    snprintf(build_info, sizeof(build_info),
        "build_date:%lu, build_time:%lu, software_version:%d.%d.%d",
        s_controller.build_date, s_controller.build_time,
        s_controller.software_version[0], s_controller.software_version[1],
        s_controller.software_version[2]);
    while (1)
    {
        ESP_LOGI(TAG, "%s", build_info);
        ESP_LOGI(TAG, "product_serial_number:%lu",
            s_controller.userCfg.product_serial_number);
        
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_controller_task_init(void)
{
    xTaskCreatePinnedToCore(app_controller_task, "app_controller_task", 2048, NULL, 5, NULL, 0);
}