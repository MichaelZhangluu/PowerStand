/**
 * @file i2c_io_expander_pcf8574.c
 * @brief PCF8574 I2C GPIO Expander Driver.
 *
 * Thread Safety:
 *
 * This driver is not thread-safe.
 * Access to a device instance shall be serialized
 * by the owning task.
 */

#include "i2c_io_expander_pcf8574.h"

#include <stdbool.h>
#include <stdlib.h>

/******************************************************************************
 * Private Constants
 *****************************************************************************/
#define PCF8574_I2C_SCL_SPEED_HZ (100000U)
#define PCF8574_I2C_SCL_WAIT_US (0U)
#define PCF8574_I2C_TIMEOUT_MS (100U)
#define PCF8574_DEFAULT_PORT_STATE (0xFFU)
#define PCF8574_ADDRESS_MIN (0x20U)
#define PCF8574_ADDRESS_MAX (0x27U)

/******************************************************************************
 * Private Types
 *****************************************************************************/
struct i2c_io_expander_pcf8574_t
{
    i2c_master_dev_handle_t i2c_dev;

    /**
     * @brief Cached port output state.
     *
     * The PCF8574 only supports full-byte writes.
     * Individual pin operations update this cache
     * and then rewrite the entire port.
     */
    uint8_t output_state;
};

/******************************************************************************
 * Private Function Prototypes
 *****************************************************************************/
static bool pcf8574_is_valid_pin(i2c_io_expander_pcf8574_pin_t pin);
static esp_err_t pcf8574_write(i2c_io_expander_pcf8574_handle_t handle, uint8_t value);
static esp_err_t pcf8574_read(i2c_io_expander_pcf8574_handle_t handle, uint8_t *value);

/******************************************************************************
 * Private Functions
 *****************************************************************************/
static bool pcf8574_is_valid_pin(i2c_io_expander_pcf8574_pin_t pin)
{
    bool valid =
        ((pin >= I2C_IO_EXPANDER_PCF8574_PIN_P0) &&
         (pin <= I2C_IO_EXPANDER_PCF8574_PIN_P7));

    return valid;
}

/******************************************************************************/
static esp_err_t pcf8574_write(i2c_io_expander_pcf8574_handle_t handle, uint8_t value)
{
    esp_err_t status = ESP_OK;

    if (handle == NULL)
    {
        status = ESP_ERR_INVALID_ARG;
    }

    if (status == ESP_OK)
    {
        status = i2c_master_transmit(handle->i2c_dev, &value, sizeof(value), PCF8574_I2C_TIMEOUT_MS);
    }

    return status;
}

/******************************************************************************/
static esp_err_t pcf8574_read(i2c_io_expander_pcf8574_handle_t handle, uint8_t *value)
{
    esp_err_t status = ESP_OK;

    if (handle == NULL)
    {
        status = ESP_ERR_INVALID_ARG;
    }

    if (value == NULL)
    {
        status = ESP_ERR_INVALID_ARG;
    }

    if (status == ESP_OK)
    {
        status = i2c_master_receive(handle->i2c_dev, value, sizeof(*value), PCF8574_I2C_TIMEOUT_MS);
    }

    return status;
}

/******************************************************************************
 * Public Functions
 *****************************************************************************/
esp_err_t i2c_io_expander_pcf8574_new(i2c_master_bus_handle_t bus_handle, const i2c_io_expander_pcf8574_config_t *config, i2c_io_expander_pcf8574_handle_t *ret_handle)
{
    esp_err_t status = ESP_OK;

    i2c_io_expander_pcf8574_handle_t device = NULL;

    i2c_device_config_t device_config =
        {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = 0U,
            .scl_speed_hz = 0U,
            .scl_wait_us = 0U};

    if (ret_handle != NULL)
    {
        *ret_handle = NULL;
    }

    if (bus_handle == NULL)
    {
        status = ESP_ERR_INVALID_ARG;
    }
    else if (config == NULL)
    {
        status = ESP_ERR_INVALID_ARG;
    }
    else if (ret_handle == NULL)
    {
        status = ESP_ERR_INVALID_ARG;
    }

    if (status == ESP_OK)
    {
        if ((config->address < PCF8574_ADDRESS_MIN) ||
            (config->address > PCF8574_ADDRESS_MAX))
        {
            status = ESP_ERR_INVALID_ARG;
        }
    }

    if (status == ESP_OK)
    {
        device = calloc(1, sizeof(struct i2c_io_expander_pcf8574_t));

        if (device == NULL)
        {
            status = ESP_ERR_NO_MEM;
        }
    }

    if (status == ESP_OK)
    {
        device->output_state = PCF8574_DEFAULT_PORT_STATE;
    }

    if (status == ESP_OK)
    {
        device_config.device_address = config->address;
        device_config.scl_speed_hz = PCF8574_I2C_SCL_SPEED_HZ;
        device_config.scl_wait_us = PCF8574_I2C_SCL_WAIT_US;

        status = i2c_master_bus_add_device(bus_handle, &device_config, &device->i2c_dev);
    }

    if (status == ESP_OK)
    {
        status = pcf8574_write(device, device->output_state);
    }

    if (status != ESP_OK)
    {
        if (device != NULL)
        {
            if (device->i2c_dev != NULL)
            {
                (void)i2c_master_bus_rm_device(device->i2c_dev);
            }

            free(device);
        }
    }
    else
    {
        *ret_handle = device;
    }

    return status;
}

