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
#include <string.h>
#include "server.h"

#include "hardware/gpio.h"
#include "hardware/adc.h"

#define UDP_PORT 4242

#ifndef RUN_FREERTOS_ON_CORE
#define RUN_FREERTOS_ON_CORE 0
#endif

#define BUFFER_SIZE 1024  // Adjust size as needed
#define WIFI_SSID "ash (2)"
#define WIFI_PASSWORD "stick123"
static char buffer[BUFFER_SIZE];
static int buffer_len = 0;

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
        DataPacket packet;
        memcpy(&packet, buffer, sizeof(DataPacket));

        // Print or process the received DataPacket only if direction or speed has changed
        if (strcmp(packet.direction, last_packet.direction) != 0 || packet.speed != last_packet.speed) {
            printf("Server: Received DataPacket - Direction: %s, Speed: %d\n",
                   packet.direction, packet.speed);
            last_packet = packet;
        }

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
        print_ip_address();
    }

    udp_server_pcb = udp_new();
    if (!udp_server_pcb) {
        printf("Server: Error creating PCB.\n");
        return;
    }

    ip_addr_t ipaddr;
    IP4_ADDR(&ipaddr, 0, 0, 0, 0);

    err_t err = udp_bind(udp_server_pcb, &ipaddr, UDP_PORT);
    if (err != ERR_OK) {
        printf("Server: Error binding PCB.\n");
        return;
    }

    udp_recv(udp_server_pcb, udp_server_recv, NULL);
    printf("Server: Listening on port %d\n", UDP_PORT);

    while (true) {
        vTaskDelay(100);
    }

    cyw43_arch_deinit();
}
