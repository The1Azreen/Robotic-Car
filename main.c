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
#include "transition.h"

// Declare task handles
TaskHandle_t wifiTaskHandle = NULL;

// Forward declaration of wifi_task
void wifi_task(void *params);

int main() {
    stdio_init_all();
    sleep_ms(4000);

    // Initialize hardware
    initialize_hardware();
    motor_init();
    
    sleep_ms(2000);

    // Create Wi-Fi task with sufficient stack size
    xTaskCreate(wifi_task, "WifiTask", 8192, NULL, 2, &wifiTaskHandle);
    printf("Wi-Fi task created\n");

    xTaskCreate(ultrasonic_task, "UltrasonicTask", 2048, NULL, 2, NULL);
    printf("Ultrasonic task created\n");
    // xTaskCreate(sensor_task, "SensorTask", 2048, NULL, 1, NULL);

    // Start the FreeRTOS scheduler
    vTaskStartScheduler();

    // Should never reach here
    while (1) {}
}
