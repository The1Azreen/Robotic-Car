/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "lwip/ip4_addr.h"
#include "lwip/netif.h"
#include "lwip/udp.h"

#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"

#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include <math.h>
#include <time.h>

// WIFI Configuration

#define UDP_PORT 4242
#define SERVER_IP "255.255.255.255"  // IP address of the server EDIT THIS !!! (use ipconfig on the server to get the IP address)


// I2C Configuration
#define I2C_PORT i2c0
#define SDA_PIN 4   // Changed from 0 to 4
#define SCL_PIN 5   // Changed from 1 to 5
#define ACCELEROMETER_ADDR 0x19
#define CTRL_REG1_A 0x20
#define CTRL_REG4_A 0x23
#define OUT_X_L_A 0x28

// The LSM303DLHC in ±4g range has a sensitivity of 2mg/LSB (LSB = Least Significant Bit)
// 1g = 1000mg = 500 LSB
#define SENSITIVITY_4G 2.0f/1000.0f  // Convert raw to g (±4g range)
#define TILT_THRESHOLD 3.6f          // Trigger tilt at 3.6g
#define SAMPLE_RATE 250              // ms between readings

#ifndef RUN_FREERTOS_ON_CORE
#define RUN_FREERTOS_ON_CORE 0
#endif

#define TEST_TASK_PRIORITY				( tskIDLE_PRIORITY + 1UL )

static struct udp_pcb *udp_client_pcb;
static ip_addr_t server_ip;

typedef struct {
    char direction[10];
    int speed;
} DataPacket;

void main_task(__unused void *params) {
    if (cyw43_arch_init()) {
        printf("failed to initialise\n");
        return;
    }
    cyw43_arch_enable_sta_mode();
    printf("Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed to connect.\n");
        exit(1);
    } else {
        printf("Connected to wifi.\n");
    }

    udp_client_pcb = udp_new();
    if (!udp_client_pcb) {
        printf("Client: Error creating PCB.\n");
        return;
    }

    ip4addr_aton(SERVER_IP, &server_ip);

    while(true) {
        vTaskDelay(100);
    }

    cyw43_arch_deinit();
}

void acc_init() {
    uint8_t config_data[2];

    // Set CTRL_REG1_A: 100Hz data rate, XYZ axes enabled
    config_data[0] = CTRL_REG1_A;
    config_data[1] = 0x57;
    i2c_write_blocking(I2C_PORT, ACCELEROMETER_ADDR, config_data, 2, false);

    // Set CTRL_REG4_A: ±2g range
    config_data[0] = CTRL_REG4_A;
    config_data[1] = 0x10;
    i2c_write_blocking(I2C_PORT, ACCELEROMETER_ADDR, config_data, 2, false);
}

// Function to apply a low-pass filter to accelerometer data
void low_pass_filter(float *ax_g, float *ay_g, float *az_g, float alpha) {
    static float prev_ax_g = 0.0f, prev_ay_g = 0.0f, prev_az_g = 0.0f;

    *ax_g = alpha * (*ax_g) + (1.0f - alpha) * prev_ax_g;
    *ay_g = alpha * (*ay_g) + (1.0f - alpha) * prev_ay_g;
    *az_g = alpha * (*az_g) + (1.0f - alpha) * prev_az_g;

    prev_ax_g = *ax_g;
    prev_ay_g = *ay_g;
    prev_az_g = *az_g;
}

// Function to read and convert accelerometer data to g-forces
void acc_read_g(float *ax_g, float *ay_g, float *az_g) {
    uint8_t reg = OUT_X_L_A | 0x80;  // Auto-increment
    uint8_t buf[6];
    int16_t ax, ay, az;

    i2c_write_blocking(I2C_PORT, ACCELEROMETER_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, ACCELEROMETER_ADDR, buf, 6, false);

    // Convert to 16-bit integers
    ax = (int16_t)((buf[1] << 8) | buf[0]);
    ay = (int16_t)((buf[3] << 8) | buf[2]);
    az = (int16_t)((buf[5] << 8) | buf[4]);

    // Convert to g-forces
    *ax_g = ax * SENSITIVITY_4G;
    *ay_g = ay * SENSITIVITY_4G;
    *az_g = az * SENSITIVITY_4G;

    // Apply low-pass filter
    low_pass_filter(ax_g, ay_g, az_g, 0.5f);  // Adjust alpha as needed
}

// Function to calculate tilt angles in degrees -> EMA
void calculate_tilt_angles(float ax_g, float ay_g, float az_g, float *pitch, float *roll) {
    // Calculate pitch (rotation around Y-axis) and roll (rotation around X-axis)
    *pitch = atan2f(ax_g, sqrtf(ay_g * ay_g + az_g * az_g)) * 180.0f / M_PI; // Ax/sqrt(Ay^2 + Az^2) -> convert to degrees
    *roll = atan2f(ay_g, sqrtf(ax_g * ax_g + az_g * az_g)) * 180.0f / M_PI;
}



