idf_component_register(SRCS "app_ota.c"
                    INCLUDE_DIRS "include"
                    REQUIRES "ble_ota" "esp_ringbuf" "app_update")
