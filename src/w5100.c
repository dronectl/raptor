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
#include "spi.h"
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
void w5100_write_byte(uint16_t addr, const uint8_t buffer) {
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
void w5100_read_byte(uint16_t addr, uint8_t *buffer) {
  spi_enable_ss();
  spi_transact_byte(READ_OPCODE);
  spi_transact_byte((addr >> 8)); // write address MSB first
  spi_transact_byte((addr & 0xFF));
  spi_transact(buffer);
  spi_disable_ss();
}

uint16_t w5100_write_bytes(uint16_t addr, const uint8_t *buffer, uint16_t len) {
  for (uint16_t i = 0; i < len; i++) {
    w5100_write_byte(addr + i, buffer[i]);
  }
  return len;
}

uint16_t w5100_read_bytes(uint16_t addr, uint8_t *buffer, uint16_t len) {
  for (uint16_t i = 0; i < len; i++) {
    w5100_read_byte(addr + i, buffer + i);
  }
  return len;
}

static uint8_t verify_w5100(void) {
  write_mr(0x00);
  return 0;
}

uint8_t w5100_soft_reset(void) {
  uint8_t counter = 0;
  // write to mode register reset bit
  write_mr(MR_RST);
  // wait for soft reset to complete
  do {
    uint8_t mr = read_mr();
    if (mr == 0x0)
      return 1;
    delay_cycles(1000);
  } while (++counter < 20);
  return 0;
}

void w5100_init(void) {
  spi_config_t config = {
      .clock = 14000000, .endian = BIG_ENDIAN, .mode = SPI_MODE0};
  spi_begin(config);
  spi_disable_ss();
  verify_w5100();
}
