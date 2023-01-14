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
 * @brief IPv4 address representation types
 * 
 */
typedef union ipv4_address_t {
  uint8_t bytes[4]; // for byte data transactions
  uint32_t dword; // for comparator ops
} ipv4_address_t;

void ethernet_init(void);
void ethernet_set_gateway(const ipv4_address_t addr);
void ethernet_get_gateway(ipv4_address_t *addr);
void ethernet_set_subnet(const uint8_t *addr);
void ethernet_get_subnet(uint8_t *addr);
void ethernet_set_mac_addr(const uint8_t *addr);
void ethernet_get_mac_addr(uint8_t *addr);
void ethernet_set_ip_addr(const uint8_t *addr);
void ethernet_get_ip_addr(uint8_t *addr);
void ethernet_set_retransmit_time(uint16_t timeout);
void ethernet_set_retransmit_count(uint8_t retry);
void ethernet_reset(void);

#endif // __ETHERNET_H__