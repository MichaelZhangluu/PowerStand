/**
 * @file lcd_msp4021.c
 * @brief device and component driver for LCD module msp4021
 */

/* Dependency Includes */
#include <stdio.h>
#include <esp_check.h>
#include "esp_log.h"
#include "esp_timer.h"
// Base LCD lib from ESP IDF
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_dev.h"
// GPIO lib
#include "hal/gpio_types.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
// LCD controller ST7796 driver.
#include "esp_lcd_st7796.h"
// Touch controller xpt2046 driver.
#include "esp_lcd_touch.h"
#include "esp_lcd_touch_xpt2046.h"
// Header file for this module
#include "lcd_msp4021.h"
// Platform hardware configurations
#include "hardware_config.h"

// This LCD module, SKU MSP4021, is a 480x320 pixel display powered by ST7796 driver and touch controller XPT2046.
// The specifcations of this module can be found: https://www.lcdwiki.com/4.0inch_SPI_Module_ST7796
// ST7796 driver datasheet: https://www.displayfuture.com/Display/datasheet/controller/ST7796s.pdf
// XPT2046 touch controller datasheet: https://grobotronics.com/images/datasheets/xpt2046-datasheet.pdf
// This driver configured the LCD module in horizontal display mode
// static const char *TAG = "lcd_msp4021.c";

// Hardware Configuration for the LCD module MSP4021
// Display dimension
#define LCD_MSP4021_DISPLAY_WIDTH (uint16_t)480
#define LCD_MSP4021_DISPLAY_HEIGHT (uint16_t)320
// Command format
#define LCD_MSP4021_CMD_OVERHEAD (uint8_t)0x08
// Clock speed setting for display device
#define LCD_MSP4021_CLOCK_SPEED_HZ (40 * 1000 * 1000) // 40 MHz clock speed for LCD panel
// Display buffers for LVGL
// 2 bytes per pixel for RGB565 format, 2 buffers for LVGL, each covers partial size (1/10) of the screen
#define LCD_MSP4021_LVGL_SCREEN_MODIFIER 10
#define LCD_MSP4021_DISPLAY_BUFFER_SIZE (LCD_MSP4021_DISPLAY_WIDTH * LCD_MSP4021_DISPLAY_HEIGHT / LCD_MSP4021_LVGL_SCREEN_MODIFIER) // This is the size in uint16_t type

static uint16_t lcd_msp4021_lvgl_display_buffer1[LCD_MSP4021_DISPLAY_BUFFER_SIZE];
static uint16_t lcd_msp4021_lvgl_display_buffer2[LCD_MSP4021_DISPLAY_BUFFER_SIZE];

#define LCD_MSP4021_SD_CLOCK_SPEED_HZ (40 * 1000 * 1000) // 40 MHz clock speed for SD Card
#define LCD_MSP4021_FS_ALLOCATION_UINT_SIZE 512 * 2 * 16 // File system allocation unit size

/**
 * @brief  Return msp4021 display module information
 *
 * @return
 *      - Display info
 */
lcd_msp4021_info_t lcd_msp4021_get_info(void)
{
    lcd_msp4021_info_t info = {
        .display_width = LCD_MSP4021_DISPLAY_WIDTH,
        .display_height = LCD_MSP4021_DISPLAY_HEIGHT,
    };

    return info;
}

/* LCD Backlight is controlled by a PWM pin. */
// PWM Backlight configuration
#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE   // Use LOW_SPEED for ESP32-S3/C3/S2
#define LEDC_DUTY_RES LEDC_TIMER_13_BIT // 13-bit resolution (0 to 8191)
#define LEDC_FREQUENCY (5000)           // 5 kHz frequency

// PWM channel for LCD backlight control
#define LEDC_CHANNEL_LCD_BKCL LEDC_CHANNEL_0

// PWM range
#define LCD_MSP4021_LCD_BKCL_BRIGHTNESS_MAX 100 // Range 0-100%
#define LCD_MSP4021_LCD_BKCL_BRIGHTNESS_MIN 20  // Keep the screen visible

/**
 * @brief Initializes the global SPI buses required for the MSP4021 module.
 *        SPI2 is dedicated to the LCD panel.
 *        SPI3 is shared between the touch controller and the SD card.
 */
