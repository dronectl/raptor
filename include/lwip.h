#ifndef __LWIP_H__
#define __LWIP_H__

/* Includes ------------------------------------------------------------------*/
#include "lwip/netif.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* DHCP process states */
#define DHCP_OFF (uint8_t)0
#define DHCP_START (uint8_t)1
#define DHCP_WAIT_ADDRESS (uint8_t)2
#define DHCP_ADDRESS_ASSIGNED (uint8_t)3
#define DHCP_TIMEOUT (uint8_t)4
#define DHCP_LINK_DOWN (uint8_t)5

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void ethernet_link_status_updated(struct netif *netif);
#if LWIP_DHCP
void dhcp_task(void *pv_params);
#endif
#endif /* __LWIP_H__ */