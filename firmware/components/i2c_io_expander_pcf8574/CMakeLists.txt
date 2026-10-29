idf_component_register(SRCS "i2c_io_expander_pcf8574.c"
                    INCLUDE_DIRS "include"
                    REQUIRES    i2c_master_device
                                esp_driver_i2c)