esp_err_t lcd_msp4021_global_spi_bus_init(void)
{
    esp_err_t status;

    // SPI bus configuration for the LCD panel
    spi_bus_config_t hspi_bus_config =
        {
            .mosi_io_num = LCD_MSP4021_MOSI,
            .miso_io_num = LCD_MSP4021_MISO,
            .sclk_io_num = LCD_MSP4021_SCK,
            .quadwp_io_num = -1,                                                           // Not used
            .quadhd_io_num = -1,                                                           // Not used
            .data4_io_num = -1,                                                            // Not used
            .data5_io_num = -1,                                                            // Not used
            .data6_io_num = -1,                                                            // Not used
            .data7_io_num = -1,                                                            // Not used
            .data_io_default_level = 0,                                                    // Default level for data IO when no transaction
            .max_transfer_sz = LCD_MSP4021_DISPLAY_BUFFER_SIZE + LCD_MSP4021_CMD_OVERHEAD, // Maximum transfer size, in bytes.
            .dma_burst_size = 0,                                                           // Use driver default
            .flags = 0,                                                                    // Does not allow power down during light sleep
            .isr_cpu_id = 0,                                                               // Use CPU 0 for SPI ISR
            .intr_flags = 0,                                                               // No special interrupt flags
    };

    // LCD Touch Panel SPI bus configurations
    spi_bus_config_t vspi_bus_config =
        {
            .mosi_io_num = LCD_MSP4021_TOUCH_MOSI,
            .miso_io_num = LCD_MSP4021_TOUCH_MISO,
            .sclk_io_num = LCD_MSP4021_TOUCH_SCK,
            .quadwp_io_num = -1,        // Not used
            .quadhd_io_num = -1,        // Not used
            .data4_io_num = -1,         // Not used
            .data5_io_num = -1,         // Not used
            .data6_io_num = -1,         // Not used
            .data7_io_num = -1,         // Not used
            .data_io_default_level = 0, // Default level for data IO when no transaction
            .max_transfer_sz = LCD_MSP4021_FS_ALLOCATION_UINT_SIZE,
            .dma_burst_size = 0, // Use driver default
            .flags = 0,          // Does not allow power down during light sleep
            .isr_cpu_id = 0,     // Use CPU 0 for SPI ISR
            .intr_flags = 0,     // No special interrupt flags
    };

    // LCD display SPI bus init
    status = spi_bus_initialize(LCD_MSP4021_DISPLAY_SPI_PORT, &hspi_bus_config, SPI_DMA_CH_AUTO);

    // Touch and SD card SPI bus configuration for the shared SPI3 bus

    if (status == ESP_OK)
    {
        // Bus init
        status = spi_bus_initialize(LCD_MSP4021_TOUCH_SPI_PORT, &vspi_bus_config, SPI_DMA_CH_AUTO);
    }

    return status;
}

/**
 * @brief Sets the LCD backlight brightness using a standard percentage window.
 *
 * @param brightness_percent  Target brightness from 0 (off) to 100 (full bright)
 */
esp_err_t lcd_msp4021_set_lcd_bklc_brightness(uint8_t brightness)
{
    esp_err_t status;

    if (brightness > LCD_MSP4021_LCD_BKCL_BRIGHTNESS_MAX)
    {
        brightness = LCD_MSP4021_LCD_BKCL_BRIGHTNESS_MAX;
    }

    if (brightness < LCD_MSP4021_LCD_BKCL_BRIGHTNESS_MIN)
    {
        brightness = LCD_MSP4021_LCD_BKCL_BRIGHTNESS_MIN;
    }

    // Map 0-100% up to the 13-bit hardware duty resolution ceiling (0 to 8191)
    uint32_t target_duty = (brightness * 8191) / 100;

    // According board schematics, the BSS138 Backlight control circuit
    // turns ON the backlight LEDs when the control pin state is HIGH (Active High).
    // Therefore, 0 duty is completely off, and 8191 duty is maximum brightness.
    status = ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_LCD_BKCL, target_duty);
    if (status == ESP_OK)
    {
        status = ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_LCD_BKCL);
    }

    return status;
}

/**
 * @brief Set LCD back light brightness to max
 */

esp_err_t lcd_msp4021_set_lcd_bklc_max(void)
{
    return lcd_msp4021_set_lcd_bklc_brightness(LCD_MSP4021_LCD_BKCL_BRIGHTNESS_MAX);
}

/**
 * @brief Set LCD back light brightness to min
 */

esp_err_t lcd_msp4021_set_lcd_bklc_min(void)
{
    return lcd_msp4021_set_lcd_bklc_brightness(LCD_MSP4021_LCD_BKCL_BRIGHTNESS_MIN);
}

