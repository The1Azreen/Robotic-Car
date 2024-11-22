#include "FreeRTOS.h"
#include "task.h"
#include "motor.h"
#include "hardware_init.h"
#include <stdio.h>
#include "server.h"

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "server.h"

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
    xTaskCreate(wifi_task, "WifiTask", 2048, NULL, 1, &wifiTaskHandle);
    printf("Wi-Fi task created\n");

    // Start the FreeRTOS scheduler
    vTaskStartScheduler();

    // Should never reach here
    while (1) {}
}
