#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

#define TRIG_PIN 0
#define ECHO_PIN 1

extern volatile bool obstacle_detected;
extern volatile absolute_time_t start_time;
extern volatile uint64_t pulse_width;

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
    sleep_us(10);
    gpio_put(TRIG_PIN, 0);
    sleep_ms(1);

    return pulse_width;
}

double ultrasonic_get_distance()
{
    uint64_t pulse_length = ultrasonic_get_pulse();
    double measured = pulse_length / 29.0 / 2.0;

    if (measured < 10)
    {
        obstacle_detected = true;
    }

    return measured;
}

#endif // ULTRASONIC_H