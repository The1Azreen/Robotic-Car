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


// Declare task handles
TaskHandle_t wifiTaskHandle = NULL;

// Forward declaration of wifi_task
void wifi_task(void *params);


void sensor_task(void *pvParameters) {
    bool sensor_activated = false;
    printf("Starting sensor task\n");

    vTaskDelay(pdMS_TO_TICKS(10000)); // Delay before starting

    while (1) {
        // Read IR sensor
        bool sensor_value = gpio_get(SENSOR_PIN);
        printf("Sensor value: %d\n", sensor_value);
        
        if (sensor_value == 0 && !sensor_activated) {
            printf("IR sensor activated. Switching to line following mode & barcode reading.\n");
            // Flag enable to disable wifi task
            printf("Priority of wifi task: %d\n", uxTaskPriorityGet(wifiTaskHandle));
            vTaskPrioritySet(wifiTaskHandle, 0);
            set_sensor_flag(true);
            sensor_activated = true;
            // print priority of wifi task
            printf("Priority of wifi task: %d\n", uxTaskPriorityGet(wifiTaskHandle));

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

    //xTaskCreate(, "", 2048, NULL, 2, NULL);

    // Start the FreeRTOS scheduler
    vTaskStartScheduler();

    // Should never reach here
    while (1) {}
}
