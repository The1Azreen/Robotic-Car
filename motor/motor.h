#ifndef MOTOR_H
#define MOTOR_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

// Define motor pins
#define L_MOTOR_IN1 3  // GPIO pin for L motor input 1
#define L_MOTOR_IN2 2  // GPIO pin for L motor input 2
#define L_MOTOR_ENA 4  // GPIO pin for L motor enable
#define R_MOTOR_IN3 9  // GPIO pin for R motor input 1
#define R_MOTOR_IN4 8  // GPIO pin for R motor input 2
#define R_MOTOR_ENB 6  // GPIO pin for R motor enable
#define PWM_MIN 1600
#define PWM_MAX 3125

// External variables
extern volatile float pwm_l;
extern volatile float pwm_r;

// Functions for motors
void init_motor_setup();
void init_motor_pwm();
void move_motor(float pwm_l, float pwm_r);
void reverse_motor(float pwm_l, float pwm_r);
void stop_motor();
void turn_motor(int direction, float pwm_l, float pwm_r, int delay_ms);
void update_motor_speed();
void move_grids(int number_of_grids);
float calculate_control_signal(float *integral, float *prev_error, float error);

#endif