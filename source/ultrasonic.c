#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include <stdio.h>
#include "pins.h"
#include "task_handles.h"
#include "motor.h"

// Define constants
#define DISTANCE_THRESHOLD 20.0
#define OBSTACLE_REMOVAL_THRESHOLD 5

volatile absolute_time_t start_time;
volatile uint64_t pulse_width = 0;
volatile bool threshold_crossed = false;

// GPIO Interrupt Handler
void echo_pulse_handler(uint gpio, uint32_t events) {
    if (gpio == ECHO_PIN && events & GPIO_IRQ_EDGE_RISE) {
        start_time = get_absolute_time();
    } else if (gpio == ECHO_PIN && events & GPIO_IRQ_EDGE_FALL) {
        pulse_width = absolute_time_diff_us(start_time, get_absolute_time());
    }
}

// Initialize Ultrasonic Sensor
void ultrasonic_init() {
    gpio_init(TRIG_PIN);
    gpio_set_dir(TRIG_PIN, GPIO_OUT);
    gpio_put(TRIG_PIN, 0);

    gpio_init(ECHO_PIN);
    gpio_set_dir(ECHO_PIN, GPIO_IN);

    // Enable interrupts for rising and falling edges
    gpio_set_irq_enabled_with_callback(ECHO_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &echo_pulse_handler);
}

// Trigger Ultrasonic Pulse
void trigger_ultrasonic_pulse() {
    gpio_put(TRIG_PIN, 1);
    busy_wait_us_32(10); // Send 10 µs pulse
    gpio_put(TRIG_PIN, 0);
}

// Calculate Distance from Pulse Width
double calculate_distance() {
    return pulse_width / 29.0 / 2.0; // Convert pulse width to cm
}


// Ultrasonic Task
void ultrasonic_task(void *pvParameters) {
    int removal_counter = 0;

    while (1) {
        // Trigger ultrasonic pulse
        trigger_ultrasonic_pulse();
        vTaskDelay(pdMS_TO_TICKS(50)); // Allow time for the echo to return

        // Calculate distance
        double distance = calculate_distance();
        printf("Measured Distance: %.2f cm\n", distance);

        // Check if the obstacle is detected
        if (distance < DISTANCE_THRESHOLD) {
            removal_counter = 0; // Reset removal counter
            printf("Obstacle detected: %.2f cm\n", distance);

            // Stay in the loop, continuously stopping motors
            while (1) {
                stop_motors();

                // Trigger ultrasonic pulse again for reevaluation
                trigger_ultrasonic_pulse();
                vTaskDelay(pdMS_TO_TICKS(50)); // Wait for echo
                distance = calculate_distance();

                printf("Re-evaluating Distance: %.2f cm\n", distance);

                // Exit the loop only if obstacle is removed for 5 consecutive scans
                if (distance >= DISTANCE_THRESHOLD) {
                    removal_counter++;
                    printf("No obstacle detected (Counter: %d)\n", removal_counter);

                    if (removal_counter >= OBSTACLE_REMOVAL_THRESHOLD) {
                        printf("Obstacle cleared.\n");
                        break; // Exit the stop loop
                    }
                } else {
                    removal_counter = 0; // Reset counter if obstacle is still present
                }

                // Delay before the next reevaluation
                vTaskDelay(pdMS_TO_TICKS(100));
            }
        }

        // Delay before the next main scan if no obstacle
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}