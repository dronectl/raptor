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

// W5100 socket register width in memory
#define W5100_SOCK_REG_MEM_SIZE 0x100
// W5100 base socket register address (Socket 0)
#define W5100_SOCK_BASE 0x0400
#define W5100_MEMAP_SREG_BASE(n)                                               \
  (W5100_SOCK_BASE + (W5100_SOCK_REG_MEM_SIZE * n))

#define W5100_RX_BUFFER_BASE (uint16_t)0x6000
#define W5100_TX_BUFFER_BASE (uint16_t)0x4000
// TX and RX buffer sizes are the same (bytes)
#define W5100_SOCKET_BUFFER_SIZE (W5100_RX_MEM_BASE - W5100_TX_MEM_BASE)

typedef struct socket_mem_t {
  uint16_t mask;
  uint16_t offset; // offset calculated from buffer base (bytes)
  uint16_t size;   // socket memory size (bytes)
} socket_mem_t;

typedef struct w5100_mem_t {
  socket_mem_t tx_mem; // transmission memory access fields
  socket_mem_t rx_mem; // receive memory access fields
} w5100_mem_t;

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

/**
 * @brief Socket N Interrupt Register
 * 4.2 Socket Registers
 */
#define SNIR_SENDOK (uint8_t)(1 << 4)  // Set if send op completed
#define SNIR_TIMEOUT (uint8_t)(1 << 3) // Set if timeout occurs
#define SNIR_RECV (uint8_t)(1 << 2)    // Set when phy receives data
// set when connection termination is requested or finished
#define SNIR_DISCON (uint8_t)(1 << 1)
#define SNIR_CON (uint8_t)(1 << 0) // set if connection established

/**
 * @brief Socket Channel numbers. Use this enum to perform transactions to
 * target sockets.
 *
 */
enum W5100SCH { CH0 = 0, CH1 = 1, CH2 = 2, CH3 = 3 };

enum W5100Proto {
  PROTO_CLOSED = 0x0,
  PROTO_TCP = 0x1,
  PROTO_UDP = 0x2,
  PROTO_IPRAW = 0x3,
  PROTO_MACRAW = 0x4,
  PROTO_PPPOE = 0x5
};

enum W5100SockCmds {
  SOCK_OPEN = 0x01,
  // TCP Mode Only. Waits for connection request from remote peer (TCP Client).
  SOCK_LISTEN = 0x02,
  // TCP Mode Only. Sends connection request to remote peer (TCP Server).
  SOCK_CONNECT = 0x04,
  // TCP Mode Only. Sends connecion termination request.
  SOCK_DISCON = 0x08,
  SOCK_CLOSE = 0x10,
  // Transmit data up to size of TX write pointer
  SOCK_SEND = 0x20,
  // UDP Mode only. Performs send operation without ARP and uses DHAR
  SOCK_SEND_MAC = 0x21,
  // TCP Mode Only. Check connection status by sending 1 byte.
  SOCK_SEND_KEEP = 0x22,
  SOCK_RECV = 0x40,
};

/**
 * @brief Socket N Status Register States.
 * Sn_SR (Socket n Status Register) [R] [0x0403, 0x0503, 0x0603, 0x0703] [0x00]
 */
enum W5100State {
  // All connection resources are released.
  SNSR_CLOSED = 0x00,
  SNSR_INIT = 0x13,
  SNSR_LISTEN = 0x14,
  SNSR_ESTABLISHED = 0x17,
  SNSR_CLOSE_WAIT = 0x1C,
  SNSR_UDP = 0x22,
  SNSR_IPRAW = 0x32,
  SNSR_MACRAW = 0x42,
  SNSR_PPPOE = 0x5F,
  SNSR_SYNSENT = 0x15,
  SNSR_SYNRECV = 0x16,
  SNSR_FIN_WAIT = 0x18,
  SNSR_CLOSING = 0x1A,
  SNSR_TIME_WAIT = 0x1B,
  SNSR_LAST_ACK = 0x1D,
  SNSR_ARP = 0x01
};

void w5100_read_byte(uint16_t addr, uint8_t *buffer);
void w5100_write_byte(uint16_t addr, const uint8_t buffer);
uint16_t w5100_read_bytes(uint16_t addr, uint8_t *buffer, uint16_t len);
uint16_t w5100_write_bytes(uint16_t addr, const uint8_t *buffer, uint16_t len);

/**
 * @brief Common 8 bit register read and write functions
 *
 */
#define __CREGISTER8(name, address)                                            \
  static void w5100_write_##name(const uint8_t _data) {                        \
    w5100_write_byte(address, _data);                                          \
  }                                                                            \
  static uint8_t w5100_read_##name(void) {                                     \
    uint8_t data;                                                              \
    w5100_read_byte(address, &data);                                           \
    return data;                                                               \
  }

/**
 * @brief Common N-bit register read and write functions
 *
 */
#define __CREGISTER_N(name, address, size)                                     \
  static uint16_t w5100_write_##name(const uint8_t *_buffer) {                 \
    return w5100_write_bytes(address, _buffer, size);                          \
  }                                                                            \
  static uint16_t w5100_read_##name(uint8_t *_buffer) {                        \
    return w5100_read_bytes(address, _buffer, size);                           \
  }

