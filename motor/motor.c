// Control L and R motor speed

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/timer.h"
#include "motor.h"
#include "math.h"
#include "encoder.h"

extern volatile float actual_speed_l;
extern volatile float actual_speed_r;

// PID parameters
float kp = 2.0;
float ki = 2.0;
float kd = 0.0;

// PID control variables
float integral_l = 0.0;
float integral_r = 0.0;
float prev_error_l = 0.0;
float prev_error_r = 0.0;

float setpoint_speed = 15.0;

volatile float pwm_l = 2800;
volatile float pwm_r = 3150;


// Function to initialize pins for motors
void init_motor_setup()
{
    // Initialize GPIO pins for L motor control
    gpio_init(L_MOTOR_IN1);
    gpio_init(L_MOTOR_IN2);
    gpio_init(L_MOTOR_ENA);

    // Initialize GPIO pins for R motor control
    gpio_init(R_MOTOR_IN3);
    gpio_init(R_MOTOR_IN4);
    gpio_init(R_MOTOR_ENB);

    // Set GPIO pins as outputs for L motor
    gpio_set_dir(L_MOTOR_IN1, GPIO_OUT);
    gpio_set_dir(L_MOTOR_IN2, GPIO_OUT);
    gpio_set_dir(L_MOTOR_ENA, GPIO_OUT);

    // Set GPIO pins as outputs for R motor
    gpio_set_dir(R_MOTOR_IN3, GPIO_OUT);
    gpio_set_dir(R_MOTOR_IN4, GPIO_OUT);
    gpio_set_dir(R_MOTOR_ENB, GPIO_OUT);

    // Enable the EN pins
    gpio_put(L_MOTOR_ENA, 1);
    gpio_put(R_MOTOR_ENB, 1);
}

// Function to initialize PWMs for motors
void init_motor_pwm()
{
    // Set GPIO pins for ENA and ENB to PWM mode
    gpio_set_function(L_MOTOR_ENA, GPIO_FUNC_PWM);
    gpio_set_function(R_MOTOR_ENB, GPIO_FUNC_PWM);

    // Get PWM slice and channel for ENA and ENB
    uint slice_left = pwm_gpio_to_slice_num(L_MOTOR_ENA);
    uint channel_left = pwm_gpio_to_channel(L_MOTOR_ENA);
    uint slice_right = pwm_gpio_to_slice_num(R_MOTOR_ENB);
    uint channel_right = pwm_gpio_to_channel(R_MOTOR_ENB);

    // Set PWM frequency to 40kHz (125MHz / 3125)
    pwm_set_wrap(slice_left, 3125);
    pwm_set_wrap(slice_right, 3125);

    // Set clock divider to 125
    pwm_set_clkdiv(slice_left, 125);
    pwm_set_clkdiv(slice_right, 125);

    // Enable PWM for both motor channels
    pwm_set_enabled(slice_left, true);
    pwm_set_enabled(slice_right, true);
}

// Function to move forward
void move_motor(float new_pwm_l, float new_pwm_r)
{
    // //printf("UPDATING MOTOR : LEFT - %f, RIGHT - %f\n", new_pwm_l, new_pwm_r);

    // stop_motor();
    sleep_ms(50);
    // Set both motors to output high for desired PWM
    // Get PWM slice and channel for ENA and ENB
    uint slice_left = pwm_gpio_to_slice_num(L_MOTOR_ENA);
    uint channel_left = pwm_gpio_to_channel(L_MOTOR_ENA);
    uint slice_right = pwm_gpio_to_slice_num(R_MOTOR_ENB);
    uint channel_right = pwm_gpio_to_channel(R_MOTOR_ENB);

    // Set PWM frequency to 40kHz (125MHz / 3125)
    pwm_set_wrap(slice_left, 3125);
    pwm_set_wrap(slice_right, 3125);

    // Set clock divider to 125
    pwm_set_clkdiv(slice_left, 125);
    pwm_set_clkdiv(slice_right, 125);

    pwm_set_chan_level(slice_left, channel_left, new_pwm_l);
    // sleep_ms(50);
    pwm_set_chan_level(slice_right, channel_right, new_pwm_r);

    // Turn on both motors
    gpio_put(L_MOTOR_IN1, 0);
    gpio_put(L_MOTOR_IN2, 1);
    gpio_put(R_MOTOR_IN3, 0);
    gpio_put(R_MOTOR_IN4, 1);

    // Enable the enable pins
    gpio_put(L_MOTOR_ENA, 1);
    gpio_put(R_MOTOR_ENB, 1);
}

