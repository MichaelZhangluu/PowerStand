#pragma once
#include "driver/i2c_master.h"

/**
 * @brief Initialize the I2C master device
 * 
 * @param i2c_master_bus_handle Pointer to the I2C master bus handle initialzed
 * @return
 *      - ESP_OK on success
 *      - ESP_FAIL on failure
 */
esp_err_t i2c_master_device_init(i2c_master_bus_handle_t *i2c_master_bus_handle);