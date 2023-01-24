/**
 * @file ethernet.h
 * @brief
 * @version 0.1
 * @date 2023-01
 *
 * @copyright Copyright © 2023 dronectl
 *
 */
#ifndef __ETHERNET_H__
#define __ETHERNET_H__

#include "w5100.h"
#include <stdint.h>

/**
 * @brief Ethernet Proxy API Status Codes
 *
 */
typedef uint8_t enet_status_t;
#define ENET_OK (enet_status_t)0              // success code
#define ENET_ERR (enet_status_t)1             // generic failure
#define ENET_NOT_INITIALIZED (enet_status_t)2 // not initialized failure

/**
 * @brief IPv4 address representation types
 *
 */
typedef union ipv4_address_t {
  uint8_t bytes[4]; // for byte data transactions
  uint32_t dword;   // for comparator ops
} ipv4_address_t;

typedef struct enet_config_t {
  ipv4_address_t ip_addr;
  ipv4_address_t subnet_mask;
  ipv4_address_t gateway;
} enet_config_t;

enet_status_t ethernet_phy_init(void);
uint8_t ethernet_phy_state(void);
enet_status_t ethernet_configure(const enet_config_t *config);
enet_status_t ethernet_set_gateway(const ipv4_address_t gateway);
enet_status_t ethernet_get_gateway(ipv4_address_t *gateway, uint16_t *len);
enet_status_t ethernet_set_subnet_mask(const ipv4_address_t subnet_mask);
enet_status_t ethernet_get_subnet_mask(ipv4_address_t *subnet_mask,
                                       uint16_t *len);
enet_status_t ethernet_set_mac_addr(const uint8_t *addr);
enet_status_t ethernet_get_mac_addr(uint8_t *addr, uint16_t *len);
enet_status_t ethernet_set_ip_addr(const ipv4_address_t ip_addr);
enet_status_t ethernet_get_ip_addr(ipv4_address_t *ip_addr, uint16_t *len);
enet_status_t ethernet_set_retransmit_timeout(const uint8_t *ms);
enet_status_t ethernet_set_retransmit_count(uint8_t retry);
void ethernet_reset(void);

#endif // __ETHERNET_H__