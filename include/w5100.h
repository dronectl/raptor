/**
 * @file w5100.h
 * @brief Functions declarations and definitions for interfacing with the w5100
 * ethernet controller.
 * @version 0.1
 * @date 2023-01
 *
 * @copyright Copyright © 2023 dronectl
 *
 */

#ifndef __W5100_H__
#define __W5100_H__

#include <avr/io.h>
#include <stdint.h>

#if defined(RAMEND) && defined(RAMSTART) && ((RAMEND - RAMSTART) <= 1024)
#define W5100_MAX_SOCK_NUM 2
#else
#define W5100_MAX_SOCK_NUM 4
#endif

#define MR_RST (uint8_t)(1 << 8)

#define __GP_REGISTER8(name, address)                                          \
  static inline void write##name(uint8_t _data) {                              \
    w5100_write_byte(address, _data);                                          \
  }                                                                            \
  static inline uint8_t read##name(void) {                                     \
    uint8_t data;                                                              \
    w5100_read_byte(address, &data);                                           \
    return data;                                                               \
  }

#define __GP_REGISTER16(name, address)                                         \
  static uint16_t write##name(uint16_t _data) {                                \
    uint8_t buf[2];                                                            \
    buf[0] = _data >> 8;                                                       \
    buf[1] = _data & 0xFF;                                                     \
    return w5100_write_bytes(address, buf, 2);                                 \
  }                                                                            \
  static uint16_t read##name(void) {                                           \
    uint8_t buf[2];                                                            \
    w5100_read_bytes(address, buf, 2);                                         \
    return (buf[0] << 8) | buf[1];                                             \
  }

#define __GP_REGISTER_N(name, address, size)                                   \
  static uint16_t write##name(const uint8_t *_buffer) {                        \
    return w5100_write_bytes(address, _buffer, size);                          \
  }                                                                            \
  static uint16_t read##name(uint8_t *_buffer) {                               \
    return w5100_read_bytes(address, _buffer, size);                           \
  }

void w5100_read_byte(uint16_t addr, uint8_t *buffer);
void w5100_write_byte(uint16_t addr, const uint8_t buffer);
uint16_t w5100_read_bytes(uint16_t addr, uint8_t *buffer, uint16_t len);
uint16_t w5100_write_bytes(uint16_t addr, const uint8_t *buffer, uint16_t len);
void w5100_init(void);
uint8_t w5100_soft_reset(void);

/**
 * @brief Common register definitions
 *
 * 4.1 Common Registers
 */
__GP_REGISTER8(_mr, 0x0000)       // mode register
__GP_REGISTER_N(_gar, 0x0001, 4)  // gateway address register
__GP_REGISTER_N(_subr, 0x0005, 4) // subnet mask address register
__GP_REGISTER_N(_shar, 0x0009, 6) // source hardware address register
__GP_REGISTER_N(_sipr, 0x000F, 4) // source ip address register
__GP_REGISTER8(_ir, 0x0015)       // interrupt register
__GP_REGISTER8(_imr, 0x0016)      // interrupt mask register
__GP_REGISTER16(_rtr, 0x0017)     // retry time-value register
__GP_REGISTER8(_rcr, 0x0019)      // retry count register
__GP_REGISTER8(_rmsr, 0x001A)     // rx memory size register
__GP_REGISTER8(_tmsr, 0x001B)     // tx memory size register
__GP_REGISTER8(_patr, 0x001C)     // authentication type in PPPoE mode
__GP_REGISTER8(_ptimer,
               0x0028) // PPP link control protocol request timer register
__GP_REGISTER8(_pmagic,
               0x0029) // PPP link control protocol magic number register
__GP_REGISTER_N(_uipr, 0x002A, 4) // unreachable ip address register (UDP Mode)
__GP_REGISTER16(_uport, 0x002E)   // unreachable port register (UDP Mode)

#endif // __W5100_H__