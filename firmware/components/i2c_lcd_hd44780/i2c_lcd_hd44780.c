/* i2c_lcd_hd44780.c */

#include <stdlib.h>

#include "esp_rom_sys.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "i2c_lcd_hd44780.h"
#include "i2c_io_expander_pcf8574.h"

/******************************************************************************
 * Private Constants
 *****************************************************************************/

#define LCD_CMD_CLEAR_DISPLAY        (0x01U)
#define LCD_CMD_RETURN_HOME          (0x02U)

#define LCD_CMD_ENTRY_MODE_SET       (0x04U)
#define LCD_CMD_DISPLAY_CONTROL      (0x08U)

#define LCD_CMD_SET_CGRAM_ADDRESS    (0x40U)
#define LCD_CMD_SET_DDRAM_ADDRESS    (0x80U)

#define LCD_ENTRY_SHIFT_BIT          (0x01U)
#define LCD_ENTRY_INCREMENT_BIT      (0x02U)

#define LCD_DISPLAY_BLINK_BIT        (0x01U)
#define LCD_DISPLAY_CURSOR_BIT       (0x02U)
#define LCD_DISPLAY_ENABLE_BIT       (0x04U)

#define LCD_FUNCTION_SET             (0x28U)

#define LCD_DELAY_SHORT_US           (50U)
#define LCD_DELAY_LONG_US            (2000U)

#define LCD_POWER_UP_DELAY_MS        (50U)

#define LCD_PIN_RS                   (0U)
#define LCD_PIN_RW                   (1U)
#define LCD_PIN_EN                   (2U)
#define LCD_PIN_BACKLIGHT            (3U)

#define LCD_DATA_SHIFT               (4U)

/******************************************************************************
 * Private Types
 *****************************************************************************/

struct i2c_lcd_hd44780_t
{
    i2c_io_expander_pcf8574_handle_t io_expander_handle;

    uint8_t port_shadow;

    bool display_enabled;
    bool cursor_enabled;
    bool blink_enabled;

    bool backlight_enabled;

    i2c_lcd_hd44780_entry_direction_t entry_direction;
    bool display_shift_enabled;
};

/******************************************************************************
 * Private Function Prototypes
 *****************************************************************************/

static esp_err_t lcd_write_expander(
    i2c_lcd_hd44780_handle_t handle,
    uint8_t value);

static esp_err_t lcd_pulse_enable(
    i2c_lcd_hd44780_handle_t handle,
    uint8_t value);

static esp_err_t lcd_write_nibble(
    i2c_lcd_hd44780_handle_t handle,
    uint8_t nibble,
    bool rs);

static esp_err_t lcd_write_byte(
    i2c_lcd_hd44780_handle_t handle,
    uint8_t value,
    bool rs);

static esp_err_t lcd_write_command(
    i2c_lcd_hd44780_handle_t handle,
    uint8_t command);

static esp_err_t lcd_update_display_control(
    i2c_lcd_hd44780_handle_t handle);

static esp_err_t lcd_update_entry_mode(
    i2c_lcd_hd44780_handle_t handle);

static esp_err_t lcd_initialize(
    i2c_lcd_hd44780_handle_t handle);

/******************************************************************************
 * Private Functions
 *****************************************************************************/

static esp_err_t lcd_write_expander(
    i2c_lcd_hd44780_handle_t handle,
    uint8_t value)
{
    esp_err_t status;

    status = ESP_OK;

    if (handle == NULL)
    {
        status = ESP_ERR_INVALID_ARG;
    }

    if (status == ESP_OK)
    {
        handle->port_shadow = value;

        status =
            i2c_io_expander_pcf8574_write_port(
                handle->io_expander_handle,
                value);
    }

    return status;
}

static esp_err_t lcd_pulse_enable(
    i2c_lcd_hd44780_handle_t handle,
    uint8_t value)
{
    esp_err_t status;

    status = ESP_OK;

    if (status == ESP_OK)
    {
        status = lcd_write_expander(
            handle,
            (uint8_t)(value | (1U << LCD_PIN_EN)));
    }

    esp_rom_delay_us(1U);

    if (status == ESP_OK)
    {
        status = lcd_write_expander(
            handle,
            (uint8_t)(value & ~(1U << LCD_PIN_EN)));
    }

    esp_rom_delay_us(LCD_DELAY_SHORT_US);

    return status;
}