/**
 * @brief Get current LCD backlight brightness percentage
 *
 * @return
 *      - Percentage
 */
uint8_t lcd_msp4021_get_lcd_bklc_brightness(void)
{
    int32_t duty = ledc_get_duty(LEDC_MODE, LEDC_CHANNEL_LCD_BKCL);
    return (duty * 100 / 8191);
}

/**
 * @brief Increase the LCD backlight brightness by 10%
 *
 * @return
 *      - ESP_OK:    Success
 *      - Otherwise: Fail
 */
esp_err_t lcd_msp4021_lcd_bklc_increase(void)
{
    return (lcd_msp4021_set_lcd_bklc_brightness(lcd_msp4021_get_lcd_bklc_brightness() + 10));
}

/**
 * @brief Decrease the LCD backlight brightness by 10%
 *
 * @return
 *      - ESP_OK:    Success
 *      - Otherwise: Fail
 */
esp_err_t lcd_msp4021_lcd_bklc_decrease(void)
{
    return (lcd_msp4021_set_lcd_bklc_brightness(lcd_msp4021_get_lcd_bklc_brightness() - 10));
}

esp_err_t lcd_msp4021_lcd_bkcl_init(int lcd_bkcl_io_num)
{
    esp_err_t status;

    // Configure the PWM Timer
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER,
        .duty_resolution = LEDC_DUTY_RES,
        .freq_hz = LEDC_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    ledc_channel_config_t lcd_bkcl_pwm_channel = {
        .gpio_num = lcd_bkcl_io_num,      // Backlight control pin
        .speed_mode = LEDC_MODE,          // Uses LEDC_LOW_SPEED_MODE
        .channel = LEDC_CHANNEL_LCD_BKCL, // Separate channel to avoid overriding RGB pins
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER, // Shared LEDC_TIMER_0 (13-bit, 5kHz)
        .duty = 8191,            // Start at 100% full brightness initially (2^13 - 1)
        .hpoint = 0,
    };

    // configure timer;
    status = ledc_timer_config(&ledc_timer);

    if (status == ESP_OK)
    {
        // Create backlight PWM pin
        status = ledc_channel_config(&lcd_bkcl_pwm_channel);
    }

    return status;
}