/******************************************************************************/
esp_err_t i2c_io_expander_pcf8574_del(i2c_io_expander_pcf8574_handle_t handle)
{
    esp_err_t status = ESP_OK;

    if (handle == NULL)
    {
        status = ESP_ERR_INVALID_ARG;
    }

    if (status == ESP_OK)
    {
        status = i2c_master_bus_rm_device(handle->i2c_dev);
    }

    if (status == ESP_OK)
    {
        free(handle);
    }

    return status;
}

/******************************************************************************/
esp_err_t i2c_io_expander_pcf8574_read_port(i2c_io_expander_pcf8574_handle_t handle, uint8_t *value)
{
    esp_err_t status = pcf8574_read(handle, value);

    return status;
}

/******************************************************************************/
esp_err_t i2c_io_expander_pcf8574_write_port(i2c_io_expander_pcf8574_handle_t handle, uint8_t value)
{
    esp_err_t status = ESP_OK;

    if (handle == NULL)
    {
        status = ESP_ERR_INVALID_ARG;
    }

    if (status == ESP_OK)
    {
        status = pcf8574_write(handle, value);
    }

    if (status == ESP_OK)
    {
        handle->output_state = value;
    }

    return status;
}

/******************************************************************************/
esp_err_t i2c_io_expander_pcf8574_update_bits(i2c_io_expander_pcf8574_handle_t handle, uint8_t mask, uint8_t value)
{
    esp_err_t status = ESP_OK;
    uint8_t new_value = 0U;

    if (handle == NULL)
    {
        status = ESP_ERR_INVALID_ARG;
    }

    if (status == ESP_OK)
    {
        new_value = (handle->output_state & (uint8_t)(~mask)) | (value & mask);

        status = pcf8574_write(handle, new_value);

        if (status == ESP_OK)
        {
            handle->output_state = new_value;
        }
    }

    return status;
}

/******************************************************************************/

esp_err_t i2c_io_expander_pcf8574_set_pin(i2c_io_expander_pcf8574_handle_t handle, i2c_io_expander_pcf8574_pin_t pin, uint32_t level)
{
    esp_err_t status = ESP_OK;
    uint8_t mask = 0U;
    uint8_t value = 0U;

    if (handle == NULL)
    {
        status = ESP_ERR_INVALID_ARG;
    }

    if ((status == ESP_OK) && (!pcf8574_is_valid_pin(pin)))
    {
        status = ESP_ERR_INVALID_ARG;
    }

    if (status == ESP_OK)
    {
        level = (level != 0U) ? 1U : 0U;
        mask = (uint8_t)(1U << pin);
        value = (level != 0U) ? mask : 0U;

        status = i2c_io_expander_pcf8574_update_bits(handle, mask, value);
    }

    return status;
}

/******************************************************************************/

esp_err_t i2c_io_expander_pcf8574_get_pin(i2c_io_expander_pcf8574_handle_t handle, i2c_io_expander_pcf8574_pin_t pin, uint32_t *level)
{
    esp_err_t status = ESP_OK;
    uint8_t port_value = 0U;

    if (handle == NULL)
    {
        status = ESP_ERR_INVALID_ARG;
    }

    if (level == NULL)
    {
        status = ESP_ERR_INVALID_ARG;
    }

    if ((status == ESP_OK) && (!pcf8574_is_valid_pin(pin)))
    {
        status = ESP_ERR_INVALID_ARG;
    }

    if (status == ESP_OK)
    {
        status = pcf8574_read(handle, &port_value);
    }

    if (status == ESP_OK)
    {
        *level = (uint32_t)((port_value >> pin) & 0x01U);
    }

    return status;
}