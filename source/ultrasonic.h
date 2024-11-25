#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <stdint.h>
#include <stdbool.h>

//Ultrasonic Pins
#define TRIG_PIN 0
#define ECHO_PIN 1

// Define constants
#define OBSTACLE_REMOVAL_THRESHOLD 5



// Function prototypes
void ultrasonic_init();
uint64_t ultrasonic_get_pulse();
double ultrasonic_get_distance();
void emergency_brake();
void ultrasonic_task(void *pvParameters);
bool get_obstacle_flag();
void set_obstacle_flag(bool flag);
void set_distance_threshold(double threshold);
double get_distance_threshold();

#endif // ULTRASONIC_H