void send_data(char direction[10], int speed) {

    if (udp_client_pcb == NULL) {
        printf("Client: UDP PCB not initialized.\n");
        return;
    }

    // Create the data packet
    DataPacket packet;
    strcpy(packet.direction, direction);
    packet.speed = speed;

    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, sizeof(packet), PBUF_RAM);
    if (!p) {
        printf("Client: Error allocating pbuf.\n");
        return;
    }

    memcpy(p->payload, &packet, sizeof(packet));

    static char prev_direction[10] = "";
    static int prev_speed = -1;

    if (udp_sendto(udp_client_pcb, p, &server_ip, UDP_PORT) != ERR_OK) {
        printf("Client: Error sending data.\n");
    } else {
        if (strcmp(prev_direction, packet.direction) != 0 || prev_speed != packet.speed) {
        printf("Client: Sent data: Direction=%s, Speed=%d\n", 
                packet.direction, packet.speed);
        }
        strcpy(prev_direction, packet.direction);
        prev_speed = packet.speed;
    }


    pbuf_free(p);
}

void get_direction(float ax_g, float ay_g, float pitch, float roll) {

    char direction[10];
    int speed = 0;

    if (ax_g < -TILT_THRESHOLD) {
        strcpy(direction, "Forward");
        if (pitch < -70) speed = 4;
        else if (pitch < -45) speed = 3;
        else if (pitch < -25) speed = 2;
        else if (pitch < -10) speed = 1;
        else speed = 1;
    } else if (ax_g > TILT_THRESHOLD) {
        strcpy(direction, "Backward");
        if (pitch > 70) speed = 4;
        else if (pitch > 45) speed = 3;
        else if (pitch > 25) speed = 2;
        else if (pitch > 10) speed = 1;
        else speed = 1;
    } else if (ay_g < -TILT_THRESHOLD) {
        strcpy(direction, "Left");
        if (roll < -70) speed = 4;
        else if (roll < -45) speed = 3;
        else if (roll < -25) speed = 2;
        else if (roll < -10) speed = 1;
        else speed = 1;
    } else if (ay_g > TILT_THRESHOLD) {
        strcpy(direction, "Right");
        if (roll > 70) speed = 4;
        else if (roll > 45) speed = 3;
        else if (roll > 25) speed = 2;
        else if (roll > 10) speed = 1;
        else speed = 1;
    } else {
        strcpy(direction, "Neutral");
        speed = 0;
    }

    send_data(direction, speed);
}

void accelerometer_task(__unused void *params) {

    // Initialize I2C
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    // Initialize accelerometer
    acc_init();
    sleep_ms(500);

    float ax_g, ay_g, az_g;
    float pitch, roll;

    while (1) {
        // Read accelerometer (in g-forces) and apply low pass filter
        acc_read_g(&ax_g, &ay_g, &az_g);
        
        // Calculate tilt angles
        calculate_tilt_angles(ax_g, ay_g, az_g, &pitch, &roll);
        
        get_direction(ax_g, ay_g, pitch, roll);
        
        sleep_ms(SAMPLE_RATE);
    }
}


void vLaunch( void) {
    TaskHandle_t task;
    xTaskCreate(main_task, "MainThread", configMINIMAL_STACK_SIZE, NULL, TEST_TASK_PRIORITY, &task);

    TaskHandle_t accelerometertask;
    xTaskCreate(accelerometer_task, "MagnetometerThread", configMINIMAL_STACK_SIZE, NULL, 8, &accelerometertask);

#if NO_SYS && configUSE_CORE_AFFINITY && configNUM_CORES > 1
    // we must bind the main task to one core (well at least while the init is called)
    // (note we only do this in NO_SYS mode, because cyw43_arch_freertos
    // takes care of it otherwise)
    vTaskCoreAffinitySet(task, 1);
#endif

    /* Start the tasks and timer running. */
    vTaskStartScheduler();
}

int main( void )
{
    stdio_init_all();

    /* Configure the hardware ready to run the demo. */
    const char *rtos_name;
#if ( portSUPPORT_SMP == 1 )
    rtos_name = "FreeRTOS SMP";
#else
    rtos_name = "FreeRTOS";
#endif

#if ( portSUPPORT_SMP == 1 ) && ( configNUM_CORES == 2 )
    printf("Starting %s on both cores:\n", rtos_name);
    vLaunch();
#elif ( RUN_FREERTOS_ON_CORE == 1 )
    printf("Starting %s on core 1:\n", rtos_name);
    multicore_launch_core1(vLaunch);
    while (true);
#else
    printf("Starting %s on core 0:\n", rtos_name);
    vLaunch();
#endif
    return 0;
}
