// main.c
#include "FreeRTOS.h"
#include "task.h"
#include "motor.h"
#include "hardware_init.h"
#include <stdio.h>
#include "pins.h"
#include "encoder.h"
#include "line.h"
#include "barcode.h"
// Declare task handles
TaskHandle_t motorTaskHandle = NULL;
TaskHandle_t lineFollowingTaskHandle = NULL;
TaskHandle_t barcodeTaskHandle = NULL;

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

void sensor_task(void *pvParameters) {
    bool previous_sensor_state = false;

    while (1) {
        // Read IR sensor
        bool sensor_value = gpio_get(SENSOR_PIN);

        if (sensor_value && !previous_sensor_state) {
            // Line detected
            printf("IR sensor activated. Switching to line following mode & barcode reading.\n");
            if (motorTaskHandle != NULL) {
                vTaskSuspend(motorTaskHandle);

            }

            if (lineFollowingTaskHandle != NULL & barcodeTaskHandle != NULL) {
                printf("Resuming line following and barcode reading tasks\n");
                vTaskResume(lineFollowingTaskHandle);
                vTaskResume(barcodeTaskHandle);

                if (sensor_task != NULL) {
                printf("Suspending sensor task\n");
                vTaskSuspend(NULL);
            }

            }
        } else if (!sensor_value && previous_sensor_state) {
            // Line lost
            printf("IR sensor deactivated. NOT Switching to normal motor control.\n");
            
            if (lineFollowingTaskHandle != NULL) {
                vTaskSuspend(lineFollowingTaskHandle);
            }
            if (motorTaskHandle != NULL) {
                vTaskResume(motorTaskHandle);
            }
            
        }

        // Update previous sensor state
        previous_sensor_state = sensor_value;

        // Delay before next read
        vTaskDelay(pdMS_TO_TICKS(100)); // Adjust as needed
    }
}


int main() {
    // Initialize hardware
    initialize_hardware();

    encoder_init();

    // Initialize motors
    motor_init();
    stdio_init_all();

// Create motor control task
    sleep_ms(4000);
    xTaskCreate(motor_task, "MotorTask", 1024, NULL, 1, &motorTaskHandle);
    printf("Motor task created\n");
    // Create line following task (start suspended)
    xTaskCreate(line_following_task, "LineFollowTask", 1024, NULL, 2, &lineFollowingTaskHandle);
    vTaskSuspend(lineFollowingTaskHandle);

    // Create sensor monitoring task (higher priority)
    xTaskCreate(sensor_task, "SensorTask", 1024, NULL, 2, NULL);
 
    xTaskCreate(barcodeTask, "BarcodeTask", 1048, NULL, 3, &barcodeTaskHandle);
    vTaskSuspend(barcodeTaskHandle);

    // Start the FreeRTOS scheduler
    vTaskStartScheduler();

    // Should never reach here
    while (1) {}

    return 0;
}