#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "motor.h"
#include "barcode_scanner.h"
#include "pico/stdlib.h"

// Queue for communication between tasks
#define QUEUE_LENGTH 5
#define QUEUE_ITEM_SIZE sizeof(BarcodeMessage)

// Priorities for tasks
#define MOTOR_TASK_PRIORITY (tskIDLE_PRIORITY + 2)
#define BARCODE_TASK_PRIORITY (tskIDLE_PRIORITY + 1)

// Queue handle for inter-task communication
static QueueHandle_t xBarcodeQueue = NULL;

// Motor control task prototype
void vMotorControlTask(void *pvParameters);

// Barcode scanning task prototype
void vBarcodeTask(void *pvParameters);

// Function to initialize tasks and queue
void init_system() {
    // Initialize queue for motor control and barcode communication
    xBarcodeQueue = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);
    if (xBarcodeQueue == NULL) {
        printf("Failed to create queue.\n");
        return;
    }

    // Create the motor control task
    if (xTaskCreate(vMotorControlTask, "MotorControl", configMINIMAL_STACK_SIZE * 2, NULL, MOTOR_TASK_PRIORITY, NULL) != pdPASS) {
        printf("Motor control task creation failed.\n");
    }

    // Create the barcode scanning task
    if (xTaskCreate(vBarcodeTask, "BarcodeScanner", configMINIMAL_STACK_SIZE * 2, NULL, BARCODE_TASK_PRIORITY, NULL) != pdPASS) {
        printf("Barcode scanning task creation failed.\n");
    }
}

// Motor Control Task
void vMotorControlTask(void *pvParameters) {
    BarcodeMessage barcode_msg;

    while (1) {
        // Check if there is a message in the queue
        if (xQueueReceive(xBarcodeQueue, &barcode_msg, portMAX_DELAY) == pdPASS) {
            if (barcode_msg.is_reset_command) {
                printf("Reset command received. Stopping motor.\n");
                stop_motor();
            } else {
                // Process the decoded barcode character
                char decoded_char = barcode_msg.decoded_char;
                printf("Received decoded character: %c\n", decoded_char);

                // Example: Based on barcode value, perform different motor actions
                switch (decoded_char) {
                    case 'A':
                        printf("Action A: Moving forward.\n");
                        move_motor(2500, 2500);  // Move forward
                        vTaskDelay(pdMS_TO_TICKS(1000));
                        stop_motor();
                        break;

                    case 'B':
                        printf("Action B: Turning right.\n");
                        turn_motor(1, 3000, 2000, 500); // Turn right
                        stop_motor();
                        break;

                    case 'C':
                        printf("Action C: Turning left.\n");
                        turn_motor(0, 2000, 3000, 500); // Turn left
                        stop_motor();
                        break;

                    default:
                        printf("No action defined for %c.\n", decoded_char);
                        break;
                }
            }
        }
    }
}

// Barcode Scanning Task
void vBarcodeTask(void *pvParameters) {
    // Initialize barcode scanner
    gpio_init(IR_SENSOR_PIN);
    gpio_set_dir(IR_SENSOR_PIN, GPIO_IN);
    
    BarInfo bars[BARS_PER_CHAR];
    int current_state = gpio_get(IR_SENSOR_PIN);
    int bar_count = 0;
    TickType_t bar_start_time = xTaskGetTickCount();

    while (1) {
        // State transition for barcode scanning
        int new_state = gpio_get(IR_SENSOR_PIN);
        if (new_state != current_state) {
            TickType_t current_time = xTaskGetTickCount();
            float bar_width_ms = (current_time - bar_start_time) * portTICK_PERIOD_MS;

            bars[bar_count].width = bar_width_ms;
            bars[bar_count].state = new_state;
            bar_count++;

            if (bar_count >= BARS_PER_CHAR) {
                // Process the detected bars and get decoded character
                process_bars(bars);
                bar_count = 0;

                // Check if there's a decoded message to send
                BarcodeMessage barcode_msg;
                if (in_barcode) {
                    barcode_msg.is_reset_command = false;
                    strcpy(barcode_msg.pattern, decoded_message);
                    xQueueSendToBack(xBarcodeQueue, &barcode_msg, portMAX_DELAY);
                } else {
                    // Send reset command if barcode scanning is complete
                    barcode_msg.is_reset_command = true;
                    xQueueSendToBack(xBarcodeQueue, &barcode_msg, portMAX_DELAY);
                }
            }

            bar_start_time = current_time;
            current_state = new_state;
        }

        // Small delay for FreeRTOS scheduling
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

int main() {
    stdio_init_all();
    printf("Starting FreeRTOS Barcode and Motor System\n");

    // Initialize system (tasks and queue)
    init_system();

    // Start the FreeRTOS scheduler
    vTaskStartScheduler();

    // Program should never reach here
    while (1) {}
}
