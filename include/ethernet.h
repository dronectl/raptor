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

#include <stdint.h>

/**
 * @brief Ethernet Proxy API Status Codes
 *
 */
typedef uint8_t enet_status_t;
#define ENET_OK (enet_status_t)0  // success code
#define ENET_ERR (enet_status_t)1 // generic failure

/**
 * @brief IPv4 address representation types
 *
 */
typedef union ipv4_address_t {
  uint8_t bytes[4]; // for byte data transactions
  uint32_t dword;   // for comparator ops
} ipv4_address_t;

enet_status_t ethernet_phy_init(void);
void ethernet_set_gateway(const ipv4_address_t gateway);
void ethernet_get_gateway(ipv4_address_t *gateway, uint16_t *len);
void ethernet_set_subnet_mask(const ipv4_address_t subnet_mask);
void ethernet_get_subnet_mask(ipv4_address_t *subnet_mask, uint16_t *len);
void ethernet_set_mac_addr(const uint8_t *addr);
void ethernet_get_mac_addr(uint8_t *addr, uint16_t *len);
void ethernet_set_ip_addr(const ipv4_address_t ip_addr);
void ethernet_get_ip_addr(ipv4_address_t *ip_addr, uint16_t *len);
void ethernet_set_retransmit_time(uint16_t timeout);
void ethernet_set_retransmit_count(uint8_t retry);
void ethernet_reset(void);

#endif // __ETHERNET_H__