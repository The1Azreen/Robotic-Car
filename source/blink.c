#include "FreeRTOS.h"
#include "task.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#define DELAY 1000

void led_task_cyw43()
{   

    while (true) {
        cyw43_arch_gpio_put(0, 1);
        printf("High");
        vTaskDelay(pdMS_TO_TICKS(DELAY));
        cyw43_arch_gpio_put(0, 0);
        printf("Low");
        vTaskDelay(pdMS_TO_TICKS(DELAY));
    }
}