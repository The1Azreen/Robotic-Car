// encoder.c
#include "encoder.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "pins.h"

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
