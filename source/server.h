#ifndef SERVER_H
#define SERVER_H
#include "lwip/ip4_addr.h"
#include "lwip/netif.h"
#include "lwip/udp.h"

// Constants
#define UDP_PORT 4242
#define BUFFER_SIZE 1024
#define TEST_TASK_PRIORITY (tskIDLE_PRIORITY + 1UL)

// DataPacket structure
typedef struct {
    char direction[10];
    int speed;
} DataPacket;

// Function declarations
static void udp_server_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);
void print_ip_address();
void wifi_task(void *params);

#endif // SERVER_H