/**
 * @file main.c
 * @author Christian Sargusingh (christian911@sympatico.ca)
 * @brief
 * @version 0.1
 * @date 2023-01
 *
 * @copyright Copyright © 2023 dronectl
 *
 */

#include "ethernet.h"
#include "spi.h"
#include "utils.h"
#include <avr/cpufunc.h>
#include <avr/io.h>

static void spinlock(void) {
  // status LED tied to SCK requires release of spi subsystem before use
  spi_end();
  // Setup PB5 as Output high (source)
  PORTB = (1 << PB5);
  DDRB = (1 << DDB5);
  // sync nop
  _NOP();
  while (1) {
    // blink SCK
    PORTB ^= (1 << PORTB5);
    delay_cycles(100000);
  }
}

int main(void) {
  uint16_t len;
  // initialize phy
  ethernet_phy_init();
  ipv4_address_t gw;
  gw.bytes[0] = 192;
  gw.bytes[1] = 168;
  gw.bytes[2] = 2;
  gw.bytes[3] = 1;
  ethernet_set_gateway(gw);
  ethernet_get_gateway(&gw, &len);
  ipv4_address_t mask;
  mask.bytes[0] = 255;
  mask.bytes[1] = 255;
  mask.bytes[2] = 0;
  mask.bytes[3] = 0;
  ethernet_set_subnet_mask(mask);
  ethernet_get_subnet_mask(&mask, &len);
  ipv4_address_t ip_addr;
  ip_addr.bytes[0] = 0;
  ip_addr.bytes[1] = 0;
  ip_addr.bytes[2] = 0;
  ip_addr.bytes[3] = 0;
  ethernet_set_ip_addr(ip_addr);
  ethernet_get_gateway(&ip_addr, &len);
  uint8_t mac_addr[6];
  mac_addr[0] = 0xFE;
  mac_addr[1] = 0xDC;
  mac_addr[2] = 0xBA;
  mac_addr[3] = 0x98;
  mac_addr[4] = 0x76;
  mac_addr[5] = 0x54;
  ethernet_set_mac_addr((const uint8_t *)mac_addr);
  ethernet_get_mac_addr(mac_addr, &len);
  spinlock();
  return 0;
}