#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "KY040_rotary_encoder.h"
#include "esp_timer.h"
#include "lvgl.h"

#include "hardware_config.h"

#define KY040_ROTARY_ENCODER_GLITCH_FILTER_NS 1000

#define KY040_ROTARY_ENCODER_SW_LONG_PRESS_TIME_MS 1000 
#define KY040_ROTARY_ENCODER_ACCEL_THRESHOLD 40.0f  
#define KY040_ROTARY_ENCODER_ACCEL_FACTOR    0.05f  
#define KY040_ROTARY_ENCODER_MAX_ACCEL_MULT  5      

static pcnt_unit_handle_t pcnt_unit = NULL;

static int32_t last_raw_count = 0;
static int32_t accumulated_abs_count = 0;
static int64_t last_poll_time_us = 0;

static int64_t sw_pressed_duration_us = 0; 
static bool sw_was_pressed = false;         

esp_err_t KY040_rotary_encoder_init(void)
{
    esp_err_t status = ESP_OK;

    pcnt_unit_config_t unit_config = {
        .high_limit = 20000,
        .low_limit = -20000,
        .flags.accum_count = true,
    };

    pcnt_glitch_filter_config_t filter_config = {
        .max_glitch_ns = KY040_ROTARY_ENCODER_GLITCH_FILTER_NS,
    };

    pcnt_chan_config_t chan_a_config = {
        .edge_gpio_num = KY040_ROTARY_ENCODER_CLK_GPIO_NUM,
        .level_gpio_num = KY040_ROTARY_ENCODER_DT_GPIO_NUM,
    };

    pcnt_chan_config_t chan_b_config = {
        .edge_gpio_num = KY040_ROTARY_ENCODER_DT_GPIO_NUM,
        .level_gpio_num = KY040_ROTARY_ENCODER_CLK_GPIO_NUM,
    };

    pcnt_channel_handle_t pcnt_chan_a = NULL;
    pcnt_channel_handle_t pcnt_chan_b = NULL;

    gpio_config_t sw_io_conf = {
        .pin_bit_mask = (1ULL << KY040_ROTARY_ENCODER_SW_GPIO_NUM),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    status = pcnt_new_unit(&unit_config, &pcnt_unit);
    if (status == ESP_OK) status = pcnt_unit_set_glitch_filter(pcnt_unit, &filter_config);
    if (status == ESP_OK) status = pcnt_new_channel(pcnt_unit, &chan_a_config, &pcnt_chan_a);
    if (status == ESP_OK) status = pcnt_new_channel(pcnt_unit, &chan_b_config, &pcnt_chan_b);
    if (status == ESP_OK) status = pcnt_channel_set_edge_action(pcnt_chan_a, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE);
    if (status == ESP_OK) status = pcnt_channel_set_level_action(pcnt_chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE);
    if (status == ESP_OK) status = pcnt_channel_set_edge_action(pcnt_chan_b, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE);
    if (status == ESP_OK) status = pcnt_channel_set_level_action(pcnt_chan_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE);
    if (status == ESP_OK) status = pcnt_unit_enable(pcnt_unit);
    if (status == ESP_OK) status = pcnt_unit_clear_count(pcnt_unit);
    if (status == ESP_OK) status = pcnt_unit_start(pcnt_unit);
    if (status == ESP_OK) status = gpio_config(&sw_io_conf);
    
    if (status == ESP_OK)
    {
        last_poll_time_us = esp_timer_get_time();
        sw_pressed_duration_us = 0;
        sw_was_pressed = false;
    }

    return status;
}

esp_err_t KY040_rotary_encoder_get_status(KY040_rotary_encoder_status_t *encoder_status)
{
    esp_err_t status = ESP_OK;
    int hw_count = 0;
    int64_t now_us = 0;
    int64_t time_delta_us = 0;
    int32_t raw_step_delta = 0;

    if (!encoder_status || !pcnt_unit)
    {
        return ESP_ERR_INVALID_STATE;
    }

    status = pcnt_unit_get_count(pcnt_unit, &hw_count);
    if (status != ESP_OK) return status;

    now_us = esp_timer_get_time();
    time_delta_us = now_us - last_poll_time_us;
    last_poll_time_us = now_us;

    // Derived raw hardware difference since last call
    raw_step_delta = (int32_t)(hw_count - last_raw_count);
    last_raw_count = hw_count;

    // 1. Derive Velocity Metrics (Steps per second based on un-accelerated raw ticks)
    if (time_delta_us > 0 && raw_step_delta != 0)
    {
        encoder_status->steps_per_second = ((float)abs(raw_step_delta) / (float)time_delta_us) * 1000000.0f;
    }
    else
    {
        encoder_status->steps_per_second = 0.0f;
    }

    // 2. Compute Velocity Acceleration Multiplier
    int32_t accelerated_step_delta = raw_step_delta;
    
    if (encoder_status->steps_per_second > KY040_ROTARY_ENCODER_ACCEL_THRESHOLD)
    {
        // Calculate an exponential/linear delta gain scaling past our filter point
        float speed_excess = encoder_status->steps_per_second - KY040_ROTARY_ENCODER_ACCEL_THRESHOLD;
        int32_t multiplier = 1 + (int32_t)(speed_excess * KY040_ROTARY_ENCODER_ACCEL_FACTOR);
        
        // Clamp multiplier bounds so value mapping remains controllable
        if (multiplier > KY040_ROTARY_ENCODER_MAX_ACCEL_MULT) {
            multiplier = KY040_ROTARY_ENCODER_MAX_ACCEL_MULT;
        }
        
        accelerated_step_delta = raw_step_delta * multiplier;
    }

    // 3. Assign outputs using accelerated calculations
    encoder_status->step_delta = accelerated_step_delta;
    accumulated_abs_count += accelerated_step_delta;
    encoder_status->absolute_count = accumulated_abs_count;

    // 4. Derive Direction
    if (raw_step_delta > 0)       encoder_status->direction = KY040_ROTARY_ENCODER_DIRECTION_CLOCKWISE;
    else if (raw_step_delta < 0)  encoder_status->direction = KY040_ROTARY_ENCODER_DIRECTION_COUNTERCLOCKWISE;
    else                          encoder_status->direction = KY040_ROTARY_ENCODER_DIRECTION_STATIONARY;

    // 5. Populate Push Switch status
    bool is_currently_low = (gpio_get_level(KY040_ROTARY_ENCODER_SW_GPIO_NUM) == 0);

    if (is_currently_low)
    {
        if (!sw_was_pressed)
        {
            sw_pressed_duration_us = 0;
            sw_was_pressed = true;
            encoder_status->sw_state = KY040_ROTARY_ENCODER_SWITCH_STATE_PRESSED;
        }
        else
        {
            sw_pressed_duration_us += time_delta_us;
            if (sw_pressed_duration_us >= (int64_t)KY040_ROTARY_ENCODER_SW_LONG_PRESS_TIME_MS * 1000)
            {
                encoder_status->sw_state = KY040_ROTARY_ENCODER_SWITCH_STATE_HOLD;
            }
            else
            {
                encoder_status->sw_state = KY040_ROTARY_ENCODER_SWITCH_STATE_PRESSED;
            }
        }
    }
    else
    {
        sw_pressed_duration_us = 0;
        sw_was_pressed = false;
        encoder_status->sw_state = KY040_ROTARY_ENCODER_SWITCH_STATE_RELEASED;
    }

    return ESP_OK;
}

esp_err_t KY040_rotary_encoder_set_absolute_count(int32_t target_val)
{
    accumulated_abs_count = target_val;
    return ESP_OK;
}


// The custom callback function requested by LVGL to read hardware
static void ky040_encoder_read_cb(lv_indev_t * indev, lv_indev_data_t * data)
{
    KY040_rotary_encoder_status_t status;
    
    // Poll the driver to update metrics and fetch telemetry
    if (KY040_rotary_encoder_get_status(&status) == ESP_OK) 
    {
        // 1. Pass the accelerated step delta to LVGL
        data->enc_diff = status.step_delta;

        // 2. Pass the basic physical switch state to LVGL
        if (status.sw_state == KY040_ROTARY_ENCODER_SWITCH_STATE_PRESSED || 
            status.sw_state == KY040_ROTARY_ENCODER_SWITCH_STATE_HOLD) 
        {
            data->state = LV_INDEV_STATE_PRESSED; // Pressed
        } 
        else 
        {
            data->state = LV_INDEV_STATE_RELEASED; // Released
        }
    }
}

// Initialization function called during system startup
void register_ky040_lvgl_input_device(void)
{
    // Initialize the underlying hardware driver and PCNT
    KY040_rotary_encoder_init();

    // Register the input device with LVGL
    lv_indev_t * encoder_indev = lv_indev_create();
    lv_indev_set_type(encoder_indev, LV_INDEV_TYPE_ENCODER);
    lv_indev_set_read_cb(encoder_indev, ky040_encoder_read_cb);
    
    // Optional: Assign this encoder to a specific global navigation group
    // lv_group_t * g = lv_group_create();
    // lv_group_set_default(g);
    // lv_indev_set_group(encoder_indev, g);
}
