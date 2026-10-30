/**
 * @file i2c_io_expander_pcf8574.h
 * @brief PCF8574 I2C GPIO Expander Driver.
 *
 * ESP-IDF 6.1 native implementation using the
 * I2C Master Bus API.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Includes
 *****************************************************************************/

#include <stdbool.h>
#include <stdint.h>

#include "driver/i2c_master.h"
#include "esp_err.h"

/******************************************************************************
 * Public Types
 *****************************************************************************/

/**
 * @brief PCF8574 device handle.
 */
typedef struct i2c_io_expander_pcf8574_t *i2c_io_expander_pcf8574_handle_t;

/**
 * @brief PCF8574 configuration structure.
 */
typedef struct
{
    /**
     * @brief 7-bit I2C device address.
     */
    uint8_t address;

} i2c_io_expander_pcf8574_config_t;

/**
 * @brief PCF8574 GPIO pin identifiers.
 */
typedef enum
{
    I2C_IO_EXPANDER_PCF8574_PIN_P0 = 0,
    I2C_IO_EXPANDER_PCF8574_PIN_P1,
    I2C_IO_EXPANDER_PCF8574_PIN_P2,
    I2C_IO_EXPANDER_PCF8574_PIN_P3,
    I2C_IO_EXPANDER_PCF8574_PIN_P4,
    I2C_IO_EXPANDER_PCF8574_PIN_P5,
    I2C_IO_EXPANDER_PCF8574_PIN_P6,
    I2C_IO_EXPANDER_PCF8574_PIN_P7
} i2c_io_expander_pcf8574_pin_t;

/******************************************************************************
 * Public Function Prototypes
 *****************************************************************************/

/**
 * @brief Create a PCF8574 device instance.
 *
 * @param[in] bus_handle
 * I2C master bus handle.
 *
 * @param[in] config
 * Device configuration.
 *
 * @param[out] ret_handle
 * Returned device handle.
 *
 * @return esp_err_t
 */
esp_err_t i2c_io_expander_pcf8574_new(i2c_master_bus_handle_t bus_handle, const i2c_io_expander_pcf8574_config_t *config, i2c_io_expander_pcf8574_handle_t *ret_handle);

/**
 * @brief Delete a device instance.
 *
 * @param[in] handle
 * Device handle.
 *
 * @return esp_err_t
 */
esp_err_t i2c_io_expander_pcf8574_del(
    i2c_io_expander_pcf8574_handle_t handle);

/**
 * @brief Read the entire port.
 *
 * @param[in] handle
 * Device handle.
 *
 * @param[out] value
 * Returned port value.
 *
 * @return esp_err_t
 */
esp_err_t i2c_io_expander_pcf8574_read_port(
    i2c_io_expander_pcf8574_handle_t handle,
    uint8_t *value);

/**
 * @brief Write the entire port.
 *
 * @param[in] handle
 * Device handle.
 *
 * @param[in] value
 * Port value.
 *
 * @return esp_err_t
 */
esp_err_t i2c_io_expander_pcf8574_write_port(
    i2c_io_expander_pcf8574_handle_t handle,
    uint8_t value);

/**
 * @brief Set a single GPIO pin state.
 *
 * @param[in] handle
 * Device handle.
 *
 * @param[in] pin
 * GPIO pin identifier.
 *
 * @param[in] level
 * Logical level (0 or non-zero).
 *
 * @return esp_err_t
 */
esp_err_t i2c_io_expander_pcf8574_set_pin(
    i2c_io_expander_pcf8574_handle_t handle,
    i2c_io_expander_pcf8574_pin_t pin,
    uint32_t level);

/**
 * @brief Read a single GPIO pin state.
 *
 * @param[in] handle
 * Device handle.
 *
 * @param[in] pin
 * GPIO pin identifier.
 *
 * @param[out] level
 * Returned GPIO level.
 *
 * @return esp_err_t
 */
esp_err_t i2c_io_expander_pcf8574_get_pin(
    i2c_io_expander_pcf8574_handle_t handle,
    i2c_io_expander_pcf8574_pin_t pin,
    uint32_t *level);

/**
 * @brief Update selected GPIO bits.
 *
 * @param[in] handle
 * Device handle.
 *
 * @param[in] mask
 * Bit mask.
 *
 * @param[in] value
 * New value bits.
 *
 * @return esp_err_t
 */
esp_err_t i2c_io_expander_pcf8574_update_bits(
    i2c_io_expander_pcf8574_handle_t handle,
    uint8_t mask,
    uint8_t value);

#ifdef __cplusplus
}
#endif