/************************************************************/
/*          LCD Display panel initialization                */
/************************************************************/
esp_err_t lcd_msp4021_lcd_panel_init(esp_lcd_panel_handle_t *display_handle)
{
    esp_err_t status = ESP_OK;

    // LCD IO device configuration
    esp_lcd_panel_io_handle_t lcd_io_handle;
    esp_lcd_panel_io_spi_config_t lcd_io_config =
        {
            .cs_gpio_num = LCD_MSP4021_CS,
            .dc_gpio_num = LCD_MSP4021_DC,
            .spi_mode = 0,                         // The driver samples the SDA (input data) at rising edge of SCL,
                                                   // but shifts SDA (output data) at the falling edge of SCL
                                                   // SPI Mode 0: CPOL = 0, CPHA = 0, SCK idle low, data latched on rising edge
            .pclk_hz = LCD_MSP4021_CLOCK_SPEED_HZ, // Safe SPI clock for this panel,
                                                   // The read might fail.
                                                   // Write clock width is limited to 66ns --> safe clock speed shall be 15.15MHz
                                                   // write clock width is limited to 150ns --> safe clock speed shall be 6.67MHz
                                                   // When set to 40MHz, the panel I used may fail with a higher temperature.
                                                   // The solution is to use lower SPI clock at 20MHz. Need to try using 3.3v power supply instead of 5V.
            .trans_queue_depth = 10,               // Transaction queue depth, can be tuned based on application needs
            .lcd_cmd_bits = 8,
            .lcd_param_bits = 8,
            .cs_ena_pretrans = 0,        // No extra CS hold time before transaction
            .cs_ena_posttrans = 0,       // No extra CS hold time after transaction
            .flags.dc_high_on_cmd = 0,   // DC level = 0 indicates command transfer
            .flags.dc_low_on_data = 0,   // DC level = 1 indicates data transfer
            .flags.dc_low_on_param = 0,  // DC level = 1 indicates parameter transfer
            .flags.octal_mode = 0,       // Not using octal mode
            .flags.quad_mode = 0,        // Not using quad mode
            .flags.sio_mode = 0,         // Not using single I/O mode, this refers to the 3-wire SPI mode, this module is hardware straped to 4-wire SPI mode
            .flags.psram_dma_direct = 0, // Not using PSRAM for DMA buffer directly
            .flags.lsb_first = 0,        // Transmit MSB first, section 8.2.4.1 wire sequence, the first bit is the MSB of the command or parameter
            .flags.cs_high_active = 0,   // CS line is active low
    };

    // LCD Panel configuration
    esp_lcd_panel_dev_config_t lcd_panel_device_config =
        {
            .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB, // RGB element order: RGB
            .data_endian = LCD_RGB_DATA_ENDIAN_LITTLE,  // RGB data endian: MSB first, ESP is a little endian system, there shall have byte swap
            .bits_per_pixel = 16,                       // Color depth: 16 bits per pixel for RGB565 format
            .reset_gpio_num = LCD_MSP4021_RST,          // Reset GPIO pin for LCD panel, not used in this module
            .vendor_config = NULL,                      // Use default vendor configuration for ST7789 driver
            .flags.reset_active_high = 0,               // Reset is active low
    };

    // Initialize the LCD panel IO
    if (status == ESP_OK)
    {
        status = esp_lcd_new_panel_io_spi(LCD_MSP4021_DISPLAY_SPI_PORT, &lcd_io_config, &lcd_io_handle);
    }

    // New ST7796 device
    if (status == ESP_OK)
    {
        status = esp_lcd_new_panel_st7796(lcd_io_handle, &lcd_panel_device_config, display_handle);
    }

    // Initialize panel
    if (status == ESP_OK)
    {
        status = esp_lcd_panel_init(*display_handle);
    }

    // The screen default is to set X and Y in a vertical orientation that hight is 480 and width is 320.
    // The screen is rotated 90 degrees to set the X and Y in a horizontal orientation that hight is 320 and width is 480.
    if (status == ESP_OK)
    {
        status = esp_lcd_panel_swap_xy(*display_handle, true);
    }

    // Initialize back light control
    if (status == ESP_OK)
    {
        status = lcd_msp4021_lcd_bkcl_init(LCD_MSP4021_BCKL);
    }

    // Set lcd back light control to max brightness
    if (status == ESP_OK)
    {
        status = lcd_msp4021_set_lcd_bklc_max();
    }

    // Turn on the display
    if (status == ESP_OK)
    {
        status = esp_lcd_panel_disp_on_off(*display_handle, true);
    }

    return status;
}

/************************************************************/
/*          Touch Driver                                    */
/************************************************************/
// Initialize touch device
esp_err_t lcd_msp4021_lcd_touch_init(esp_lcd_touch_handle_t *touch_handle)
{
    esp_err_t status = ESP_OK;
    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    esp_lcd_panel_io_spi_config_t tp_io_config = ESP_LCD_TOUCH_IO_SPI_XPT2046_CONFIG(LCD_MSP4021_TOUCH_CS);

    // Touch Panel controller device configurations
    esp_lcd_touch_config_t touch_config =
        {
            .x_max = LCD_MSP4021_DISPLAY_HEIGHT,
            .y_max = LCD_MSP4021_DISPLAY_WIDTH,

            .rst_gpio_num = GPIO_NUM_NC,
            .int_gpio_num = LCD_MSP4021_TOUCH_IRQ,

            .levels = {
                .reset = 0,
                .interrupt = 0,
            },

            .flags.swap_xy = true,
            .flags.mirror_x = true,
            .flags.mirror_y = true,

            .process_coordinates = NULL,
            .interrupt_callback = NULL,
            .user_data = NULL,
            .driver_data = NULL,
    };

    if (status == ESP_OK)
    {
        // Touch controller IO init
        status = esp_lcd_new_panel_io_spi(LCD_MSP4021_TOUCH_SPI_PORT, &tp_io_config, &tp_io_handle);
    }

    if (status == ESP_OK)
    {
        // New touch device
        status = esp_lcd_touch_new_spi_xpt2046(tp_io_handle, &touch_config, touch_handle);
    }

    return status;
}

/**
 * @brief  LVGL touch read callback function to read the touch data from the XPT2046 touch controller
 * @param[in] indev: lvgl input device handle
 * @param[out] data: touch data to be filled
 */
