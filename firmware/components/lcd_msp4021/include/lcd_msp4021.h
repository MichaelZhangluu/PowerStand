#pragma once

/**
 * @file lcd_msp4021.h
 * @brief device and component driver for LCD module msp4021
 */

#ifndef LCD_MSP4021_H
#define LCD_MSP4021_H

/* Dependency includes */
/* Display*/
#include "lvgl.h"
/* File system */
#include "esp_vfs_fat.h"

/* Public structure */
typedef struct 
{
    uint16_t    display_width;
    uint16_t    display_height;
} lcd_msp4021_info_t;

/************************************************************/
/*          LCD Back Light Control                          */
/************************************************************/
/**
 * @brief Sets the LCD backlight brightness using a standard percentage window.
 * 
 * @param brightness_percent  Target brightness from 0 (off) to 100 (full bright)
 */
esp_err_t lcd_msp4021_set_lcd_bklc_brightness(uint8_t brightness);

/**
 * @brief Get current LCD backlight brightness percentage
 * 
 * @return
 *      - Percentage
 */
uint8_t lcd_msp4021_get_lcd_bklc_brightness(void);

/**
 * @brief Set LCD back light brightness to max
 */
esp_err_t lcd_msp4021_set_lcd_bklc_max(void);

/**
 * @brief Set LCD back light brightness to min
 */
esp_err_t lcd_msp4021_set_lcd_bklc_min(void);

/**
 * @brief Increase the LCD backlight brightness by 10%
 * 
 * @return
 *      - ESP_OK:    Success
 *      - Otherwise: Fail
 */
esp_err_t lcd_msp4021_lcd_bklc_increase(void);

/**
 * @brief Decrease the LCD backlight brightness by 10%
 * 
 * @return
 *      - ESP_OK:    Success
 *      - Otherwise: Fail
 */
esp_err_t lcd_msp4021_lcd_bklc_decrease(void);

/************************************************************/
/*          MSP4021 Module SD card file system              */
/************************************************************/
/**
 * @brief Safely combines the global mount path and a filename into a destination buffer.
 * 
 * @param filename           The raw file name or subpath (e.g., "hello_file.txt")
 * @param full_filename      The output destination string buffer array
 * @param max_size           The maximum memory capacity allocation of full_filename
 * @return esp_err_t         ESP_OK on success, or an error code on configuration failure
 */
esp_err_t lcd_msp4021_fs_get_full_filename(const char *filename, char *full_filename, size_t max_size);


/************************************************************/
/*          MSP4021 Module Initialization                   */
/************************************************************/
/**
 * @brief  Return msp4021 display module information
 *
 * @return
 *      - Display info
 */
lcd_msp4021_info_t lcd_msp4021_get_info(void);

/**
 * @brief  Initialize the MSP4021 LCD panel as lvgl display with integrated touch input device.
 *
 * @param[out] lvgl_display_handle: Pointer to the handle of the initialized LVGL display.
 * @param[out] lvgl_touch_handle: Pointer to the handle of the initialzed LVGL touch device.
 * @return
 *      - ESP_OK:    Success
 *      - Otherwise: Fail
 */
esp_err_t lcd_msp4021_display_init(lv_display_t *lvgl_display_handle, lv_indev_t *lvgl_touch_handle, sdmmc_card_t **sd_fat_card_handle);

#endif // LCD_MSP4021_H
