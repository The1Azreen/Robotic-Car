#ifndef ENCODER_H_
#define ENCODER_H_

#include <stdint.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pins.h"

#define CM_PER_NOTCH 1.005
#define TIMEOUT_THRESHOLD 1500000

static int left_stop_counter = 0;
static int right_stop_counter = 0;

// Left wheel measurement data
volatile uint32_t left_notch_count = 0;
volatile double left_total_distance = 0.0;
volatile uint64_t left_last_notch_time = 0;
volatile double left_encoder_speed = 0.0;
volatile float left_pulse_width = 0.0f;

// Right wheel measurement data
volatile uint32_t right_notch_count = 0;
volatile double right_total_distance = 0.0;
volatile uint64_t right_last_notch_time = 0;
volatile double right_encoder_speed = 0.0;
volatile float right_pulse_width = 0.0f;

/// @brief Prints current encoder data for both wheels
static inline void encoder_print_data(void) {
    printf("Left Wheel - Notch Count: %u, Distance: %.4f cm, Speed: %.4f cm/s\n",
           left_notch_count, left_total_distance, left_encoder_speed);
    printf("Right Wheel - Notch Count: %u, Distance: %.4f cm, Speed: %.4f cm/s\n",
           right_notch_count, right_total_distance, right_encoder_speed);
}

/// @brief Encoder callback function for GPIO interrupts
/// @param gpio GPIO pin number
/// @param events GPIO events
void encoder_callback(uint gpio, uint32_t events) {
    uint64_t current_time = time_us_64();

    if (gpio == WHEEL_ENCODER_LEFT_PIN) {
        uint64_t time_diff = current_time - left_last_notch_time;
        if (time_diff > 0 && time_diff < TIMEOUT_THRESHOLD) {
            left_notch_count++;
            left_total_distance = (double)left_notch_count * CM_PER_NOTCH;
            left_encoder_speed = CM_PER_NOTCH / (time_diff / 1e6);
            left_pulse_width = time_diff / 1e6;
        } else {
            left_encoder_speed = 0.0;
        }
        left_last_notch_time = current_time;
    } else if (gpio == WHEEL_ENCODER_RIGHT_PIN) {
        uint64_t time_diff = current_time - right_last_notch_time;
        if (time_diff > 0 && time_diff < TIMEOUT_THRESHOLD) {
            right_notch_count++;
            right_total_distance = (double)right_notch_count * CM_PER_NOTCH;
            right_encoder_speed = CM_PER_NOTCH / (time_diff / 1e6);
            right_pulse_width = time_diff / 1e6;
        } else {
            right_encoder_speed = 0.0;
        }
        right_last_notch_time = current_time;
    }
}

/// @brief Checks if the wheels have stopped and updates speed accordingly
void encoder_check_if_stopped() {
    uint64_t current_time = time_us_64();

    if (current_time - left_last_notch_time > TIMEOUT_THRESHOLD) {
        left_stop_counter++;
    } else {
        left_stop_counter = 0;
    }

    if (current_time - right_last_notch_time > TIMEOUT_THRESHOLD) {
        right_stop_counter++;
    } else {
        right_stop_counter = 0;
    }

    if (left_stop_counter >= 3) {
        left_encoder_speed = 0.0;
    }

    if (right_stop_counter >= 3) {
        right_encoder_speed = 0.0;
    }
}

/// @brief Sets up encoder pins and interrupts
void encoder_setup_pins() {
    gpio_init(WHEEL_ENCODER_LEFT_PIN);
    gpio_set_dir(WHEEL_ENCODER_LEFT_PIN, GPIO_IN);

    gpio_init(WHEEL_ENCODER_RIGHT_PIN);
    gpio_set_dir(WHEEL_ENCODER_RIGHT_PIN, GPIO_IN);
}

#endif // ENCODER_H_