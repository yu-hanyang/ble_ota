idf_component_register(SRCS "app_ble.c"
                    INCLUDE_DIRS "include"
                    REQUIRES "app_ota" "app_nvs" "app_ble" "ble_ota" "esp_ringbuf" "app_update")
