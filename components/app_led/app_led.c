
#include "app_Led.h"

#define BLINK_GPIO CONFIG_BLINK_GPIO
const char * TAG = "app_Led";
static uint8_t s_led_state = 0;
static void blink_led(uint8_t s_led_state)
{
    /* Set the GPIO level according to the state (LOW or HIGH)*/
    gpio_set_level(BLINK_GPIO, s_led_state);
}

static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink GPIO LED!");
    gpio_reset_pin(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}


static void button_single_click_cb(void *arg,void *usr_data)
{
    s_controller.userCfg.product_serial_number = 0;
    ESP_LOGI(TAG, "BUTTON_SINGLE_CLICK");
}


static void app_led_task(void * pvParameter)
{
    // create gpio button
    const button_config_t btn_cfg = {0};
    const button_gpio_config_t btn_gpio_cfg = {
        .gpio_num = 11,
        .active_level = 0,
    };
    button_handle_t gpio_btn = NULL;
    //esp_err_t ret = iot_button_new_gpio_device(&btn_cfg, &btn_gpio_cfg, &gpio_btn);
    iot_button_new_gpio_device(&btn_cfg, &btn_gpio_cfg, &gpio_btn);
    if (NULL == gpio_btn)
    {
        ESP_LOGE(TAG, "Button create failed");
    }

    /* Configure the peripheral according to the LED type */
    configure_led();
    iot_button_register_cb(gpio_btn, BUTTON_SINGLE_CLICK, NULL, button_single_click_cb,NULL);
    while (1) {
        ESP_LOGI(TAG, "Turning the LED %s!", s_led_state == true ? "ON" : "OFF");
        blink_led(s_led_state);
        /* Toggle the LED state */
        s_led_state = !s_led_state;
        s_controller.userCfg.product_serial_number++;
        update_flag = true;
        ESP_LOGI(TAG, "Serial number: %lu", s_controller.userCfg.product_serial_number);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_led_task_init(void)
{
    xTaskCreatePinnedToCore(app_led_task, "app_led_task", 2048, NULL, 5, NULL, 0);
}