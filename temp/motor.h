#ifndef MOTOR_H_
#define MOTOR_H_

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdio.h>

#define PWM_WRAP 65535

void motor_setup(uint pwm_pin, uint out_pin_1, uint out_pin_2);
void motor_setup_pwm(uint pwm_pin, float duty_cycle);
void motor_set_pwm_duty_cycle(uint pwm_pin, float duty_cycle);
void motor_set_direction(uint out_pin_1, uint out_pin_2, bool clockwise);
void motor_stop(uint out_pin_1, uint out_pin_2);

void motor_setup(uint pwm_pin, uint out_pin_1, uint out_pin_2) {
    motor_setup_pwm(pwm_pin, 0.0f);
    gpio_init(out_pin_1);
    gpio_init(out_pin_2);
    gpio_set_dir(out_pin_1, GPIO_OUT);
    gpio_set_dir(out_pin_2, GPIO_OUT);
}

void motor_setup_pwm(uint pwm_pin, float duty_cycle) {
    // Set up PWM frequency and wrap value
    float clock_freq = 125000000.0f;  // Default Pico clock frequency in Hz
    uint16_t freq = 25;
    float divider = clock_freq / (freq * PWM_WRAP);  // Compute divider

    // Configure GPIO for PWM
    gpio_set_function(pwm_pin, GPIO_FUNC_PWM);

    // Get PWM slice number
    uint slice_num = pwm_gpio_to_slice_num(pwm_pin);

    pwm_set_clkdiv(slice_num, divider);
    pwm_set_wrap(slice_num, PWM_WRAP);
    motor_set_pwm_duty_cycle(pwm_pin, duty_cycle);

    pwm_set_enabled(slice_num, true);
}

void motor_set_pwm_duty_cycle(uint pwm_pin, float duty_cycle) {
    pwm_set_gpio_level(pwm_pin, (uint16_t)(duty_cycle * PWM_WRAP));
}

void motor_set_direction(uint out_pin_1, uint out_pin_2, bool clockwise) {
    gpio_put(out_pin_1, clockwise);
    gpio_put(out_pin_2, !clockwise);
}

void motor_stop(uint out_pin_1, uint out_pin_2) {
    gpio_put(out_pin_1, true);
    gpio_put(out_pin_2, true);
}

#endif // MOTOR_H_