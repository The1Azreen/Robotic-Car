/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>

#include "pico/cyw43_arch.h"
#include "motor.h"
#include "pico/stdlib.h"

#include "lwip/ip4_addr.h"
#include "lwip/netif.h"
#include "lwip/udp.h"

#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"
#include <string.h>
#include "server.h"

#include "hardware/gpio.h"
#include "hardware/adc.h"

#include "ultrasonic.h"

#define UDP_PORT 4242

#ifndef RUN_FREERTOS_ON_CORE
#define RUN_FREERTOS_ON_CORE 0
#endif

#define BUFFER_SIZE 1024  // Adjust size as needed
#define WIFI_SSID "ash (2)"
#define WIFI_PASSWORD "stick123"
static char buffer[BUFFER_SIZE];
static int buffer_len = 0;

bool sensor_flag;

void set_sensor_flag(bool flag) {
    sensor_flag = flag;
}

bool get_sensor_flag() {
    return sensor_flag;
}

#define TEST_TASK_PRIORITY (tskIDLE_PRIORITY + 1UL)

static struct udp_pcb *udp_server_pcb;

static DataPacket last_packet = { .direction = "", .speed = -1 };

static void udp_server_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    if (!p) {
        printf("Server: Connection closed\n");
        return;
    }

    // Append the received data to buffer
    if (buffer_len + p->len < BUFFER_SIZE) {
        memcpy(buffer + buffer_len, p->payload, p->len);
        buffer_len += p->len;
    } else {
        printf("Warning: Buffer overflow\n");
        pbuf_free(p);
        return;
    }

    // Check if we have a complete DataPacket
    while (buffer_len >= sizeof(DataPacket)) {
        printf("meow\n");
        DataPacket packet;
        memcpy(&packet, buffer, sizeof(DataPacket));
        Direction dir;
        if (strcmp(packet.direction, "Forward") == 0 && get_obstacle_flag() == false && get_sensor_flag() == 0) {
            dir = DIRECTION_FORWARD;
        } else if (strcmp(packet.direction, "Backward") == 0 && get_obstacle_flag() == false && get_sensor_flag() == 0) {
            dir = DIRECTION_BACKWARD;
        } else if (strcmp(packet.direction, "Left") == 0 && get_obstacle_flag() == false && get_sensor_flag() == 0) {
            dir = DIRECTION_LEFT;
        } else if (strcmp(packet.direction, "Right") == 0 && get_obstacle_flag() == false && get_sensor_flag() == 0) {
            dir = DIRECTION_RIGHT;
        }else if (strcmp(packet.direction, "Neutral") == 0 || get_obstacle_flag() == true || get_sensor_flag() == 1) {
            dir = DIRECTION_NEUTRAL;
        }
         else {
            printf("Unknown direction: %s\n", packet.direction);
            dir = DIRECTION_NEUTRAL; // Default direction
        }

        move_robot(dir, (uint8_t)packet.speed);
        // Move any remaining data to the start of the buffer
        buffer_len -= sizeof(DataPacket);
        memmove(buffer, buffer + sizeof(DataPacket), buffer_len);
    }

    pbuf_free(p);
}

void print_ip_address() {
    const struct netif *netif = netif_list;

    if (netif_is_up(netif)) {
        printf("IP Address: %s\n", ip4addr_ntoa(netif_ip4_addr(netif)));
    } else {
        printf("No IP address assigned yet.\n");
    }
}

void wifi_task(__unused void *params) {
    printf("Initializing Wi-Fi...\n");

    // Initialize Wi-Fi after scheduler has started
    if (cyw43_arch_init()) {
        printf("Failed to initialize Wi-Fi\n");
        vTaskDelete(NULL); // Terminate this task
    }
    printf("Wi-Fi initialized\n");

    // Set Wi-Fi credentials
    const char *ssid = "ash (2)";
    const char *password = "stick123";

    // Connect to Wi-Fi
    cyw43_arch_enable_sta_mode();
    printf("Connecting to Wi-Fi...\n");

    if (cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Failed to connect to Wi-Fi\n");
        cyw43_arch_deinit();
        vTaskDelete(NULL); // Terminate this task
    }
    printf("Wi-Fi connected\n");
    
    // Initialize UDP server
    udp_server_pcb = udp_new();
    if (!udp_server_pcb) {
        printf("Failed to create UDP PCB\n");
        // Handle error
    }
    
    if (udp_bind(udp_server_pcb, IP_ADDR_ANY, UDP_PORT) != ERR_OK) {
        printf("Failed to bind UDP server to port %d\n", UDP_PORT);
        // Handle error
    }
    
    udp_recv(udp_server_pcb, udp_server_recv, NULL);

    // Print IP address
    const struct netif *netif = netif_list;
    if (netif_is_up(netif)) {
        printf("IP Address: %s\n", ip4addr_ntoa(netif_ip4_addr(netif)));
    } else {
        printf("No IP address assigned yet.\n");
    }

    // Keep the task alive
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    // Cleanup (if ever reached)
    cyw43_arch_deinit();
    vTaskDelete(NULL);
}