// Main program

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "pico/stdlib.h"
//#include "pico/cyw43_arch.h"
#include "hardware/adc.h"
#include "barcode.h"


// Function to init all sensors and motors
void initAll()
{
    // Initialise standard I/O
    stdio_init_all();
    sleep_ms(1000);



    // Initialise barcode sensor pin
    init_barcode();
    while (1)
    {
        track_bars();
    }
    printf("1/9 - Barcode sensor pin initialised\n");
    sleep_ms(500);

}



int main()
{
    // Init all required
    initAll();


    return 0;
}
