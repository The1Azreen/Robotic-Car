// transition.c
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "pins.h"
#include "task_handles.h"
#include "transition.h"

void sensor_task(void *pvParameters) {
    bool previous_sensor_state = false;
    printf("Starting sensor task\n");

    while (1) {
        // Read IR sensor
        bool sensor_value = gpio_get(SENSOR_PIN);
        printf("Sensor value: %d\n", sensor_value);
        /*
        if (sensor_value && !previous_sensor_state) {
            // Line detected
            printf("IR sensor activated. Switching to line following mode & barcode reading.\n");
            if (motorTaskHandle != NULL) {
                vTaskSuspend(motorTaskHandle);
            }

            if (lineFollowingTaskHandle != NULL && barcodeTaskHandle != NULL) {
                printf("Resuming line following and barcode reading tasks\n");
                vTaskResume(lineFollowingTaskHandle);
                vTaskResume(barcodeTaskHandle);

                printf("Suspending sensor task\n");
                vTaskSuspend(NULL);  // Suspend itself
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
        */
        // Delay before next read
        vTaskDelay(pdMS_TO_TICKS(100)); // Adjust as needed
    }

}
