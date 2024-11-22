#include "FreeRTOS.h"
#include "task.h"
#include "motor.h"
#include "hardware_init.h"
#include <stdio.h>
#include "server.h"
#include "ultrasonic.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "server.h"
#include "barcode.h"
#include "line.h"


// Declare task handles
TaskHandle_t wifiTaskHandle = NULL;
TaskHandle_t lineFollinwgTaskHandle = NULL;
TaskHandle_t barcodeTaskHandle= NULL;
bool sensor_activated = false;

// Forward declaration of wifi_task
void wifi_task(void *params);


void sensor_task(void *pvParameters) {
    bool sensor_fla = false;
    printf("Starting sensor task\n");
    while (1) {
        // Read IR sensor
        bool sensor_value = gpio_get(SENSOR_PIN);
        set_sensor_flag(true);
        set_threshold_distance(50.0);
        printf("Sensor value: %d\n", sensor_value);
        //print sensor flag
        printf("Sensor flag: %d\n", get_sensor_flag());
        //print obstacle flag
        printf("Obstacle flag: %d\n", get_obstacle_flag());
        
        if (sensor_value == 1 && !sensor_activated) {
            printf("IR sensor activated. Switching to line following mode & barcode reading.\n");
            // Flag enable to disable wifi task
            printf("Priority of wifi task: %d\n", uxTaskPriorityGet(wifiTaskHandle));
            vTaskPrioritySet(wifiTaskHandle, 0);
            set_sensor_flag(false);
            sensor_activated = true;
            // print priority of wifi task
            printf("Priority of wifi task: %d\n", uxTaskPriorityGet(wifiTaskHandle));
            //resume the line following task
            vTaskResume(lineFollinwgTaskHandle);
            //resume the barcode task
            vTaskResume(barcodeTaskHandle);
            vTaskDelete(NULL);
        }

        // Delay before next read
        vTaskDelay(pdMS_TO_TICKS(100)); // Adjust as needed
    }
}


int main() {
    stdio_init_all();
    sleep_ms(4000);

    // Initialize hardware
    initialize_hardware();
    motor_init();
    
    sleep_ms(2000);

    // Create Wi-Fi task with sufficient stack size
    xTaskCreate(wifi_task, "WifiTask", 8192, NULL, 3, &wifiTaskHandle);
    printf("Wi-Fi task created\n");

    xTaskCreate(ultrasonic_task, "UltrasonicTask", 4096, NULL, 2, NULL);
    printf("Ultrasonic task created\n");
    xTaskCreate(sensor_task, "SensorTask", 2048, NULL, 4, NULL);

    xTaskCreate(line_following_task, "lineFollowingTask", 2048, NULL, 3, &lineFollinwgTaskHandle);
    //suspend the line following task
    vTaskSuspend(lineFollinwgTaskHandle);
    xTaskCreate(barcode_task, "BarcodeTask", 2048, NULL, 4, &barcodeTaskHandle);
    //suspend the barcode task
    vTaskSuspend(barcodeTaskHandle);

    // Start the FreeRTOS scheduler
    vTaskStartScheduler();

    // Should never reach here
    while (1) {}
}
