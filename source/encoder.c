// encoder.c
#include "encoder.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "pins.h"
#include <stdio.h>

// Initialize encoder counts
volatile int l_encoder_count = 0;
volatile int r_encoder_count = 0;

// Encoder ISR for left motor
void l_encoder_isr(uint gpio, uint32_t events) {
    l_encoder_count++;
}

// Encoder ISR for right motor
void r_encoder_isr(uint gpio, uint32_t events) {
    r_encoder_count++;
}

void encoder_init(void) {
    // Initialize left encoder pin
    gpio_init(L_ENCODER_PIN);
    gpio_set_dir(L_ENCODER_PIN, GPIO_IN);
    gpio_pull_up(L_ENCODER_PIN);
    gpio_set_irq_enabled_with_callback(L_ENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &l_encoder_isr);

    // Initialize right encoder pin
    gpio_init(R_ENCODER_PIN);
    gpio_set_dir(R_ENCODER_PIN, GPIO_IN);
    gpio_pull_up(R_ENCODER_PIN);
    gpio_set_irq_enabled_with_callback(R_ENCODER_PIN, GPIO_IRQ_EDGE_RISE, true, &r_encoder_isr);
}

// Function to read and reset encoder counts
int read_and_reset_encoder(volatile int *encoder_count) {
    int count = *encoder_count;
    *encoder_count = 0;
    return count;
}
// Function to calculate and print distance traveled in cm
void print_distance_cm(void *pvParameters) {
    const float counts_per_rev = 360.0; // Adjust based on your encoder
    const float wheel_diameter_cm = 10.0; // Adjust based on your wheel
    const float pi = 3.1415926535;

    int left_counts = read_and_reset_encoder(&l_encoder_count);
    int right_counts = read_and_reset_encoder(&r_encoder_count);

    float left_distance = (left_counts / counts_per_rev) * (pi * wheel_diameter_cm);
    float right_distance = (right_counts / counts_per_rev) * (pi * wheel_diameter_cm);

    printf("Left Distance: %.2f cm, Right Distance: %.2f cm\n", left_distance, right_distance);
}