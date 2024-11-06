// barcode_scanner.h
#ifndef BARCODE_SCANNER_H
#define BARCODE_SCANNER_H

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// Pin definitions
#define IR_SENSOR_PIN 26
#define RESET_BUTTON_PIN 21

// Scanner configuration
#define SAMPLE_SIZE 10    
#define THRESHOLD 0.5     
#define BARS_PER_CHAR 9
#define WIDE_BARS_PER_CHAR 3  
#define OUTLIER_RATIO 5.0
#define MAX_MESSAGE_LENGTH 50

// Message structure matching original CoreMessage
typedef struct {
    char pattern[BARS_PER_CHAR + 1];
    char decoded_char;
    bool is_reset_command;
} BarcodeMessage;

// This is the important part - declaring the initialization function
BaseType_t xBarcodeTaskInit(UBaseType_t uxPriority, QueueHandle_t resultQueue);

#endif // BARCODE_SCANNER_H