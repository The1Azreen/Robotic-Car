#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#define DELAY 1000

void print_task_cyw43()
{   

    while (true) {
        printf("High");
        sleep_ms(DELAY);
        printf("Low");
    }
}