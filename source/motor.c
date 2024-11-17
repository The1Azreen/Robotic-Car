// motor.c
#include "motor.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include <stdio.h>

// Function to initialize motors
void motor_init(void) {
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
    gpio_set_function(L_MOTOR_ENA, GPIO_FUNC_PWM);
    uint slice_num_left = pwm_gpio_to_slice_num(L_MOTOR_ENA);
    pwm_set_wrap(slice_num_left, 255);
    pwm_set_chan_level(slice_num_left, PWM_CHAN_A, 0);
    pwm_set_enabled(slice_num_left, true);

    gpio_set_function(R_MOTOR_ENB, GPIO_FUNC_PWM);
    uint slice_num_right = pwm_gpio_to_slice_num(R_MOTOR_ENB);
    pwm_set_wrap(slice_num_right, 255);
    pwm_set_chan_level(slice_num_right, PWM_CHAN_A, 0);
    pwm_set_enabled(slice_num_right, true);
}

// Internal function to control left motor
void left_motor(bool forward, uint16_t speed) {
    if (forward) {
        gpio_put(L_MOTOR_IN1, false);
        gpio_put(L_MOTOR_IN2, true);
    } else {
        gpio_put(L_MOTOR_IN1, true);
        gpio_put(L_MOTOR_IN2, false);
    }
    pwm_set_chan_level(pwm_gpio_to_slice_num(L_MOTOR_ENA), PWM_CHAN_A, speed);
}

// Internal function to control right motor
void right_motor(bool forward, uint16_t speed) {
    if (forward) {
        gpio_put(R_MOTOR_IN3, false);
        gpio_put(R_MOTOR_IN4, true);
    } else {
        gpio_put(R_MOTOR_IN3, true);
        gpio_put(R_MOTOR_IN4, false);
    }
    pwm_set_chan_level(pwm_gpio_to_slice_num(R_MOTOR_ENB), PWM_CHAN_A, speed);
}

// Function to move robot in a given direction at a speed level
// Function to move robot in a given direction at a speed level
void move_robot(Direction direction, uint8_t speed_level) {
    // Ensure speed level is within valid range
    if (speed_level < 1) speed_level = 1;
    if (speed_level > 5) speed_level = 5;

    // Map speed level to PWM value
    uint16_t speed = MINIMUM_SPEED + ((MAX_SPEED - MINIMUM_SPEED) / 4) * (speed_level - 1);
    
    // Ensure speed adheres to the minimum threshold
    if (speed < MINIMUM_SPEED) {
        speed = MINIMUM_SPEED;
    }
    

    switch (direction) {
        case DIRECTION_FORWARD:
            left_motor(true, speed);
            right_motor(true, speed);
            printf("Moving FORWARD - Speed Level: %d, PWM Speed: %d\n", speed_level, speed);
            break;
        case DIRECTION_BACKWARD:
            left_motor(false, speed);
            right_motor(false, speed);
            printf("Moving BACKWARD - Speed Level: %d, PWM Speed: %d\n", speed_level, speed);
            break;
        case DIRECTION_LEFT:
            // For left turn, reduce speed of left motor
            left_motor(true, speed / 2);
            right_motor(true, speed);
            printf("Turning LEFT - Speed Level: %d, PWM Speeds: L=%d, R=%d\n", speed_level, speed / 2, speed);
            break;
        case DIRECTION_RIGHT:
            // For right turn, reduce speed of right motor
            left_motor(true, speed);
            right_motor(true, speed / 2);
            printf("Turning RIGHT - Speed Level: %d, PWM Speeds: L=%d, R=%d\n", speed_level, speed, speed / 2);
            break;
        default:
            // Stop motors if invalid direction
            stop_motors();
            printf("Invalid Direction - Stopping Motors\n");
            break;
    }
}


// Function to stop both motors
void stop_motors(void) {
    left_motor(true, 0);
    right_motor(true, 0);
}