/**
 * @brief Nth Socket 8 bit register read and write functions
 *
 */
#define __SREGISTER8(name, address)                                            \
  static void w5100_write_sn_##name(const enum W5100SCH _s,                    \
                                    const uint8_t _data) {                     \
    w5100_write_byte(W5100_MEMAP_SREG_BASE(_s) + address, _data);              \
  }                                                                            \
  static uint8_t w5100_read_sn_##name(enum W5100SCH _s) {                      \
    uint8_t buffer;                                                            \
    w5100_read_byte(W5100_MEMAP_SREG_BASE(_s) + address, &buffer);             \
    return buffer;                                                             \
  }

/**
 * @brief Nth Socket N-bit register read and write functions
 *
 */
#define __SREGISTER_N(name, address, size)                                     \
  static uint16_t w5100_write_sn_##name(const enum W5100SCH _s,                \
                                        const uint8_t *_buff) {                \
    return w5100_write_bytes(W5100_MEMAP_SREG_BASE(_s) + address, _buff,       \
                             size);                                            \
  }                                                                            \
  static uint16_t w5100_read_sn_##name(enum W5100SCH _s, uint8_t *_buff) {     \
    return w5100_read_bytes(W5100_MEMAP_SREG_BASE(_s) + address, _buff, size); \
  }

w5100_status_t w5100_configure(void);
uint16_t w5100_get_tx_offset(enum W5100SCH _s);
uint16_t w5100_get_rx_offset(enum W5100SCH _s);
uint16_t w5100_get_tx_mask(enum W5100SCH _s);
uint16_t w5100_get_rx_mask(enum W5100SCH _s);
uint16_t w5100_get_tx_size(enum W5100SCH _s);
uint16_t w5100_get_rx_size(enum W5100SCH _s);
uint16_t w5100_read_tx_fsr(enum W5100SCH _s, uint16_t *_buff);
uint16_t w5100_read_rx_rsr(enum W5100SCH _s, uint16_t *_buff);
w5100_status_t w5100_verify_hw(void);
void w5100_exec_sock_cmd(const enum W5100SCH sock,
                         const enum W5100SockCmds cmd);
w5100_status_t w5100_reset(void);

/**
 * @brief Common register definitions
 *
 * 4.1 Common Registers
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
__CREGISTER8(mr, 0x0000)       // mode register
__CREGISTER_N(gar, 0x0001, 4)  // gateway address register
__CREGISTER_N(subr, 0x0005, 4) // subnet mask address register
__CREGISTER_N(shar, 0x0009, 6) // source hardware address register
__CREGISTER_N(sipr, 0x000F, 4) // source ip address register
__CREGISTER8(ir, 0x0015)       // interrupt register
__CREGISTER8(imr, 0x0016)      // interrupt mask register
__CREGISTER_N(rtr, 0x0017, 2)  // retry time-value register
__CREGISTER8(rcr, 0x0019)      // retry count register
__CREGISTER8(rmsr, 0x001A)     // rx memory size register
__CREGISTER8(tmsr, 0x001B)     // tx memory size register
__CREGISTER8(patr, 0x001C)     // authentication type in PPPoE mode
__CREGISTER8(ptimer,
             0x0028) // PPP link control protocol request timer register
__CREGISTER8(pmagic,
             0x0029) // PPP link control protocol magic number register
__CREGISTER_N(uipr, 0x002A, 4)  // unreachable ip address register (UDP Mode)
__CREGISTER_N(uport, 0x002E, 2) // unreachable port register (UDP Mode)

/**
 * @brief Socket register definitions
 *
 * 4.2 Socket Registers
 */
__SREGISTER8(mr, 0x0000)        // Mode
__SREGISTER8(cr, 0x0001)        // Command
__SREGISTER8(ir, 0x0002)        // Interrupt
__SREGISTER8(sr, 0x0003)        // Status
__SREGISTER_N(port, 0x0004, 2)  // Source Port
__SREGISTER_N(dhar, 0x0006, 6)  // Destination Hardw Addr
__SREGISTER_N(dipr, 0x000C, 4)  // Destination IP Addr
__SREGISTER_N(dport, 0x0010, 2) // Destination Port
__SREGISTER_N(mssr, 0x0012, 2)  // Max Segment Size
__SREGISTER8(proto, 0x0014)     // Protocol in IP RAW Mode
__SREGISTER8(tos, 0x0015)       // IP TOS
__SREGISTER8(ttl, 0x0016)       // IP TTL
__SREGISTER8(rx_size, 0x001E)   // RX Memory Size (W5200 only)
__SREGISTER8(tx_size, 0x001F)   // RX Memory Size (W5200 only)
__SREGISTER_N(tx_rd, 0x0022, 2) // TX Read Pointer
__SREGISTER_N(tx_wr, 0x0024, 2) // TX Write Pointer
__SREGISTER_N(rx_rd, 0x0028, 2) // RX Read Pointer
__SREGISTER_N(rx_wr, 0x002A, 2) // RX Write Pointer (supported?)
#pragma GCC diagnostic pop

#endif // __W5100_H__