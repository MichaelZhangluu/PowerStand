idf_component_register(SRCS "lcd_msp4021.c"
                    INCLUDE_DIRS "include"
                    REQUIRES    general_types
                                hardware_config
                                esp_timer
                                esp_driver_ledc
                                esp_lcd
                                esp_lcd_st7796
                                esp_lcd_touch
                                esp_lcd_touch_xpt2046
                                lvgl
                                fatfs
                    )
