/**
 * @file app_ethernet.c
 * @brief TCPIP stack powered by LWIP
 * @version 0.1
 * @date 2024-04
 *
 * @copyright Copyright © 2024 dronectl
 *
 */

#include "app_ethernet.h"
#include "ethernetif.h"
#include "netif/ethernet.h"
#include "stm32h7xx_nucleo.h"
#if LWIP_DHCP
#include <lwip/dhcp.h>
#endif
#include <lwip/tcpip.h>
#include <lwip/netif.h>
#include <cmsis_os2.h>

/*Static IP ADDRESS: IP_ADDR0.IP_ADDR1.IP_ADDR2.IP_ADDR3 */
#define IP_ADDR0 ((uint8_t)192U)
#define IP_ADDR1 ((uint8_t)168U)
#define IP_ADDR2 ((uint8_t)0U)
#define IP_ADDR3 ((uint8_t)10U)

/*NETMASK*/
#define NETMASK_ADDR0 ((uint8_t)255U)
#define NETMASK_ADDR1 ((uint8_t)255U)
#define NETMASK_ADDR2 ((uint8_t)255U)
#define NETMASK_ADDR3 ((uint8_t)0U)

/*Gateway Address*/
#define GW_ADDR0 ((uint8_t)192U)
#define GW_ADDR1 ((uint8_t)168U)
#define GW_ADDR2 ((uint8_t)0U)
#define GW_ADDR3 ((uint8_t)1U)

// global gnetif struct
static struct netif gnetif;

const osThreadAttr_t link_attr = {
    .name = "link_task",
    .priority = osPriorityNormal,
};
const osThreadAttr_t dhcp_attr = {
    .name = "dhcp_task",
    .priority = osPriorityNormal,
};

static osThreadId_t link_handle;
static osThreadId_t dhcp_handle;

#if LWIP_DHCP
#define MAX_DHCP_TRIES 4
uint32_t DHCPfineTimer = 0;

/* DHCP process states */
#define DHCP_OFF (uint8_t)0
#define DHCP_START (uint8_t)1
#define DHCP_WAIT_ADDRESS (uint8_t)2
#define DHCP_ADDRESS_ASSIGNED (uint8_t)3
#define DHCP_TIMEOUT (uint8_t)4
#define DHCP_LINK_DOWN (uint8_t)5

uint8_t DHCP_state = DHCP_OFF;

/**
 * @brief  DHCP Process
 * @param  argument: network interface
 * @retval None
 */
static void dhcp_task(void *argument) {
  struct netif *netif = (struct netif *)argument;
  ip_addr_t ipaddr;
  ip_addr_t netmask;
  ip_addr_t gw;
  struct dhcp *dhcp;

  for (;;) {
    switch (DHCP_state) {
      case DHCP_START: {
        ip_addr_set_zero_ip4(&netif->ip_addr);
        ip_addr_set_zero_ip4(&netif->netmask);
        ip_addr_set_zero_ip4(&netif->gw);
        DHCP_state = DHCP_WAIT_ADDRESS;

        BSP_LED_Off(LED2);
        BSP_LED_Off(LED3);

        dhcp_start(netif);
      } break;
      case DHCP_WAIT_ADDRESS: {
        if (dhcp_supplied_address(netif)) {
          DHCP_state = DHCP_ADDRESS_ASSIGNED;

          BSP_LED_On(LED2);
          BSP_LED_Off(LED3);
        } else {
          dhcp = (struct dhcp *)netif_get_client_data(
              netif, LWIP_NETIF_CLIENT_DATA_INDEX_DHCP);

          /* DHCP timeout */
          if (dhcp->tries > MAX_DHCP_TRIES) {
            DHCP_state = DHCP_TIMEOUT;

            /* Static address used */
            IP_ADDR4(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
            IP_ADDR4(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2,
                     NETMASK_ADDR3);
            IP_ADDR4(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
            netif_set_addr(netif, ip_2_ip4(&ipaddr), ip_2_ip4(&netmask),
                           ip_2_ip4(&gw));

            BSP_LED_On(LED2);
            BSP_LED_Off(LED3);
          }
        }
      } break;
      case DHCP_LINK_DOWN: {

        DHCP_state = DHCP_OFF;

        BSP_LED_Off(LED2);
        BSP_LED_On(LED3);
      } break;
      default:
        break;
    }

    /* wait 500 ms */
    osDelay(500);
  }
}
#endif /* LWIP_DHCP */

/**
 * @brief  Notify the User about the network interface config status
 * @param  netif: the network interface
 * @retval None
 */
static void ethernet_link_status_updated(struct netif *netif) {
  if (netif_is_link_up(netif)) {
#if LWIP_DHCP
    /* Update DHCP state machine */
    DHCP_state = DHCP_START;
#else
    BSP_LED_On(LED2);
    BSP_LED_Off(LED3);
#endif /* LWIP_DHCP */
  } else {
#if LWIP_DHCP
    /* Update DHCP state machine */
    DHCP_state = DHCP_LINK_DOWN;
#else
    BSP_LED_Off(LED2);
    BSP_LED_On(LED3);
#endif /* LWIP_DHCP */
  }
}

/**
 * @brief  Setup the network interface
 * @param  None
 * @retval None
 */
static void netconfig_init(void __attribute__((unused)) * args) {
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
  IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
  IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
#endif
  /* add the network interface */
  netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);
  /*  Registers the default network interface */
  netif_set_default(&gnetif);
  ethernet_link_status_updated(&gnetif);
#if LWIP_NETIF_LINK_CALLBACK
  netif_set_link_callback(&gnetif, ethernet_link_status_updated);
  link_handle = osThreadNew(ethernet_link_thread, &gnetif, &link_attr);
  if (link_handle == NULL) {
  }
#endif
#if LWIP_DHCP
  dhcp_handle = osThreadNew(dhcp_task, &gnetif, &dhcp_attr);
  if (dhcp_handle == NULL) {
  }
#endif
}

system_status_t app_ethernet_init(void) {
  // invoke netconfig_init once tcpip init is complete
  tcpip_init(netconfig_init, NULL);
  return SYSTEM_OK;
}
