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
#include "utils.h"
#include <string.h>

// ethernet phy initialized flag (used as guard check)
static uint8_t phy_initialized = 0;

static void generate_mac_address(uint8_t *mac_addr) {
  assert(mac_addr != NULL);
  // for (int i = 0; i < 6; i++) {
  //   mac_addr[i] = generate_random() % 0xFF;
  // }
  mac_addr[5] = 0xfa;
  mac_addr[4] = 0x3d;
  mac_addr[3] = 0xe4;
  mac_addr[2] = 0x56;
  mac_addr[1] = 0x67;
  mac_addr[0] = 0x53;
  // set mac address to locally administered (NIC set by MFN). See IEEE 802 MAC
  // address standard.
  mac_addr[0] |= 0x02;
}

uint8_t ethernet_phy_state(void) { return phy_initialized; }

/**
 * @brief Initialize ethernet phy chip.
 *
 */
enet_status_t ethernet_phy_init(void) {
  enet_status_t status = ENET_OK;
  spi_begin(w5100_spi_config);
  if (w5100_verify_hw() != W5100_OK)
    status = ENET_ERR;
  if (w5100_configure() != W5100_OK)
    status = ENET_ERR;
  spi_end();
  // set initialization status
  phy_initialized = status == ENET_OK ? 1 : 0;
  return status;
}

/**
 * @brief Configure device ethernet phy
 *
 * @param config ethernet parameters
 * @return enet_status_t
 */
enet_status_t ethernet_configure(const enet_config_t *config) {
  uint8_t mac_addr[6];
  if (!phy_initialized) {
    return ENET_NOT_INITIALIZED;
  }
  // generate locally administered mac address
  generate_mac_address(mac_addr);
  // set phy properties
  ethernet_set_gateway(config->gateway);
  ethernet_set_subnet_mask(config->subnet_mask);
  ethernet_set_ip_addr(config->ip_addr);
  ethernet_set_mac_addr(mac_addr);
  return ENET_OK;
}

/**
 * @brief Write ethernet gateway to ethernet phy
 *
 * @param gateway gateway raw bytes
 * @return enet_status_t
 */
enet_status_t ethernet_set_gateway(const ipv4_address_t gateway) {
  if (!phy_initialized) {
    return ENET_NOT_INITIALIZED;
  }
  spi_begin(w5100_spi_config);
  w5100_write_gar(gateway.bytes);
  spi_end();
  return ENET_OK;
}

/**
 * @brief Query ethernet gateway from ethernet phy
 *
 * @param gateway ipv4_address_t pointer buffer
 * @param len number of bytes read from transaction
 * @return enet_status_t
 */
enet_status_t ethernet_get_gateway(ipv4_address_t *gateway, uint16_t *len) {
  assert(len != NULL);
  assert(gateway != NULL);
  if (!phy_initialized) {
    return ENET_NOT_INITIALIZED;
  }
  spi_begin(w5100_spi_config);
  *len = w5100_read_gar(gateway->bytes);
  spi_end();
  return ENET_OK;
}

/**
 * @brief Write ethernet subnet mask to ethernet phy
 *
 * @param subnet_mask subnet mask raw bytes
 * @return enet_status_t
 */
enet_status_t ethernet_set_subnet_mask(const ipv4_address_t subnet_mask) {
  if (!phy_initialized) {
    return ENET_NOT_INITIALIZED;
  }
  spi_begin(w5100_spi_config);
  w5100_write_subr(subnet_mask.bytes);
  spi_end();
  return ENET_OK;
}

/**
 * @brief Query ethernet subnet mask from ethernet phy
 *
 * @param subnet_mask ipv4_address_t pointer buffer
 * @param len number of bytes read from transaction
 * @return enet_status_t
 */
enet_status_t ethernet_get_subnet_mask(ipv4_address_t *subnet_mask,
                                       uint16_t *len) {
  assert(len != NULL);
  assert(subnet_mask != NULL);
  if (!phy_initialized) {
    return ENET_NOT_INITIALIZED;
  }
  spi_begin(w5100_spi_config);
  *len = w5100_read_subr(subnet_mask->bytes);
  spi_end();
  return ENET_OK;
}

/**
 * @brief Write ethernet ip address to ethernet phy
 *
 * @param ip_addr ip address raw bytes
 * @return enet_status_t
 */
enet_status_t ethernet_set_ip_addr(const ipv4_address_t ip_addr) {
  if (!phy_initialized) {
    return ENET_NOT_INITIALIZED;
  }
  spi_begin(w5100_spi_config);
  w5100_write_sipr(ip_addr.bytes);
  spi_end();
  return ENET_OK;
}

/**
 * @brief Query ethernet ip address from ethernet phy
 *
 * @param ip_addr ipv4_address_t pointer buffer
 * @param len number of bytes read from transaction
 * @return enet_status_t
 */
enet_status_t ethernet_get_ip_addr(ipv4_address_t *ip_addr, uint16_t *len) {
  assert(len != NULL);
  assert(ip_addr != NULL);
  if (!phy_initialized) {
    return ENET_NOT_INITIALIZED;
  }
  spi_begin(w5100_spi_config);
  *len = w5100_read_sipr(ip_addr->bytes);
  spi_end();
  return ENET_OK;
}

/**
 * @brief Write ethernet mac address to ethernet phy
 *
 * @param mac_addr mac address raw bytes
 * @return enet_status_t
 */
enet_status_t ethernet_set_mac_addr(const uint8_t *mac_addr) {
  assert(mac_addr != NULL);
  if (!phy_initialized) {
    return ENET_NOT_INITIALIZED;
  }
  spi_begin(w5100_spi_config);
  w5100_write_shar(mac_addr);
  spi_end();
  return ENET_OK;
}

/**
 * @brief Query ethernet mac address from ethernet phy
 *
 * @param mac_addr uint8_t pointer buffer
 * @param len number of bytes read from transaction
 * @return enet_status_t
 */
enet_status_t ethernet_get_mac_addr(uint8_t *mac_addr, uint16_t *len) {
  assert(len != NULL);
  assert(mac_addr != NULL);
  if (!phy_initialized) {
    return ENET_NOT_INITIALIZED;
  }
  spi_begin(w5100_spi_config);
  *len = w5100_read_shar(mac_addr);
  spi_end();
  return ENET_OK;
}

/**
 * @brief Set ethernet phy retransmission timeout time (ms)
 *
 * @param ms retransmission timeout (ms)
 * @return enet_status_t
 */
enet_status_t ethernet_set_retransmit_timeout(const uint8_t *ms) {
  assert(ms != NULL);
  if (!phy_initialized) {
    return ENET_NOT_INITIALIZED;
  }
  spi_begin(w5100_spi_config);
  w5100_write_rtr(ms);
  spi_end();
  return ENET_OK;
}

/**
 * @brief Set ethernet phy retransmission counter
 *
 * @param retry retry count
 * @return enet_status_t
 */
enet_status_t ethernet_set_retransmit_count(uint8_t retry) {
  if (!phy_initialized) {
    return ENET_NOT_INITIALIZED;
  }
  spi_begin(w5100_spi_config);
  w5100_write_rcr(retry);
  spi_end();
  return ENET_OK;
}

/**
 * @brief Reset ethernet phy
 *
 */
void ethernet_reset(void) {
  spi_begin(w5100_spi_config);
  // reset phy init flag on successful reset
  if (w5100_reset() == W5100_OK) {
    phy_initialized = 0;
  }
  spi_end();
}