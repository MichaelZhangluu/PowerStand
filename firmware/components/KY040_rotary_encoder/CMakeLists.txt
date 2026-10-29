idf_component_register(SRCS "KY040_rotary_encoder.c"
                    INCLUDE_DIRS "include"
                    REQUIRES hardware_config
                             esp_driver_gpio
                             esp_driver_pcnt
                             esp_timer
                             lvgl)