// Function to move backward
void reverse_motor(float new_pwm_l, float new_pwm_r)
{
    // stop_motor();
    sleep_ms(50);

    pwm_set_chan_level(pwm_gpio_to_slice_num(L_MOTOR_ENA), pwm_gpio_to_channel(L_MOTOR_ENA), new_pwm_l);
    pwm_set_chan_level(pwm_gpio_to_slice_num(R_MOTOR_ENB), pwm_gpio_to_channel(R_MOTOR_ENB), new_pwm_r);

    // Turn on both motors
    gpio_put(L_MOTOR_IN1, 1);
    gpio_put(L_MOTOR_IN2, 0);
    gpio_put(R_MOTOR_IN3, 1);
    gpio_put(R_MOTOR_IN4, 0);

    // Enable the enable pins
    gpio_put(L_MOTOR_ENA, 1);
    gpio_put(R_MOTOR_ENB, 1);
}

// Function to stop
void stop_motor()
{
    // Turn off all motors
    gpio_put(L_MOTOR_IN1, 0);
    gpio_put(L_MOTOR_IN2, 0);
    gpio_put(R_MOTOR_IN3, 0);
    gpio_put(R_MOTOR_IN4, 0);

    // Disable the enable pins
    gpio_put(L_MOTOR_ENA, 0);
    gpio_put(R_MOTOR_ENB, 0);
}

// Function to turn
// 0 - left, 1 - right
void turn_motor(int direction, float pwm_l, float pwm_r, int delay_ms) {
    // Set PWM values for turning
    uint slice_left = pwm_gpio_to_slice_num(L_MOTOR_ENA);
    uint slice_right = pwm_gpio_to_slice_num(R_MOTOR_ENB);

    pwm_set_chan_level(slice_left, pwm_gpio_to_channel(L_MOTOR_ENA), pwm_l);
    pwm_set_chan_level(slice_right, pwm_gpio_to_channel(R_MOTOR_ENB), pwm_r);

    // Set motor direction for turning
    if (direction == 0) { // Left turn
        gpio_put(L_MOTOR_IN1, 1);  // Left motor backward
        gpio_put(L_MOTOR_IN2, 0);
        gpio_put(R_MOTOR_IN3, 0);  // Right motor forward
        gpio_put(R_MOTOR_IN4, 1);
    } else { // Right turn
        gpio_put(L_MOTOR_IN1, 0);  // Left motor forward
        gpio_put(L_MOTOR_IN2, 1);
        gpio_put(R_MOTOR_IN3, 1);  // Right motor backward
        gpio_put(R_MOTOR_IN4, 0);
    }

    // Enable motors
    gpio_put(L_MOTOR_ENA, 1);
    gpio_put(R_MOTOR_ENB, 1);

    // Hard-coded delay for the turn (adjust based on testing)
    sleep_ms(delay_ms);

    // Stop motors after delay
    stop_motor();
    printf("Turn complete with delay: %d ms\n", delay_ms);
}

// Function to move forward for a set number of grids
void move_grids(int number_of_grids)
{
    start_tracking(number_of_grids);

    while (!complete_movement)
    {
        move_motor(pwm_l, pwm_r);
        get_grids_moved(false);
        sleep_ms(50);
    }
    // Stop once reached target grids
    stop_motor();
    // get_grids_moved(true);
}

// Function to move backwards for a set number of grids
float calculate_control_signal(float *integral, float *prev_error, float error)
{
    *integral += error;

    float derivative = error - *prev_error;

    float control_signal = kp * error + ki * (*integral) + kd * derivative;

    *prev_error = error;

    return control_signal;
}