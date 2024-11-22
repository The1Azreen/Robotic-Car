// line.c
#include "line.h"
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "motor.h"
#include "pins.h"
#include "encoder.h"
#include <stdio.h>
#include "encoder.h"
#include "task_handles.h"

#define MINIMUM_SPEED 100  // Adjust based on testing

// PID control constants
static const float LINE_KP = 1.0f;
static const float LINE_KI = 0.1f;
static const float LINE_KD = 0.05f;


// Target speed (in ticks per interval)
const int TARGET_SPEED = 1;

// Base motor speed
const int MOTOR_SPEED = 300;

// Variables for PID control
float l_integral = 0, l_previous_error = 0;
float r_integral = 0, r_previous_error = 0;

// PID controller function
float pid_control(float target_speed, float measured_speed, float *integral, float *previous_error) {
    float error = target_speed - measured_speed;
    *integral += error;
    float derivative = error - *previous_error;
    *previous_error = error;
    return LINE_KP * error + LINE_KI * (*integral) + LINE_KD * derivative;
}

// Line-following task
void line_following_task(void *pvParameters) {
    uint16_t l_motor_speed = 0;
    uint16_t r_motor_speed = 0;

    int line_lost_counter = 0; // Initialize line lost counter

    printf("Starting line following task\n");

    while (true) {
        //printf("Starting line following task\n");
        // Read encoder counts
        int l_ticks = read_and_reset_encoder(&l_encoder_count);
        int r_ticks = read_and_reset_encoder(&r_encoder_count);

        // Compute PID adjustments
        float l_adjustment = pid_control(TARGET_SPEED, l_ticks, &l_integral, &l_previous_error);
        float r_adjustment = pid_control(TARGET_SPEED, r_ticks, &r_integral, &r_previous_error);

        // Update motor speeds
        l_motor_speed = (uint16_t) (MOTOR_SPEED + l_adjustment);
        r_motor_speed = (uint16_t) (MOTOR_SPEED + r_adjustment);

        // Clamp speeds to 0-255 range
        if (l_motor_speed > 255) l_motor_speed = 255;
        if (r_motor_speed > 255 ) r_motor_speed = 255;

        // Read IR sensor and adjust motor control
        bool sensor_value = gpio_get(SENSOR_PIN);
        if (sensor_value) {
            // Line detected, reset counter and move forward
            line_lost_counter = 0;
            left_motor(true, l_motor_speed);
            right_motor(true, r_motor_speed);
            // printf("Moving forward: L:%d R:%d\n", l_motor_speed, r_motor_speed);
        } else {
            // Line lost
            line_lost_counter++;

            if (line_lost_counter == 1) {
                // First readjustment
                // printf("Line lost, first readjustment. Turning left.\n");

                // Adjust motors to turn left (e.g., slow down left motor)
                uint16_t adjusted_left_speed = l_motor_speed / 2;
                if (adjusted_left_speed < MINIMUM_SPEED) {
                    adjusted_left_speed = MINIMUM_SPEED;
                }
                uint16_t adjusted_right_speed = r_motor_speed;
                left_motor(true, adjusted_left_speed);
                right_motor(true, adjusted_right_speed);
                // printf("Turning left: L:%d R:%d\n", adjusted_left_speed, adjusted_right_speed);

                vTaskDelay(pdMS_TO_TICKS(10)); // Adjust delay as needed
            } else if (line_lost_counter == 2) {
                // Second readjustment
                // printf("Line lost, second readjustment. Turning left more sharply.\n");

                // Adjust motors for a sharper turn
                uint16_t adjusted_left_speed = l_motor_speed / 4;
                if (adjusted_left_speed < MINIMUM_SPEED) {
                    adjusted_left_speed = MINIMUM_SPEED;
                }
                uint16_t adjusted_right_speed = r_motor_speed;
                left_motor(true, adjusted_left_speed);
                right_motor(true, adjusted_right_speed);
                // printf("Turning left sharper: L:%d R:%d\n", adjusted_left_speed, adjusted_right_speed);

                vTaskDelay(pdMS_TO_TICKS(5)); // Adjust delay as needed
            } else if (line_lost_counter >= 3) {
                // Turn left indefinitely until line is detected again
                // printf("Line lost multiple times, turning left indefinitely until line found.\n");

                // Set motors to turn left indefinitely
                left_motor(true, 0); // Stop left motor
                right_motor(true, r_motor_speed);
                // printf("Turning left indefinitely: L:%d R:%d\n", 0, r_motor_speed);

                // No need to adjust speeds further; will continue until line is found
            }
        }

        // Delay to match control interval (e.g., 100ms)
        vTaskDelay(pdMS_TO_TICKS(0.5));
    }
}
