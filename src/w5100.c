/**
 * @file w5100.c
 * @brief
 * @version 0.1
 * @date 2023-01
 *
 * @copyright Copyright © 2023 dronectl
 *
 */

#include "w5100.h"
#include "utils.h"

/**
 * @brief Command opcodes for w5100
 * 6.3.2 Commands
 */
#define WRITE_OPCODE 0xF0
#define READ_OPCODE 0x0F

/**
 * @brief Write a byte to W5100 ethernet controller using SPI bus. SPI data
 * bytes need to be written in 32-bit units in big endian format: OP-Code (1
 * byte) -> Address (2 byte) -> Data (1 byte)
 *
 * @param addr W5100 register address
 * @param buffer data byte
 */
void _write_byte(uint16_t addr, const uint8_t buffer) {
  spi_enable_ss();
  spi_transact_byte(WRITE_OPCODE);
  spi_transact_byte((addr >> 8)); // write address MSB first
  spi_transact_byte((addr & 0xFF));
  spi_transact_byte(buffer);
  spi_disable_ss();
}

/**
 * @brief Read a byte from the W5100 ethernet controller using SPI bus. SPI
 * bytes need to be written in 32-bit units in big endian format: OP-Code (1
 * byte) -> Address (2 byte) -> Data (1 byte)
 *
 * @param addr W5100 register address
 * @param buffer byte data buffer
 */
void _read_byte(uint16_t addr, uint8_t *buffer) {
  spi_enable_ss();
  spi_transact_byte(READ_OPCODE);
  spi_transact_byte((addr >> 8)); // write address MSB first
  spi_transact_byte((addr & 0xFF));
  spi_transact(buffer);
  spi_disable_ss();
}

uint16_t _write_bytes(uint16_t addr, const uint8_t *buffer, uint16_t len) {
  for (uint16_t i = 0; i < len; i++) {
    _write_byte(addr + i, buffer[i]);
  }
  return len;
}

uint16_t _read_bytes(uint16_t addr, uint8_t *buffer, uint16_t len) {
  for (uint16_t i = 0; i < len; i++) {
    _read_byte(addr + i, buffer + i);
  }
  return len;
}

/**
 * @brief W5100 hardware verification function.
 *
 * @return w5100_status_t W5100_ERR if device could not be identified, W5100_OK
 * if success.
 */
w5100_status_t w5100_verify_hw(void) {
  if (w5100_reset() != W5100_OK)
    return W5100_ERR;
  w5100_write_mr(MR_PB);
  if (w5100_read_mr() != MR_PB)
    return W5100_ERR;
  return W5100_OK;
}

/**
 * @brief Perform software reset of w5100 ethernet phy. This function blocks
 * until the reset is complete
 *
 * @return w5100_status_t W5100_ERR if device failed to respond after reset
 * W5100_OK if success.
 */
w5100_status_t w5100_reset(void) {
  uint8_t counter = 0;
  // write to mode register reset bit
  w5100_write_mr(MR_RST);
  // wait for soft reset to complete
  do {
    uint8_t mr = w5100_read_mr();
    if (mr == 0x0)
      return W5100_OK;
    delay_cycles(1000);
  } while (++counter < 20);
  return W5100_ERR;
}

void w5100_exec_sock_cmd(const enum W5100SockChannel sock,
                         const enum W5100SockCmds cmd) {
  w5100_write_sn_cr(sock, cmd);
  // after command completion SnCR will autoclear
  while (w5100_read_sn_cr(sock))
    ;
}