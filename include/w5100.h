/**
 * @file w5100.h
 * @brief Functions declarations and definitions for interfacing with the w5100
 * ethernet phy controller.
 * @version 0.1
 * @date 2023-01
 *
 * @copyright Copyright © 2023 dronectl
 *
 */

#ifndef __W5100_H__
#define __W5100_H__

#include "spi.h"
#include <avr/io.h>
#include <stdint.h>

#if defined(RAMEND) && defined(RAMSTART) && ((RAMEND - RAMSTART) <= 1024)
#define W5100_MAX_SOCK_NUM 2
#else
#define W5100_MAX_SOCK_NUM 4
#endif

/**
 * @brief W5100 API Status Codes
 *
 */
typedef uint8_t w5100_status_t;
#define W5100_OK (w5100_status_t)0  // success code
#define W5100_ERR (w5100_status_t)1 // generic failure

// static for multiple translation units
static const spi_config_t w5100_spi_config = {
    .clock = 14000000, .endian = BIG_ENDIAN, .mode = SPI_MODE0};

/**
 * @brief Mode Register Bits
 * 4.1 Common Registers
 */
#define MR_RST (uint8_t)(1 << 7)   // S/W Reset
#define MR_PB (uint8_t)(1 << 4)    // Ping Block Mode
#define MR_PPPOE (uint8_t)(1 << 3) // PPPoE Mode
#define MR_AI (uint8_t)(1 << 1)    // Address Auto-Increment in Indirect Bus I/F
#define MR_IND (uint8_t)(1 << 0)   // Indirect Bus I/F mode

void _read_byte(uint16_t addr, uint8_t *buffer);
void _write_byte(uint16_t addr, const uint8_t buffer);
uint16_t _read_bytes(uint16_t addr, uint8_t *buffer, uint16_t len);
uint16_t _write_bytes(uint16_t addr, const uint8_t *buffer, uint16_t len);

/**
 * @brief Common 8 bit register read and write functions
 *
 */
#define __CREGISTER8(name, address)                                            \
  static void w5100_write_##name(uint8_t _data) {                              \
    _write_byte(address, _data);                                               \
  }                                                                            \
  static uint8_t w5100_read_##name(void) {                                     \
    uint8_t data;                                                              \
    _read_byte(address, &data);                                                \
    return data;                                                               \
  }

/**
 * @brief Common 16 bit register read and write functions
 *
 */
#define __CREGISTER16(name, address)                                           \
  static uint16_t w5100_write_##name(const uint16_t _data) {                   \
    uint8_t buf[2];                                                            \
    buf[0] = _data >> 8;                                                       \
    buf[1] = _data & 0xFF;                                                     \
    return _write_bytes(address, buf, 2);                                      \
  }                                                                            \
  static uint16_t w5100_read_##name(void) {                                    \
    uint8_t buf[2];                                                            \
    read_bytes(address, buf, 2);                                               \
    return (buf[0] << 8) | buf[1];                                             \
  }

/**
 * @brief Common N-bit register read and write functions
 *
 */
#define __CREGISTER_N(name, address, size)                                     \
  static uint16_t w5100_write_##name(const uint8_t *_buffer) {                 \
    return _write_bytes(address, _buffer, size);                               \
  }                                                                            \
  static uint16_t w5100_read_##name(uint8_t *_buffer) {                        \
    return _read_bytes(address, _buffer, size);                                \
  }

w5100_status_t w5100_verify_hw(void);
w5100_status_t w5100_reset(void);

/**
 * @brief Common register definitions
 *
 * 4.1 Common Registers
 */
__CREGISTER8(mr, 0x0000)      // mode register
__CREGISTER_N(gar, 0x0001, 4) // gateway address register
// __CREGISTER_N(subr, 0x0005, 4) // subnet mask address register
// __CREGISTER_N(shar, 0x0009, 6) // source hardware address register
// __CREGISTER_N(sipr, 0x000F, 4) // source ip address register
// __CREGISTER8(ir, 0x0015)       // interrupt register
// __CREGISTER8(imr, 0x0016)      // interrupt mask register
// __CREGISTER_N(rtr, 0x0017, 2)  // retry time-value register
// __CREGISTER8(rcr, 0x0019)      // retry count register
// __CREGISTER8(rmsr, 0x001A)     // rx memory size register
// __CREGISTER8(tmsr, 0x001B)     // tx memory size register
// __CREGISTER8(patr, 0x001C)     // authentication type in PPPoE mode
// __CREGISTER8(ptimer,
//              0x0028) // PPP link control protocol request timer register
// __CREGISTER8(pmagic,
//              0x0029) // PPP link control protocol magic number register
// __CREGISTER_N(uipr, 0x002A, 4)  // unreachable ip address register (UDP Mode)
// __CREGISTER_N(uport, 0x002E, 2) // unreachable port register (UDP Mode)

#endif // __W5100_H__