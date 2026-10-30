/**
 * @file i2c_lcd_hd44780.h
 * @brief HD44780 LCD controller driver using PCF8574 I2C backpack.
 *
 * This driver provides low-level access to an HD44780-compatible
 * LCD controller connected through a PCF8574 I/O expander.
 *
 * The driver exposes HD44780 controller functionality while hiding
 * HD44780 command encoding details.
 *
 * This driver intentionally does not provide:
 * - Display geometry management
 * - Row/column positioning
 * - String handling
 * - Text formatting
 * - Menu systems
 * - User interface functionality
 *
 * @note
 * This driver is not thread-safe.
 *
 * Access to a driver instance shall be serialized
 * by the owning task.
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
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
 * @brief LCD driver handle.
 */
typedef struct i2c_lcd_hd44780_t
    *i2c_lcd_hd44780_handle_t;

/**
 * @brief Entry mode cursor direction.
 */
typedef enum
{
    /**
     * @brief Increment DDRAM address after data write.
     */
    I2C_LCD_HD44780_ENTRY_INCREMENT = 0,

    /**
     * @brief Decrement DDRAM address after data write.
     */
    I2C_LCD_HD44780_ENTRY_DECREMENT

} i2c_lcd_hd44780_entry_direction_t;

/**
 * @brief Driver configuration.
 */
typedef struct
{
    /**
     * @brief I2C master bus handle.
     */
    i2c_master_bus_handle_t bus_handle;

    /**
     * @brief PCF8574 I2C device address.
     */
    uint16_t device_address;

    /**
     * @brief Initial backlight state.
     */
    bool backlight_enabled;

} i2c_lcd_hd44780_config_t;

/******************************************************************************
 * Public Function Prototypes
 *****************************************************************************/

/**
 * @brief Create LCD controller instance.
 *
 * @param[in] config
 * Driver configuration.
 *
 * @param[out] handle
 * Returned driver handle.
 *
 * @return esp_err_t
 */
esp_err_t i2c_lcd_hd44780_create(
    const i2c_lcd_hd44780_config_t *config,
    i2c_lcd_hd44780_handle_t *handle);

/**
 * @brief Delete LCD controller instance.
 *
 * @param[in] handle
 * Driver handle.
 *
 * @return esp_err_t
 */
esp_err_t i2c_lcd_hd44780_delete(
    i2c_lcd_hd44780_handle_t handle);

/**
 * @brief Clear display.
 *
 * HD44780 command:
 * Clear Display (0x01)
 *
 * @param[in] handle
 * Driver handle.
 *
 * @return esp_err_t
 */
esp_err_t i2c_lcd_hd44780_clear(
    i2c_lcd_hd44780_handle_t handle);

/**
 * @brief Return cursor to home position.
 *
 * HD44780 command:
 * Return Home (0x02)
 *
 * @param[in] handle
 * Driver handle.
 *
 * @return esp_err_t
 */
esp_err_t i2c_lcd_hd44780_home(
    i2c_lcd_hd44780_handle_t handle);

/**
 * @brief Set DDRAM address.
 *
 * @param[in] handle
 * Driver handle.
 *
 * @param[in] address
 * DDRAM address (0x00-0x7F).
 *
 * @return esp_err_t
 */
esp_err_t i2c_lcd_hd44780_set_ddram_address(
    i2c_lcd_hd44780_handle_t handle,
    uint8_t address);

/**
 * @brief Set CGRAM address.
 *
 * @param[in] handle
 * Driver handle.
 *
 * @param[in] address
 * CGRAM address (0x00-0x3F).
 *
 * @return esp_err_t
 */
esp_err_t i2c_lcd_hd44780_set_cgram_address(
    i2c_lcd_hd44780_handle_t handle,
    uint8_t address);

/**
 * @brief Write a byte to the HD44780 data register.
 *
 * This API is used for:
 * - Writing character codes into DDRAM
 * - Writing custom character references
 * - Writing bitmap rows into CGRAM
 *
 * @param[in] handle
 * Driver handle.
 *
 * @param[in] data
 * Data byte.
 *
 * @return esp_err_t
 */
esp_err_t i2c_lcd_hd44780_write_data(
    i2c_lcd_hd44780_handle_t handle,
    uint8_t data);

/**
 * @brief Configure entry mode.
 *
 * @param[in] handle
 * Driver handle.
 *
 * @param[in] direction
 * Entry mode direction.
 *
 * @param[in] display_shift_enabled
 * Display shift enable state.
 *
 * @return esp_err_t
 */
esp_err_t i2c_lcd_hd44780_set_entry_mode(
    i2c_lcd_hd44780_handle_t handle,
    i2c_lcd_hd44780_entry_direction_t direction,
    bool display_shift_enabled);

/**
 * @brief Enable or disable display.
 *
 * @param[in] handle
 * Driver handle.
 *
 * @param[in] enabled
 * Display enable state.
 *
 * @return esp_err_t
 */
esp_err_t i2c_lcd_hd44780_set_display_enabled(
    i2c_lcd_hd44780_handle_t handle,
    bool enabled);

/**
 * @brief Enable or disable cursor.
 *
 * @param[in] handle
 * Driver handle.
 *
 * @param[in] enabled
 * Cursor enable state.
 *
 * @return esp_err_t
 */
esp_err_t i2c_lcd_hd44780_set_cursor_enabled(
    i2c_lcd_hd44780_handle_t handle,
    bool enabled);

/**
 * @brief Enable or disable cursor blink.
 *
 * @param[in] handle
 * Driver handle.
 *
 * @param[in] enabled
 * Blink enable state.
 *
 * @return esp_err_t
 */
esp_err_t i2c_lcd_hd44780_set_blink_enabled(
    i2c_lcd_hd44780_handle_t handle,
    bool enabled);

/**
 * @brief Enable or disable LCD backlight.
 *
 * @param[in] handle
 * Driver handle.
 *
 * @param[in] enabled
 * Backlight enable state.
 *
 * @return esp_err_t
 */
esp_err_t i2c_lcd_hd44780_set_backlight_enabled(
    i2c_lcd_hd44780_handle_t handle,
    bool enabled);

#ifdef __cplusplus
}
#endif