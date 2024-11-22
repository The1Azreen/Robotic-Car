// encoder.h
#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>
#include "pico/stdlib.h"

// External variables for encoder counts
extern volatile int l_encoder_count;
extern volatile int r_encoder_count;

// Functions
void encoder_init(void);
void l_encoder_isr(uint gpio, uint32_t events);
void r_encoder_isr(uint gpio, uint32_t events);
int read_and_reset_encoder(volatile int *encoder_count);
void print_distance_cm(void *pvParameters);

#endif // ENCODER_H
