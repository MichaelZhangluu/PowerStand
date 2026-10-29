#include <stdio.h>

#include "driver/i2c_types.h"
#include "driver/i2c_master.h"
#include "hardware_config.h"
#include "i2c_master_device.h"

esp_err_t i2c_master_device_init(i2c_master_bus_handle_t *i2c_master_bus_handle)
{
    esp_err_t status = ESP_OK;

    // I2C Master device configuration
    i2c_master_bus_config_t i2c_master_bus_config = {
        .i2c_port = I2C_MASTER_DEVICE_PORT_NUM,
        .sda_io_num = I2C_MASTER_DEVICE_SDA_GPIO_NUM,
        .scl_io_num = I2C_MASTER_DEVICE_SCL_GPIO_NUM,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 1,
        .trans_queue_depth = 0,
        .flags = {
            .enable_internal_pullup = true,
            .allow_pd = 0,
        },
    };

    // Initialize the I2C master bus
    status = i2c_new_master_bus(&i2c_master_bus_config, i2c_master_bus_handle);

    return status;
}