void lcd_msp4021_lvgl_touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
#define LCD_MSP4021_MAX_TOUCH_COUNT 1 // resistive touch

    // Calibration Constants derived from log analysis
    //    const int32_t RAW_MIN_X = 27;
    //    const int32_t RAW_MAX_X = 449;
    //    const int32_t RAW_MIN_Y = 27;
    //    const int32_t RAW_MAX_Y = 303;

    const int32_t RAW_MIN_X = 21;
    const int32_t RAW_MAX_X = 459;
    const int32_t RAW_MIN_Y = 24;
    const int32_t RAW_MAX_Y = 307;

    const int32_t SCREEN_MAX_X = 479; // 480 - 1
    const int32_t SCREEN_MAX_Y = 319; // 320 - 1

    int32_t cal_x;
    int32_t cal_y;

    uint8_t touch_count = 0;
    esp_lcd_touch_point_data_t touch_data;
    esp_err_t status = ESP_OK;

    esp_lcd_touch_handle_t touch_handle = NULL;

    data->state = LV_INDEV_STATE_RELEASED;

    // Get touch handle from user data;
    touch_handle = (esp_lcd_touch_handle_t)lv_indev_get_user_data(indev);
    if (touch_handle == NULL)
    {
        return;
    }

    status = esp_lcd_touch_read_data(touch_handle);
    if (status == ESP_OK)
    {
        status = esp_lcd_touch_get_data(touch_handle, &touch_data, &touch_count, LCD_MSP4021_MAX_TOUCH_COUNT);
    }

    if (status == ESP_OK && touch_count > 0)
    {
        data->state = LV_INDEV_STATE_PRESSED;

        // Apply X Calibration & Clamping
        cal_x = ((int32_t)touch_data.x - RAW_MIN_X) * SCREEN_MAX_X / (RAW_MAX_X - RAW_MIN_X);
        if (cal_x < 0)
        {
            cal_x = 0;
        }
        if (cal_x > SCREEN_MAX_X)
        {
            cal_x = SCREEN_MAX_X;
        }
        data->point.x = (lv_coord_t)cal_x;

        // Apply Y Calibration & Clamping
        cal_y = ((int32_t)touch_data.y - RAW_MIN_Y) * SCREEN_MAX_Y / (RAW_MAX_Y - RAW_MIN_Y);
        if (cal_y < 0)
        {
            cal_y = 0;
        }
        if (cal_y > SCREEN_MAX_Y)
        {
            cal_y = SCREEN_MAX_Y;
        }
        data->point.y = (lv_coord_t)cal_y;
    }

    return;
}

/********************************************************/
/*      SD Card FAT File system initialization          */
/********************************************************/
/* Initialize SD Card component*/
#define LCD_MSP4021_FS_MAX_FILES_OPENED 4   // 4 files can be opened at the same time
#define LCD_MSP4021_FS_MOUNT_PATH "/sdcard" // Mount file system to SD card
char lcd_msp4021_fs_mount_path[] = LCD_MSP4021_FS_MOUNT_PATH;

/**
 * @brief // Initialize SD Card SPI host device
 *
 * @return
 *      - ESP_OK:    Success
 *      - Otherwise: Fail
 */
esp_err_t lcd_msp4021_sd_card_init(sdmmc_card_t **out_card)
{
    esp_err_t status = ESP_OK;

    // SPI SD card configuration
    sdspi_device_config_t lcd_msp4021_sd_device_config = {
        .host_id = LCD_MSP4021_SD_SPI_PORT,
        .gpio_cs = LCD_MSP4021_SD_CS,
        .gpio_cd = SDSPI_SLOT_NO_CD,
        .gpio_int = SDSPI_SLOT_NO_INT,
        .gpio_wp = SDSPI_SLOT_NO_WP,
        .wait_for_miso = 0};

    // SD Card FAT mounting configuration
    esp_vfs_fat_mount_config_t lcd_msp4021_sd_fat_mount_config = {
        .format_if_mount_failed = false,
        .max_files = LCD_MSP4021_FS_MAX_FILES_OPENED,
        .allocation_unit_size = LCD_MSP4021_FS_ALLOCATION_UINT_SIZE,
        .disk_status_check_enable = false,
        .use_one_fat = false};

    sdmmc_host_t lcd_msp4021_sdmmc_host_config = SDSPI_HOST_DEFAULT();
    lcd_msp4021_sdmmc_host_config.slot = LCD_MSP4021_SD_SPI_PORT;

    // SD Card host device init
    status = sdspi_host_init();

    if (status == ESP_OK)
    {
        status = esp_vfs_fat_sdspi_mount(lcd_msp4021_fs_mount_path, &lcd_msp4021_sdmmc_host_config, &lcd_msp4021_sd_device_config, &lcd_msp4021_sd_fat_mount_config, out_card);
    }

    return status;
}