static esp_err_t lcd_write_nibble(
    i2c_lcd_hd44780_handle_t handle,
    uint8_t nibble,
    bool rs)
{
    esp_err_t status;
    uint8_t port_value;

    status = ESP_OK;

    port_value = handle->port_shadow;

    port_value &= (uint8_t)0x0FU;

    port_value |=
        (uint8_t)((nibble & 0x0FU) << LCD_DATA_SHIFT);

    if (rs)
    {
        port_value |=
            (uint8_t)(1U << LCD_PIN_RS);
    }
    else
    {
        port_value &=
            (uint8_t)(~(1U << LCD_PIN_RS));
    }

    port_value &=
        (uint8_t)(~(1U << LCD_PIN_RW));

    status = lcd_pulse_enable(
        handle,
        port_value);

    return status;
}

static esp_err_t lcd_write_byte(
    i2c_lcd_hd44780_handle_t handle,
    uint8_t value,
    bool rs)
{
    esp_err_t status;

    status = ESP_OK;

    if (status == ESP_OK)
    {
        status = lcd_write_nibble(
            handle,
            (uint8_t)((value >> 4U) & 0x0FU),
            rs);
    }

    if (status == ESP_OK)
    {
        status = lcd_write_nibble(
            handle,
            (uint8_t)(value & 0x0FU),
            rs);
    }

    return status;
}

static esp_err_t lcd_write_command(
    i2c_lcd_hd44780_handle_t handle,
    uint8_t command)
{
    esp_err_t status;

    status = ESP_OK;

    if (status == ESP_OK)
    {
        status = lcd_write_byte(
            handle,
            command,
            false);
    }

    if (status == ESP_OK)
    {
        if ((command == LCD_CMD_CLEAR_DISPLAY) ||
            (command == LCD_CMD_RETURN_HOME))
        {
            esp_rom_delay_us(
                LCD_DELAY_LONG_US);
        }
        else
        {
            esp_rom_delay_us(
                LCD_DELAY_SHORT_US);
        }
    }

    return status;
}

static esp_err_t lcd_update_display_control(
    i2c_lcd_hd44780_handle_t handle)
{
    uint8_t command;

    command = LCD_CMD_DISPLAY_CONTROL;

    if (handle->display_enabled)
    {
        command |= LCD_DISPLAY_ENABLE_BIT;
    }

    if (handle->cursor_enabled)
    {
        command |= LCD_DISPLAY_CURSOR_BIT;
    }

    if (handle->blink_enabled)
    {
        command |= LCD_DISPLAY_BLINK_BIT;
    }

    return lcd_write_command(
        handle,
        command);
}

static esp_err_t lcd_update_entry_mode(
    i2c_lcd_hd44780_handle_t handle)
{
    uint8_t command;

    command = LCD_CMD_ENTRY_MODE_SET;

    if (handle->entry_direction ==
        I2C_LCD_HD44780_ENTRY_INCREMENT)
    {
        command |= LCD_ENTRY_INCREMENT_BIT;
    }

    if (handle->display_shift_enabled)
    {
        command |= LCD_ENTRY_SHIFT_BIT;
    }

    return lcd_write_command(
        handle,
        command);
}

static esp_err_t lcd_initialize(
    i2c_lcd_hd44780_handle_t handle)
{
    esp_err_t status;

    status = ESP_OK;

    vTaskDelay(
        pdMS_TO_TICKS(
            LCD_POWER_UP_DELAY_MS));

    if (status == ESP_OK)
    {
        status = lcd_write_nibble(
            handle,
            0x03U,
            false);
    }

    if (status == ESP_OK)
    {
        esp_rom_delay_us(5000U);

        status = lcd_write_nibble(
            handle,
            0x03U,
            false);
    }

    if (status == ESP_OK)
    {
        esp_rom_delay_us(150U);

        status = lcd_write_nibble(
            handle,
            0x03U,
            false);
    }

    if (status == ESP_OK)
    {
        esp_rom_delay_us(150U);

        status = lcd_write_nibble(
            handle,
            0x02U,
            false);
    }

    if (status == ESP_OK)
    {
        status = lcd_write_command(
            handle,
            LCD_FUNCTION_SET);
    }

    if (status == ESP_OK)
    {
        status = lcd_update_display_control(
            handle);
    }

    if (status == ESP_OK)
    {
        status = lcd_write_command(
            handle,
            LCD_CMD_CLEAR_DISPLAY);
    }

    if (status == ESP_OK)
    {
        status = lcd_update_entry_mode(
            handle);
    }

    return status;
}

