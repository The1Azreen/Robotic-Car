#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/gpio.h"
#include "header.h" // Don't forget to add your functions into the header file

// Sample main loop that calls a simple blink function from the source/blink.c file
int main()
{
    stdio_init_all();
    cyw43_arch_init();
    xTaskCreate(led_task_cyw43, "LED_Task", 256, NULL, 1, NULL);
    xTaskCreate(print_task_cyw43, "PRINT_Task", 256, NULL, 1, NULL);
    xTaskCreate(motor_task_cyw43, "MOTOR_Task", 256, NULL, 1, NULL);
    vTaskStartScheduler();

    while(1){
        // Do nothing
    };
}