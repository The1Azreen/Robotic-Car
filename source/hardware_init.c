// hardware_init.c
#include "hardware_init.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "pins.h"
#include "encoder.h"
#include "motor.h"
#include <stdio.h>
// hardware_init.c (Add these at the top)
#include "encoder.h"
// Function to set up PWM for motor enable pins
void setup_pwm(uint gpio_pin, uint slice_num) {
    gpio_set_function(gpio_pin, GPIO_FUNC_PWM);
    pwm_set_wrap(slice_num, 255);
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 0);
    pwm_set_enabled(slice_num, true);
}

// Hardware initialization function
void initialize_hardware() {
    stdio_init_all();
    printf("Initializing Robot with Encoder Feedback...\n");

    // Initialize motor control pins
    gpio_init(L_MOTOR_IN1);
    gpio_set_dir(L_MOTOR_IN1, GPIO_OUT);
    gpio_init(L_MOTOR_IN2);
    gpio_set_dir(L_MOTOR_IN2, GPIO_OUT);
    gpio_init(R_MOTOR_IN3);
    gpio_set_dir(R_MOTOR_IN3, GPIO_OUT);
    gpio_init(R_MOTOR_IN4);
    gpio_set_dir(R_MOTOR_IN4, GPIO_OUT);

    // Initialize PWM for motor enable pins
    setup_pwm(L_MOTOR_ENA, pwm_gpio_to_slice_num(L_MOTOR_ENA));
    setup_pwm(R_MOTOR_ENB, pwm_gpio_to_slice_num(R_MOTOR_ENB));

    // Initialize encoder pins
    gpio_init(L_ENCODER_PIN);
    gpio_set_dir(L_ENCODER_PIN, GPIO_IN);
    gpio_pull_up(L_ENCODER_PIN);
    gpio_set_irq_enabled_with_callback(L_ENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &l_encoder_isr);

    gpio_init(R_ENCODER_PIN);
    gpio_set_dir(R_ENCODER_PIN, GPIO_IN);
    gpio_pull_up(R_ENCODER_PIN);
    gpio_set_irq_enabled_with_callback(R_ENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &r_encoder_isr);

    // Initialize IR sensor pin
    gpio_init(SENSOR_PIN);
    gpio_set_dir(SENSOR_PIN, GPIO_IN);

    printf("Hardware Initialization Complete.\n");
}
