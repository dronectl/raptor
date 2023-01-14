/**
 * @file ethernet.c
 * @brief
 * @version 0.1
 * @date 2023-01
 *
 * @copyright Copyright © 2023 dronectl
 *
 */

#include "ethernet.h"
#include "assert.h"
#include "spi.h"
#include "w5100.h"

void ethernet_init(void) {
  spi_begin(w5100_spi_config);
  w5100_verify_hw();
  spi_end();
}

void ethernet_set_gateway(const ipv4_address_t gateway) {
  spi_begin(w5100_spi_config);
  w5100_write_gar(gateway.bytes);
  spi_end();
}

void ethernet_get_gateway(ipv4_address_t *gateway) {
  spi_begin(w5100_spi_config);
  w5100_read_gar(gateway->bytes);
  spi_end();
}