/******************************************************************************
 * Public Functions
 *****************************************************************************/

esp_err_t i2c_lcd_hd44780_create(
    const i2c_lcd_hd44780_config_t *config,
    i2c_lcd_hd44780_handle_t *handle)
{
    esp_err_t status;
    i2c_lcd_hd44780_handle_t instance;
    i2c_io_expander_pcf8574_config_t io_config;

    status = ESP_OK;
    instance = NULL;

    if ((config == NULL) ||
        (handle == NULL))
    {
        status = ESP_ERR_INVALID_ARG;
    }

    if (status == ESP_OK)
    {
        instance = calloc(
            1U,
            sizeof(*instance));

        if (instance == NULL)
        {
            status = ESP_ERR_NO_MEM;
        }
    }

    if (status == ESP_OK)
    {
        io_config =
            (i2c_io_expander_pcf8574_config_t)
        {
            .address =
                (uint8_t)config->device_address
        };

        status =
            i2c_io_expander_pcf8574_new(
                config->bus_handle,
                &io_config,
                &instance->io_expander_handle);
    }

    if (status == ESP_OK)
    {
        instance->display_enabled = true;
        instance->cursor_enabled = false;
        instance->blink_enabled = false;

        instance->backlight_enabled =
            config->backlight_enabled;

        instance->entry_direction =
            I2C_LCD_HD44780_ENTRY_INCREMENT;

        instance->display_shift_enabled = false;

        instance->port_shadow = 0U;

        if (instance->backlight_enabled)
        {
            instance->port_shadow |=
                (uint8_t)(
                    1U << LCD_PIN_BACKLIGHT);
        }

        status = lcd_initialize(
            instance);
    }

    if (status == ESP_OK)
    {
        *handle = instance;
    }
    else
    {
        if (instance != NULL)
        {
            if (instance->io_expander_handle
                != NULL)
            {
                (void)
                i2c_io_expander_pcf8574_del(
                    instance->io_expander_handle);
            }

            free(instance);
        }
    }

    return status;
}

esp_err_t i2c_lcd_hd44780_delete(
    i2c_lcd_hd44780_handle_t handle)
{
    esp_err_t status;

    status = ESP_OK;

    if (handle == NULL)
    {
        status = ESP_ERR_INVALID_ARG;
    }

    if (status == ESP_OK)
    {
        status =
            i2c_io_expander_pcf8574_del(
                handle->io_expander_handle);
    }

    if (status == ESP_OK)
    {
        free(handle);
    }

    return status;
}

esp_err_t i2c_lcd_hd44780_clear(
    i2c_lcd_hd44780_handle_t handle)
{
    esp_err_t status;

    status = ESP_OK;

    if (handle == NULL)
    {
        status = ESP_ERR_INVALID_ARG;
    }

    if (status == ESP_OK)
    {
        status = lcd_write_command(
            handle,
            LCD_CMD_CLEAR_DISPLAY);
    }

    return status;
}

esp_err_t i2c_lcd_hd44780_home(
    i2c_lcd_hd44780_handle_t handle)
{
    esp_err_t status;

    status = ESP_OK;

    if (handle == NULL)
    {
        status = ESP_ERR_INVALID_ARG;
    }

    if (status == ESP_OK)
    {
        status = lcd_write_command(
            handle,
            LCD_CMD_RETURN_HOME);
    }

    return status;
}

esp_err_t i2c_lcd_hd44780_set_ddram_address(
    i2c_lcd_hd44780_handle_t handle,
    uint8_t address)
{
    esp_err_t status;

    status = ESP_OK;

    if ((handle == NULL) ||
        (address > 0x7FU))
    {
        status = ESP_ERR_INVALID_ARG;
    }

    if (status == ESP_OK)
    {
        status = lcd_write_command(
            handle,
            (uint8_t)(
                LCD_CMD_SET_DDRAM_ADDRESS |
                address));
    }

    return status;
}

