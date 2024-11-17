// main.c
#include "FreeRTOS.h"
#include "task.h"
#include "motor.h"
#include "hardware_init.h"
#include <stdio.h>
#include "encoder.h"

void motor_task(void *pvParameters) {
    while (1) {
        for (int speed_level = 0; speed_level <= 5; speed_level++) {
            // Move forward at the current speed level
            move_robot(DIRECTION_FORWARD, speed_level);
            printf("Moving FORWARD - Speed Level: %d\n", speed_level);

            // Maintain each speed for 3 seconds
            vTaskDelay(pdMS_TO_TICKS(3000)); // Wait for 3 seconds
        }

        // Stop for 1 second before repeating
        stop_motors();
        printf("Stopping for 1 second\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


int main() {
    // Initialize hardware
    initialize_hardware();

    encoder_init();

    // Initialize motors
    motor_init();

    // Create motor control task
    xTaskCreate(motor_task, "MotorTask", 1024, NULL, 1, NULL);

    // Start the FreeRTOS scheduler
    vTaskStartScheduler();

    // Should never reach here
    while (1) {}

    return 0;
}
