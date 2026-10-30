idf_component_register(SRCS "i2c_lcd_hd44780.c"
                    INCLUDE_DIRS "include"
                    REQUIRES    hardware_config
                                i2c_io_expander_pcf8574
                    )
