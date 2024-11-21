#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "ultrasonic.h"
#include "FreeRTOS.h"
#include "task.h"
#include "motor.h"
#include "task_handles.h"



volatile absolute_time_t start_time;
volatile uint64_t pulse_width = 0;
bool obstacle_detected = false;


void echo_pulse_handler(uint gpio, uint32_t events)
{
    if (gpio == ECHO_PIN && events & GPIO_IRQ_EDGE_RISE)
    {
        // Rising edge detected, start the timer
        start_time = get_absolute_time();
    }
    else if (gpio == ECHO_PIN && events & GPIO_IRQ_EDGE_FALL)
    {
        // Falling edge detected, calculate the pulse width
        pulse_width = absolute_time_diff_us(start_time, get_absolute_time());
    }
}

void ultrasonic_init()
{
    gpio_init(TRIG_PIN);
    gpio_init(ECHO_PIN);
    gpio_set_dir(TRIG_PIN, GPIO_OUT);
    gpio_set_dir(ECHO_PIN, GPIO_IN);
}

uint64_t ultrasonic_get_pulse()
{
    gpio_put(TRIG_PIN, 1);
    sleep_ms(10);  // Replace vTaskDelay with sleep_ms
    gpio_put(TRIG_PIN, 0);
    sleep_ms(1);

    return pulse_width;
}

double ultrasonic_get_distance()
{
    uint64_t pulse_length = ultrasonic_get_pulse();
    double measured = pulse_length / 29.0 / 2.0;

    if (measured < 50) // Threshold in centimeters
    {
        obstacle_detected = true;
        printf("Obstacle detected at %.2f cm. Activating Emergency Brake!\n", measured);
        
        // Call the emergency brake function
        emergency_brake();
    }

    return measured;
}

#define OBSTACLE_REMOVAL_THRESHOLD 5
static int removal_counter = 0;

bool is_obstacle_removed() {
    double measured = ultrasonic_get_distance();
    if (measured >= 50) {
        removal_counter++;
        if (removal_counter >= OBSTACLE_REMOVAL_THRESHOLD) {
            removal_counter = 0;
            return true;
        }
    } else {
        removal_counter = 0;
    }
    return false;
}


volatile bool halt_motors = false;

void emergency_brake() {
    printf("Emergency Brake Activated!\nSuspending motor, barcode,linefollowing\n");
    halt_motors = true;
    vTaskSuspend(motorTaskHandle);
    vTaskSuspend(barcodeTaskHandle);
    vTaskSuspend(lineFollowingTaskHandle);
    stop_motors();

    while (halt_motors) {
        gpio_put(E_BRAKE_LED_PIN, 1);
        sleep_ms(500);
        gpio_put(E_BRAKE_LED_PIN, 0);
        sleep_ms(500);

        if (is_obstacle_removed()) {
            printf("Obstacle removed. Resuming operation...\n");
            halt_motors = false;
        }
        vTaskDelay(10);
    }
    printf("Emergency Brake Deactivated!\nResuming motor, barcode,linefollowing\n");
    vTaskResume(motorTaskHandle);
    vTaskResume(barcodeTaskHandle);
    vTaskResume(lineFollowingTaskHandle);
}


void ultrasonic_task(void *pvParameters)
{
    gpio_set_irq_enabled_with_callback(ECHO_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &echo_pulse_handler);

    while (1)
    {
        double distance = ultrasonic_get_distance();
        printf("Distance: %.2f cm\n", distance);
        vTaskDelay(50);
    }
}