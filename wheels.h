#ifndef WHEELS_H_
#define WHEELS_H_

#include "motor.h"
#include "pins.h"
#include "hafizencoder.h"
#include "pico/stdlib.h"
#include "hardware/timer.h"

enum CAR_STATE {
    CAR_STATIONARY = 0,
    CAR_FORWARD,
    CAR_BACKWARD,
    CAR_TURN_RIGHT,
    CAR_TURN_LEFT,
    CAR_TURN_LEFT_FORWARD,
    CAR_TURN_RIGHT_FORWARD,
    NUM_CAR_STATES
};

typedef struct PID_VAR {
    float current_speed;
    float target_speed;
    float duty_cycle;
    float integral;
    float prev_error;
    bool turning;
} PID_VAR;

// Variables for controlling PID speed
PID_VAR pid_left = {
    .current_speed = 1.0f,
    .target_speed = 1.0f,
    .duty_cycle = 0.0f,
    .integral = 0.0f,
    .prev_error = 0.0f,
    .turning = false
};

PID_VAR pid_right = {
    .current_speed = 1.0f,
    .target_speed = 1.0f,
    .duty_cycle = 0.0f,
    .integral = 0.0f,
    .prev_error = 0.0f,
    .turning = false
};

struct repeating_timer pid_timer;

/// @brief Initializes all pins and PWM for both motors
void wheels_init();

/// @brief Sets the car state (e.g., stationary, forward, backward)
/// @param next_state Next car state
void wheels_set_car_state(uint8_t next_state);

/// @brief Sets the duty cycle of both wheels
/// @param duty_cycle Duty cycle value
void wheels_set_duty_cycle(float duty_cycle);

/// @brief Sets the duty cycle for the left wheel
/// @param duty_cycle Duty cycle value
void wheels_set_left_duty_cycle(float duty_cycle);

/// @brief Sets the duty cycle for the right wheel
/// @param duty_cycle Duty cycle value
void wheels_set_right_duty_cycle(float duty_cycle);

/// @brief Computes wheel duty cycle based on PID
/// @param pid Pointer to PID_VAR structure
void wheels_compute_duty_cycle(PID_VAR *pid);

/// @brief PID timer callback function
/// @param t Pointer to repeating_timer structure
bool wheels_pid_timer_callback(struct repeating_timer *t);

/// @brief Starts the PID control timer
void wheels_start_pid();

/// @brief Stops the PID control timer
void wheels_end_pid();

void wheels_init() {
    motor_setup(WHEEL_LEFT_PWM_PIN, WHEEL_LEFT_OUT_PIN_1, WHEEL_LEFT_OUT_PIN_2);
    motor_setup(WHEEL_RIGHT_PWM_PIN, WHEEL_RIGHT_OUT_PIN_1, WHEEL_RIGHT_OUT_PIN_2);

    wheels_set_car_state(CAR_STATIONARY);

    motor_set_pwm_duty_cycle(WHEEL_LEFT_PWM_PIN, 0.0f);
    motor_set_pwm_duty_cycle(WHEEL_RIGHT_PWM_PIN, 0.0f);
}

