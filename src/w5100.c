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
#include "assert.h"
#include "utils.h"

/**
 * @brief Command opcodes for w5100
 * 6.3.2 Commands
 */
#define WRITE_OPCODE 0xF0
#define READ_OPCODE 0x0F

#define MEMSIZE_CONF_8192 (uint8_t)0x3
#define MEMSIZE_CONF_4096 (uint8_t)0x2
#define MEMSIZE_CONF_2048 (uint8_t)0x1
#define MEMSIZE_CONF_1024 (uint8_t)0x0

// REVIEW: Hardcoded rx and tx buffer allocation sizes per socket. idx=sock num
static const uint16_t rx_alloc_sizes[4] = {0x800, 0x400, 0x1000, 0x400};
static const uint16_t tx_alloc_sizes[4] = {0x800, 0x1000, 0x400, 0x400};

// socket memory map for tx and rx buffers
static w5100_mem_t smem_map[4] = {
    {.rx_mem = {.mask = 0x0, .offset = 0x0, .size = 0x0},
     .tx_mem = {.mask = 0x0, .offset = 0x0, .size = 0x0}},
    {.rx_mem = {.mask = 0x0, .offset = 0x0, .size = 0x0},
     .tx_mem = {.mask = 0x0, .offset = 0x0, .size = 0x0}},
    {.rx_mem = {.mask = 0x0, .offset = 0x0, .size = 0x0},
     .tx_mem = {.mask = 0x0, .offset = 0x0, .size = 0x0}},
    {.rx_mem = {.mask = 0x0, .offset = 0x0, .size = 0x0},
     .tx_mem = {.mask = 0x0, .offset = 0x0, .size = 0x0}}

};

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

/**
 * @brief Configure socket RX and TX buffer sizes and save to local struct array
 *
 * SOCK |  SO     S1      S2     S3
 * -----------------------------------
 * RX   | 2048   1024    4096   1024
 * TX   | 2048   4096    1024   1024
 * DESC | C&C    HSU     DSU    AUX
 *
 * @return w5100_status_t
 */
w5100_status_t w5100_configure(void) {
  // set RMSR
  spi_begin(w5100_spi_config);
  // config bit ordering [ 7 6 ] [ 5 4 ] [ 3]
  uint8_t size_config = (6 << MEMSIZE_CONF_1024) | (4 << MEMSIZE_CONF_4096) |
                        (2 << MEMSIZE_CONF_1024) | (MEMSIZE_CONF_2048);
  w5100_write_rmsr(size_config);
  // set TMSR
  size_config = (6 << MEMSIZE_CONF_1024) | (4 << MEMSIZE_CONF_1024) |
                (2 << MEMSIZE_CONF_4096) | (MEMSIZE_CONF_2048);
  w5100_write_tmsr(size_config);
  spi_end();
  // set socket tx and rx memory mask and base
  for (int i = 0; i <= 3; i++) {
    if (i == 0) {
      smem_map[i].rx_mem.offset = W5100_RX_BUFFER_BASE;
      smem_map[i].tx_mem.offset = W5100_TX_BUFFER_BASE;
    } else {
      smem_map[i].rx_mem.size = rx_alloc_sizes[i];
      smem_map[i].tx_mem.size = tx_alloc_sizes[i];
      smem_map[i].rx_mem.offset =
          smem_map[i - 1].rx_mem.offset + smem_map[i - 1].rx_mem.mask + 0x1;
      smem_map[i].tx_mem.offset =
          smem_map[i - 1].tx_mem.offset + smem_map[i - 1].tx_mem.mask + 0x1;
    }
    smem_map[i].rx_mem.mask = rx_alloc_sizes[i] - 0x1;
    smem_map[i].tx_mem.mask = tx_alloc_sizes[i] - 0x1;
  }
  return W5100_OK;
}

uint16_t w5100_get_tx_offset(enum W5100SCH _s) {
  return smem_map[_s].tx_mem.offset;
}

uint16_t w5100_get_rx_offset(enum W5100SCH _s) {
  return smem_map[_s].rx_mem.offset;
}

uint16_t w5100_get_tx_mask(enum W5100SCH _s) {
  return smem_map[_s].tx_mem.mask;
}

uint16_t w5100_get_rx_mask(enum W5100SCH _s) {
  return smem_map[_s].rx_mem.mask;
}

uint16_t w5100_get_tx_size(enum W5100SCH _s) {
  return smem_map[_s].rx_mem.size;
}

uint16_t w5100_get_rx_size(enum W5100SCH _s) {
  return smem_map[_s].tx_mem.size;
}

/**
 * @brief Custom read function for 16 bit SN_RX_RSR (RX Received Size register).
 * The datasheet indicates the register must be read starting with the MSB to
 * the LSB. See section 4.2 RX_RSR.
 *
 * @param _s target socket
 * @param _buff read buffer
 * @return uint16_t number of bytes read (for consistency with other expanded
 * functions)
 */
uint16_t w5100_read_rx_rsr(enum W5100SCH _s, uint16_t *_buff) {
  assert(_buff != NULL);
  // read from 16 bit register MSB first.
  for (int i = 1; i >= 0; i--) {
    w5100_read_byte(W5100_MEMAP_SREG_BASE(_s) + 0x0026 + i,
                    (uint8_t *)_buff + i);
  }
  return 2;
}

/**
 * @brief Custom read function for 16 bit SN_TX_FSR (socket tx free size
 * register). The datasheet indicates the register must be read starting with
 * the MSB to the LSB. See section 4.2 SN_TX_FSR.
 *
 * @param _s target socket
 * @param _buff read buffer
 * @return uint16_t number of bytes read (for consistency with other expanded
 * functions)
 */
uint16_t w5100_read_tx_fsr(enum W5100SCH _s, uint16_t *_buff) {
  assert(_buff != NULL);
  // read from 16 bit register MSB first.
  for (int i = 1; i >= 0; i--) {
    w5100_read_byte(W5100_MEMAP_SREG_BASE(_s) + 0x0020 + i,
                    (uint8_t *)_buff + i);
  }
  return 2;
}

uint16_t w5100_write_bytes(uint16_t addr, const uint8_t *buffer, uint16_t len) {
  assert(buffer != NULL);
  for (uint16_t i = 0; i < len; i++) {
    w5100_write_byte(addr + i, buffer[i]);
  }
  return len;
}

uint16_t w5100_read_bytes(uint16_t addr, uint8_t *buffer, uint16_t len) {
  assert(buffer != NULL);
  for (uint16_t i = 0; i < len; i++) {
    w5100_read_byte(addr + i, buffer + i);
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
  // just write initial MR state
  w5100_write_mr(0x0);
  if (w5100_read_mr() != 0x0)
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

/**
 * @brief Execute socket command on device and block until completion.
 *
 * @param sock socket
 * @param cmd socket command
 */
void w5100_exec_sock_cmd(const enum W5100SCH sock,
                         const enum W5100SockCmds cmd) {
  w5100_write_sn_cr(sock, cmd);
  // after command completion SnCR will autoclear
  while (w5100_read_sn_cr(sock))
    ;
}
