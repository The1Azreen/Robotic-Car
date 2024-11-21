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
#include "ultrasonic.h"
#include "task_handles.h"
#include "transition.h"
#include "server.h"

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "lwip/tcp.h"
#include "lwipopts.h"
#include "lwip/udp.h"
#include "lwip/ip4_addr.h"
#include "lwip/netif.h"


// Declare task handles
TaskHandle_t motorTaskHandle = NULL;
TaskHandle_t lineFollowingTaskHandle = NULL;
TaskHandle_t barcodeTaskHandle = NULL;
TaskHandle_t wifiTaskHandle = NULL;

int main() {
    stdio_init_all();
    sleep_ms(4000);
    // Initialize hardware
    initialize_hardware();
    printf("Hardware initialized\n");
    // encoder_init();
    printf("Encoder initialized\n");
    // // Initialize motors
    motor_init();
    // printf("Motors initialized\n");
    // // Initialize ultrasonic sensor
    // ultrasonic_init();
    // printf("Ultrasonic sensor initialized\n");

    // Create wifi task
    xTaskCreate(wifi_task, "MainThread", configMINIMAL_STACK_SIZE, NULL, TEST_TASK_PRIORITY, &wifiTaskHandle);
    printf("Wifi task created\n");

// Create motor control task
    // xTaskCreate(motor_task, "MotorTask", 1024, NULL, 1, &motorTaskHandle);
    // xTaskCreate(ultrasonic_task, "UltraSonicTask", 1024, NULL, 2, NULL);
    // printf("Motor task created\n");

    
    // // Create line following task (start suspended)
    // xTaskCreate(line_following_task, "LineFollowTask", 1024, NULL, 2, &lineFollowingTaskHandle);
    // vTaskSuspend(lineFollowingTaskHandle);

    // // Create sensor monitoring task
    // xTaskCreate(sensor_task, "SensorTask", 1024, NULL, 3, NULL);
    // printf("Sensor task created\n");
 
    // xTaskCreate(barcodeTask, "BarcodeTask", 1048, NULL, 3, &barcodeTaskHandle);
    // vTaskSuspend(barcodeTaskHandle);

    // Start the FreeRTOS scheduler
    vTaskStartScheduler();

    // Should never reach here
    while (1) {}

    return 0;
}
