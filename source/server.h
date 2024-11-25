#ifndef SERVER_H
#define SERVER_H
#include "lwip/ip4_addr.h"
#include "lwip/netif.h"
#include "lwip/udp.h"
#include "FreeRTOS.h"
#include "task.h"
#include "task_handles.h"

extern TaskHandle_t wifiTaskHandle;

// Constants
#define UDP_PORT 4242
#define BUFFER_SIZE 1024
#define TEST_TASK_PRIORITY (tskIDLE_PRIORITY + 1UL)

// DataPacket structure
typedef struct {
    char direction[10];
    int speed;
} DataPacket;

static ip_addr_t client_addr;
static uint16_t client_port;

// Function declarations
static void udp_server_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);
void print_ip_address();
void wifi_task(void *params);
void set_sensor_flag(bool flag);
bool get_sensor_flag();
void set_client_address(const char *ip_str, uint16_t port);
void send_message_to_client(const char *message);

#endif // SERVER_H