esp_err_t i2c_lcd_hd44780_set_cgram_address(
    i2c_lcd_hd44780_handle_t handle,
    uint8_t address)
{
    esp_err_t status;

    status = ESP_OK;

    if ((handle == NULL) ||
        (address > 0x3FU))
    {
        status = ESP_ERR_INVALID_ARG;
    }

    if (status == ESP_OK)
    {
        status = lcd_write_command(
            handle,
            (uint8_t)(
                LCD_CMD_SET_CGRAM_ADDRESS |
                address));
    }

    return status;
}

esp_err_t i2c_lcd_hd44780_write_data(
    i2c_lcd_hd44780_handle_t handle,
    uint8_t data)
{
    esp_err_t status;

    status = ESP_OK;

    if (handle == NULL)
    {
        status = ESP_ERR_INVALID_ARG;
    }

    if (status == ESP_OK)
    {
        status = lcd_write_byte(
            handle,
            data,
            true);
    }

    return status;
}

esp_err_t i2c_lcd_hd44780_set_entry_mode(
    i2c_lcd_hd44780_handle_t handle,
    i2c_lcd_hd44780_entry_direction_t direction,
    bool display_shift_enabled)
{
    esp_err_t status;

    status = ESP_OK;

    if (handle == NULL)
    {
        status = ESP_ERR_INVALID_ARG;
    }

    if (status == ESP_OK)
    {
        if ((direction !=
                I2C_LCD_HD44780_ENTRY_INCREMENT) &&
            (direction !=
                I2C_LCD_HD44780_ENTRY_DECREMENT))
        {
            status = ESP_ERR_INVALID_ARG;
        }
    }

    if (status == ESP_OK)
    {
        handle->entry_direction = direction;
        handle->display_shift_enabled =
            display_shift_enabled;

        status = lcd_update_entry_mode(
            handle);
    }

    return status;
}

esp_err_t i2c_lcd_hd44780_set_display_enabled(
    i2c_lcd_hd44780_handle_t handle,
    bool enabled)
{
    esp_err_t status;

    status = ESP_OK;

    if (handle == NULL)
    {
        status = ESP_ERR_INVALID_ARG;
    }

    if (status == ESP_OK)
    {
        handle->display_enabled = enabled;

        status =
            lcd_update_display_control(
                handle);
    }

    return status;
}

esp_err_t i2c_lcd_hd44780_set_cursor_enabled(
    i2c_lcd_hd44780_handle_t handle,
    bool enabled)
{
    esp_err_t status;

    status = ESP_OK;

    if (handle == NULL)
    {
        status = ESP_ERR_INVALID_ARG;
    }

    if (status == ESP_OK)
    {
        handle->cursor_enabled = enabled;

        status =
            lcd_update_display_control(
                handle);
    }

    return status;
}

esp_err_t i2c_lcd_hd44780_set_blink_enabled(
    i2c_lcd_hd44780_handle_t handle,
    bool enabled)
{
    esp_err_t status;

    status = ESP_OK;

    if (handle == NULL)
    {
        status = ESP_ERR_INVALID_ARG;
    }

    if (status == ESP_OK)
    {
        handle->blink_enabled = enabled;

        status =
            lcd_update_display_control(
                handle);
    }

    return status;
}

esp_err_t i2c_lcd_hd44780_set_backlight_enabled(
    i2c_lcd_hd44780_handle_t handle,
    bool enabled)
{
    esp_err_t status;

    status = ESP_OK;

    if (handle == NULL)
    {
        status = ESP_ERR_INVALID_ARG;
    }

    if (status == ESP_OK)
    {
        handle->backlight_enabled = enabled;

        if (enabled)
        {
            handle->port_shadow |=
                (uint8_t)(
                    1U << LCD_PIN_BACKLIGHT);
        }
        else
        {
            handle->port_shadow &=
                (uint8_t)(
                    ~(1U << LCD_PIN_BACKLIGHT));
        }

        status = lcd_write_expander(
            handle,
            handle->port_shadow);
    }

    return status;
}