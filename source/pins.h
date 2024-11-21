// pins.h
#ifndef PINS_H
#define PINS_H

// Define motor pins
#define L_MOTOR_IN1 3
#define L_MOTOR_IN2 2
#define L_MOTOR_ENA 4
#define R_MOTOR_IN3 9
#define R_MOTOR_IN4 8
#define R_MOTOR_ENB 6

// Encoder pins
#define L_ENCODER_PIN 5
#define R_ENCODER_PIN 7

// Sensor pin
#define SENSOR_PIN 26  // GPIO pin for the IR sensor

#define BARCODE_IR_SENSOR_PIN 27
#define RESET_BUTTON_PIN 20 // GP20 for reset button

#define TRIG_PIN 0
#define ECHO_PIN 1
#define E_BRAKE_LED_PIN 25  // Built-in LED on many RP2040 boards, like Raspberry Pi Pico


#endif // PINS_H