void wheels_set_car_state(uint8_t next_state) {
    switch (next_state) {
        case CAR_STATIONARY:
            motor_stop(WHEEL_LEFT_OUT_PIN_1, WHEEL_LEFT_OUT_PIN_2);
            motor_stop(WHEEL_RIGHT_OUT_PIN_1, WHEEL_RIGHT_OUT_PIN_2);
            pid_left.turning = false;
            pid_right.turning = false;
            break;
        case CAR_FORWARD:
            motor_set_direction(WHEEL_LEFT_OUT_PIN_1, WHEEL_LEFT_OUT_PIN_2, false);
            motor_set_direction(WHEEL_RIGHT_OUT_PIN_1, WHEEL_RIGHT_OUT_PIN_2, true);
            pid_left.turning = true;
            pid_right.turning = true;
            break;
        case CAR_BACKWARD:
            motor_set_direction(WHEEL_LEFT_OUT_PIN_1, WHEEL_LEFT_OUT_PIN_2, true);
            motor_set_direction(WHEEL_RIGHT_OUT_PIN_1, WHEEL_RIGHT_OUT_PIN_2, false);
            pid_left.turning = true;
            pid_right.turning = true;
            break;
        case CAR_TURN_RIGHT:
            motor_stop(WHEEL_RIGHT_OUT_PIN_1, WHEEL_RIGHT_OUT_PIN_2);
            motor_set_direction(WHEEL_LEFT_OUT_PIN_1, WHEEL_LEFT_OUT_PIN_2, false);
            pid_left.turning = true;
            pid_right.turning = false;
            break;
        case CAR_TURN_LEFT:
            motor_stop(WHEEL_LEFT_OUT_PIN_1, WHEEL_LEFT_OUT_PIN_2);
            motor_set_direction(WHEEL_RIGHT_OUT_PIN_1, WHEEL_RIGHT_OUT_PIN_2, true);
            pid_left.turning = false;
            pid_right.turning = true;
            break;
        case CAR_TURN_LEFT_FORWARD:
            motor_set_direction(WHEEL_LEFT_OUT_PIN_1, WHEEL_LEFT_OUT_PIN_2, false);
            motor_set_direction(WHEEL_RIGHT_OUT_PIN_1, WHEEL_RIGHT_OUT_PIN_2, true);
            pid_left.turning = true;
            pid_right.turning = true;
            break;
        case CAR_TURN_RIGHT_FORWARD:
            motor_set_direction(WHEEL_LEFT_OUT_PIN_1, WHEEL_LEFT_OUT_PIN_2, true);
            motor_set_direction(WHEEL_RIGHT_OUT_PIN_1, WHEEL_RIGHT_OUT_PIN_2, false);
            pid_left.turning = true;
            pid_right.turning = true;
            break;
        default:
            break;
    }
}

void wheels_set_duty_cycle(float duty_cycle) {
    wheels_set_left_duty_cycle(duty_cycle);
    wheels_set_right_duty_cycle(duty_cycle);
}

void wheels_set_left_duty_cycle(float duty_cycle) {
    if (pid_left.turning) {
        motor_set_pwm_duty_cycle(WHEEL_LEFT_PWM_PIN, duty_cycle);
        pid_left.duty_cycle = duty_cycle;
    } else {
        motor_set_pwm_duty_cycle(WHEEL_LEFT_PWM_PIN, 0.0f);
        pid_left.duty_cycle = 0.0f;
    }
}

void wheels_set_right_duty_cycle(float duty_cycle) {
    if (pid_right.turning) {
        motor_set_pwm_duty_cycle(WHEEL_RIGHT_PWM_PIN, duty_cycle);
        pid_right.duty_cycle = duty_cycle;
    } else {
        motor_set_pwm_duty_cycle(WHEEL_RIGHT_PWM_PIN, 0.0f);
        pid_right.duty_cycle = 0.0f;
    }
}

void wheels_compute_duty_cycle(PID_VAR *pid) {
    float kp = 1.0f;
    float ki = 0.5f;
    float kd = 0.1f;

    float error = pid->target_speed - pid->current_speed;
    pid->integral += error;
    float derivative = error - pid->prev_error;

    pid->duty_cycle += kp * error + ki * pid->integral + kd * derivative;

    if (pid->duty_cycle > 1.0f)
        pid->duty_cycle = 1.0f;
    else if (pid->duty_cycle < 0.0f)
        pid->duty_cycle = 0.0f;

    pid->prev_error = error;
}

bool wheels_pid_timer_callback(struct repeating_timer *t) {
    // Update current speed from encoder readings
    pid_left.current_speed = left_encoder_speed;
    pid_right.current_speed = right_encoder_speed;

    wheels_compute_duty_cycle(&pid_left);
    wheels_compute_duty_cycle(&pid_right);

    wheels_set_left_duty_cycle(pid_left.duty_cycle);
    wheels_set_right_duty_cycle(pid_right.duty_cycle);

    return true; // Continue repeating
}

void wheels_start_pid() {
    // Start a repeating timer for PID control
    add_repeating_timer_ms(10, wheels_pid_timer_callback, NULL, &pid_timer);
}

void wheels_end_pid() {
    // Cancel the PID timer
    cancel_repeating_timer(&pid_timer);
}

#endif // WHEELS_H_