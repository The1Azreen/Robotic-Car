#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <stdint.h>
#include <stdbool.h>

//Ultrasonic Pins
#define TRIG_PIN 0
#define ECHO_PIN 1

// Function prototypes
void ultrasonic_init();
uint64_t ultrasonic_get_pulse();
double ultrasonic_get_distance();
void emergency_brake();
void ultrasonic_task(void *pvParameters);

#endif // ULTRASONIC_H