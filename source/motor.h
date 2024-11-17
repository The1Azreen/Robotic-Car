// motor.h
#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>
#include <stdbool.h>
#include "pins.h"

// Direction enumeration
typedef enum {
    DIRECTION_FORWARD,
    DIRECTION_BACKWARD,
    DIRECTION_LEFT,
    DIRECTION_RIGHT
} Direction;

// Define the minimum and maximum speed PWM values
#define MINIMUM_SPEED 120  // Adjust based on testing
#define MAX_SPEED 255

// Function declarations
void motor_init(void);
void move_robot(Direction direction, uint8_t speed_level);
void stop_motors(void);
void left_motor(bool forward, uint16_t speed);
void right_motor(bool forward, uint16_t speed);

#endif // MOTOR_H
