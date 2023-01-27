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
  // initialize phy
  ethernet_phy_init();
  ipv4_address_t gw;
  gw.bytes[0] = 192;
  gw.bytes[1] = 168;
  gw.bytes[2] = 2;
  gw.bytes[3] = 1;
  ipv4_address_t mask;
  mask.bytes[0] = 255;
  mask.bytes[1] = 255;
  mask.bytes[2] = 0;
  mask.bytes[3] = 0;
  ipv4_address_t ip_addr;
  ip_addr.bytes[0] = 0;
  ip_addr.bytes[1] = 0;
  ip_addr.bytes[2] = 0;
  ip_addr.bytes[3] = 0;
  enet_config_t configuration = {
      .gateway = gw, .subnet_mask = mask, .ip_addr = ip_addr};
  ethernet_configure(&configuration);
  spinlock();
  return 0;
}