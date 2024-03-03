#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__

#include "cmsis_os2.h"
#include "lwip/err.h"
#include "lwip/netif.h"

err_t ethernetif_init(struct netif *netif);
void ethernetif_input(void *argument);
void ethernet_link_thread(void *arguments);
#endif