/********************************************************/
/*      LVGL supporting driver                          */
/********************************************************/
/**
 * @brief  LVGL tick get function using system timer
 *
 * @param[out] lvgl_display_handle: Pointer to the handle of the initialized LVGL display.
 * @return
 *      - Tick value
 */

static uint32_t lcd_msp4021_lvgl_tick_get(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000ULL);
}

/**
 * @brief  LVGL display flush callback function to draw the pixel map to the LCD panel
 *
 * @param[in] display: lvgl display handle
 * @param[in] area: area to be flushed
 * @param[in] px_map: pixel map to be flushed
 */
void lcd_msp4021_lvgl_display_flush_callback(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
{
    esp_lcd_panel_handle_t display_device_handle = (esp_lcd_panel_handle_t)lv_display_get_user_data(display);
    if (display_device_handle)
    {
        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_lcd_panel_draw_bitmap(display_device_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, px_map));
    }

    lv_display_flush_ready(display);
}

void lcd_msp4021_lvgl_display_init(esp_lcd_panel_handle_t display_handle, esp_lcd_touch_handle_t touch_handle, lv_display_t *lvgl_display_handle, lv_indev_t *lvgl_touch_handle)
{
    // 1. lvgl init
    lv_init();
    lv_tick_set_cb(lcd_msp4021_lvgl_tick_get);

    // 2. Create primary display container (main LCD screen)
    lvgl_display_handle = lv_display_create(LCD_MSP4021_DISPLAY_WIDTH, LCD_MSP4021_DISPLAY_HEIGHT);
    lv_display_set_user_data(lvgl_display_handle, (void *)display_handle);
    lv_display_set_buffers(lvgl_display_handle, lcd_msp4021_lvgl_display_buffer1, lcd_msp4021_lvgl_display_buffer2, LCD_MSP4021_DISPLAY_BUFFER_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_color_format(lvgl_display_handle, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(lvgl_display_handle, lcd_msp4021_lvgl_display_flush_callback);

    // 3. Create the base input device abstraction
    lvgl_touch_handle = lv_indev_create();
    lv_indev_set_user_data(lvgl_touch_handle, (void *)touch_handle);
    lv_indev_set_type(lvgl_touch_handle, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(lvgl_touch_handle, lcd_msp4021_lvgl_touch_read_cb);

    // 4. Use a built-in LVGL symbol as the touch pointer cursor (Optional)
    lv_obj_t *cursor_obj = lv_label_create(lv_screen_active());
    lv_label_set_text(cursor_obj, LV_SYMBOL_RIGHT);
    lv_indev_set_cursor(lvgl_touch_handle, cursor_obj);

    return;
}
/********************************************************/
/*      MSP4021 HW Module initialzation                 */
/********************************************************/
/**
 * @brief  Initialize the MSP4021 LCD panel as lvgl display with integrated touch input device.
 *
 * @param[out] lvgl_display_handle: Pointer to the handle of the initialized LVGL display.
 * @param[out] lvgl_touch_handle: Pointer to the handle of the initialzed LVGL touch device.
 * @return
 *      - ESP_OK:    Success
 *      - Otherwise: Fail
 */
esp_err_t lcd_msp4021_display_init(lv_display_t *lvgl_display_handle, lv_indev_t *lvgl_touch_handle, sdmmc_card_t **sd_fat_card_handle)
{
    esp_err_t status = ESP_OK;

    esp_lcd_panel_handle_t display_handle = NULL;
    esp_lcd_touch_handle_t touch_handle = NULL;

    // Initialize the global SPI buses for the LCD panel and touch controller
    status = lcd_msp4021_global_spi_bus_init();

    // 1. Initialze display panel controller
    if (status == ESP_OK)
    {
        status = lcd_msp4021_lcd_panel_init(&display_handle);
    }

    // 2. Initialize the touch controller
    if (status == ESP_OK)
    {
        status = lcd_msp4021_lcd_touch_init(&touch_handle);
    }

    // 3. Initialize the LVGL display with the LCD panel and touch controller
    if (status == ESP_OK)
    {
        lcd_msp4021_lvgl_display_init(display_handle, touch_handle, lvgl_display_handle, lvgl_touch_handle);
    }

    // 4. Initialize the SD FAT file system
    if (status == ESP_OK)
    {
        //        status = lcd_msp4021_sd_card_init(sd_fat_card_handle);
    }

    return status;
}
