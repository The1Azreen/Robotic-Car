#ifndef TASK_HANDLES_H
#define TASK_HANDLES_H

#include "FreeRTOS.h"
#include "task.h"

// Declare task handles
extern TaskHandle_t motorTaskHandle;
extern TaskHandle_t lineFollowingTaskHandle;
extern TaskHandle_t barcodeTaskHandle;

#endif // TASK_HANDLES_H
