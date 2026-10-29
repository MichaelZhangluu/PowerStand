idf_component_register(SRCS "i2c_master_device.c"
                    INCLUDE_DIRS "include"
                    REQUIRES    hardware_config
                                esp_driver_i2c)