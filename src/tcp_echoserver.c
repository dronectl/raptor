#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/opt.h"
#include "netif/etharp.h"
#if LWIP_DHCP
#include "lwip/dhcp.h"
#endif
#include "FreeRTOS.h"
#include "app_ethernet.h"
#include "ethernetif.h"
#include "lwip/api.h"
#include "lwip/debug.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "lwip/stats.h"
#include "lwip/tcpip.h"
#include "task.h"
#include "tcp_echoserver.h"

#if LWIP_TCP
struct netif gnetif;

/* structure for maintaining connection infos to be passed as argument
   to LwIP callbacks*/
struct tcp_echoserver_struct {
  u8_t state; /* current connection state */
  u8_t retries;
  struct tcp_pcb *pcb; /* pointer on the current tcp_pcb */
  struct pbuf *p;      /* pointer on the received/to be transmitted pbuf */
};

/**
 * @brief  Setup the network interface
 * @param  None
 * @retval None
 */
static void netconfig_init(void) {
  ip_addr_t ipaddr;
  ip_addr_t netmask;
  ip_addr_t gw;

#if LWIP_DHCP
  ip_addr_set_zero_ip4(&ipaddr);
  ip_addr_set_zero_ip4(&netmask);
  ip_addr_set_zero_ip4(&gw);
#else

  /* IP address default setting */
  IP4_ADDR(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
  IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2,
           NETMASK_ADDR3);
  IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);

#endif

  /* add the network interface */
  netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init,
            &ethernet_input);

  /*  Registers the default network interface */
  netif_set_default(&gnetif);

  ethernet_link_status_updated(&gnetif);
  TaskHandle_t xHandle = NULL;
  BaseType_t x_returned;
#if LWIP_NETIF_LINK_CALLBACK
  netif_set_link_callback(&gnetif, ethernet_link_status_updated);

  x_returned = xTaskCreate(ethernet_link_task, "ethernet_link_task",
                           configMINIMAL_STACK_SIZE * 4, &gnetif,
                           tskIDLE_PRIORITY + 3, &xHandle);
  configASSERT(xHandle);
  if (x_returned != pdPASS) {
    vTaskDelete(xHandle);
  }
#endif

#if LWIP_DHCP
  x_returned = xTaskCreate(dhcp_task, "dhcp_task", configMINIMAL_STACK_SIZE * 4,
                           &gnetif, tskIDLE_PRIORITY + 2, &xHandle);
  configASSERT(xHandle);
  if (x_returned != pdPASS) {
    vTaskDelete(xHandle);
  }
#endif
}

void tcp_server_task(void *pv_params) {
  /* Create tcp_ip stack thread */
  tcpip_init(NULL, NULL);
  // create DHCP and link tasks
  netconfig_init();

  int sock, newconn, size;
  struct sockaddr_in address, remotehost;

  /* create a TCP socket */
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    return;
  }

  /* bind to port 80 at any interface */
  address.sin_family = AF_INET;
  address.sin_port = htons(7);
  address.sin_addr.s_addr = INADDR_ANY;

  if (bind(sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
    return;
  }

  /* listen for incoming connections (TCP listen backlog = 5) */
  listen(sock, 5);

  size = sizeof(remotehost);

  while (1) {
    newconn = accept(sock, (struct sockaddr *)&remotehost, (socklen_t *)&size);
    if (newconn < 0) {
      // Error in accepting connection
      continue;
    }
    char buffer[1024];
    ssize_t bytes_received = recv(newconn, buffer, sizeof(buffer), 0);
    if (bytes_received < 0) {
      // Error in receiving data
      close(newconn);
      continue;
    } else if (bytes_received == 0) {
      // Client disconnected
      close(newconn);
      continue;
    }
    ssize_t bytes_sent = send(newconn, buffer, bytes_received, 0);
    if (bytes_sent < 0) {
      // Error in sending data
      close(newconn);
      continue;
    }
    // Close the connection
    close(newconn);
  }
}

#endif /* LWIP_TCP */
