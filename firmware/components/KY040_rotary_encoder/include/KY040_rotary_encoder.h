#pragma once

/**
 * @file KY040_rotary_encoder.h
 * @brief device and component driver for KY040 rotary encoder
 */

#include "driver/pulse_cnt.h"
#include "driver/gpio.h"
#include "esp_err.h"

#define KY040_ROTARY_ENCODER_DIRECTION_CLOCKWISE 1
#define KY040_ROTARY_ENCODER_DIRECTION_COUNTERCLOCKWISE -1
#define KY040_ROTARY_ENCODER_DIRECTION_STATIONARY 0

#define KY040_ROTARY_ENCODER_SWITCH_STATE_PRESSED 1
#define KY040_ROTARY_ENCODER_SWITCH_STATE_RELEASED 0
#define KY040_ROTARY_ENCODER_SWITCH_STATE_HOLD 2

typedef struct {
    // 1. Bounded / Infinite Use Cases
    int32_t absolute_count;     /*!< Continuous, un-bounded counting value since initialization */
    int32_t step_delta;         /*!< Relative steps turned exactly since the last status poll */
    
    // 2. Direction Use Case
    int8_t direction;           /*!< Current rotation direction: +1 (Clockwise), -1 (Counter-Clockwise), 0 (Stationary) */
    
    // 3. Speed / Motor Use Cases
    float steps_per_second;     /*!< Angular velocity calculated over the tracking window time delta */
    
    // 4. Switch Profile Use Cases
    uint8_t sw_state;            /*!< Real-time instantaneous state of physical button */
} KY040_rotary_encoder_status_t;

/**
 * @brief Initialize consolidated hardware resources
 */
esp_err_t KY040_rotary_encoder_init(void);

/**
 * @brief Poll the driver to update metrics and pull unified application telemetry
 */
esp_err_t KY040_rotary_encoder_get_status(KY040_rotary_encoder_status_t *status);

/**
 * @brief Explicitly override or reset the current absolute tracking index position
 */
esp_err_t KY040_rotary_encoder_set_absolute_count(int32_t target_val);