/**
 * @file socket.c
 * @brief
 * @version 0.1
 * @date 2023-01
 *
 * @copyright Copyright © 2023 dronectl
 *
 */

#include "socket.h"
#include "assert.h"

typedef struct socket_state_t {
  uint16_t rx_rsr; // number of bytes received
  uint16_t rx_rd;  // address to read
  uint16_t tx_fsr; // free space ready for transmit
  uint8_t rx_inc;  // rx_rd increment counter
} socket_state_t;

static socket_state_t socket_state[W5100_MAX_SOCK_NUM];

/**
 * @brief Find an available socket to configure with target protocol and source
 * port.
 *
 * @param protocol
 * @param port
 * @return uint8_t
 */
socket_status_t socket_begin(enum W5100Proto protocol, uint16_t port,
                             enum W5100SCH *channel) {
  uint8_t sock_idx;
  enum W5100State socket_status[W5100_MAX_SOCK_NUM];
  assert(channel != NULL);
  if (!ethernet_phy_state()) {
    *channel = W5100_MAX_SOCK_NUM;
    return SOCKET_ERR;
  }
  // find an unused socket and attempt to provision first available
  spi_begin(w5100_spi_config);
  for (sock_idx = 0; sock_idx < W5100_MAX_SOCK_NUM; sock_idx++) {
    socket_status[sock_idx] = (enum W5100State)w5100_read_sn_sr(sock_idx);
    if (socket_status[sock_idx] == SNSR_CLOSED)
      goto make;
  }
  // attempt to forcibly close socket in closing state
  for (sock_idx = 0; sock_idx < W5100_MAX_SOCK_NUM; sock_idx++) {
    if (socket_status[sock_idx] == SNSR_LAST_ACK)
      goto closemake;
    if (socket_status[sock_idx] == SNSR_TIME_WAIT)
      goto closemake;
    if (socket_status[sock_idx] == SNSR_FIN_WAIT)
      goto closemake;
    if (socket_status[sock_idx] == SNSR_CLOSING)
      goto closemake;
  }
  spi_end();
  // failed to provision socket
  *channel = W5100_MAX_SOCK_NUM;
  return SOCKET_FULL;
closemake:
  w5100_exec_sock_cmd(sock_idx, SOCK_CLOSE);
make:
  w5100_write_sn_mr(sock_idx, (const uint8_t)protocol);
  w5100_write_sn_ir(sock_idx, 0xFF); // clear interrupt register
  w5100_write_sn_port(sock_idx, (const uint8_t *)&port);
  w5100_exec_sock_cmd(sock_idx, SOCK_OPEN);
  socket_state[sock_idx].rx_inc = 0;
  socket_state[sock_idx].rx_rd = 0;
  socket_state[sock_idx].rx_rsr = 0;
  socket_state[sock_idx].tx_fsr = 0;
  spi_end();
  return sock_idx;
}

/**
 * @brief Close target socket
 *
 * @param channel target socket
 * @return socket_status_t
 */
socket_status_t socket_close(enum W5100SCH channel) {
  if (!ethernet_phy_state()) {
    return SOCKET_UNINITIALIZED;
  }
  spi_begin(w5100_spi_config);
  w5100_exec_sock_cmd(channel, SOCK_CLOSE);
  spi_end();
  return SOCKET_OK;
}

/**
 * @brief Set socket to listen (server) mode. Socket must be INIT state to
 * transition to LISTEN.
 *
 * @param channel target socket
 * @return socket_status_t
 */
socket_status_t socket_listen(enum W5100SCH channel) {
  if (!ethernet_phy_state()) {
    return SOCKET_UNINITIALIZED;
  }
  // socket must be init state to transistion to listen
  if (socket_get_status(channel) != SNSR_INIT) {
    return SOCKET_ERR;
  }
  spi_begin(w5100_spi_config);
  w5100_exec_sock_cmd(channel, SOCK_LISTEN);
  spi_end();
  return SOCKET_OK;
}

/**
 * @brief Get state of target socket
 *
 * @param channel target socket
 * @return enum W5100State
 */
enum W5100State socket_get_status(enum W5100SCH channel) {
  if (!ethernet_phy_state()) {
    return SOCKET_UNINITIALIZED;
  }
  spi_begin(w5100_spi_config);
  enum W5100State state = (enum W5100State)w5100_read_sn_sr(channel);
  spi_end();
  return state;
}

/**
 * @brief Connect target socket. Establish a TCP connection in active (client)
 * mode
 *
 * @param channel target socket
 * @param addr destination ip address
 * @param port destination port
 * @return socket_status_t
 */
socket_status_t socket_connect(enum W5100SCH channel,
                               const ipv4_address_t *addr, uint16_t port) {
  if (!ethernet_phy_state()) {
    return SOCKET_UNINITIALIZED;
  }
  spi_begin(w5100_spi_config);
  w5100_write_sn_dipr(channel, addr->bytes);
  w5100_write_sn_dport(channel, (const uint8_t *)&port);
  w5100_exec_sock_cmd(channel, SOCK_CONNECT);
  spi_end();
  return SOCKET_OK;
}

/**
 * @brief Disconnect target socket
 *
 * @param channel target socket
 * @return socket_status_t
 */
socket_status_t socket_disconnect(enum W5100SCH channel) {
  if (!ethernet_phy_state()) {
    return SOCKET_UNINITIALIZED;
  }
  spi_begin(w5100_spi_config);
  w5100_exec_sock_cmd(channel, SOCK_DISCON);
  spi_end();
  return SOCKET_OK;
}

// TODO: Remove if rsr is validated
#if 0
static uint16_t get_rx_rsr(enum W5100SCH channel) {
  uint16_t val, prev;
  w5100_read_sn_rx_rsr(channel, (uint8_t *)&prev);
  while (1) {
    w5100_read_sn_rx_rsr(channel, (uint8_t *)&val);
    if (val == prev) {
      return val;
    }
    prev = val;
  }
}
#endif

/**
 * @brief Request recieve data from target socket. If the available bytes is
 * less than the requested size return SOCKET_NO_DATA or SOCKET_EOF
 *
 * @param channel target socket
 * @param buffer data buffer
 * @param size size in bytes
 * @return socket_status_t
 */
socket_status_t socket_recv(enum W5100SCH channel, uint8_t *buffer,
                            uint16_t size) {
  assert(buffer != NULL);
  if (!ethernet_phy_state()) {
    return SOCKET_UNINITIALIZED;
  }
  socket_status_t state = SOCKET_OK;
  uint16_t rx_buf_size = socket_state[channel].rx_rsr;
  spi_begin(w5100_spi_config);
  if (rx_buf_size < size) {
    uint16_t rsr;
    w5100_read_rx_rsr(channel, &rsr);
    // compute unread rx buffer size
    rx_buf_size = rsr - socket_state[channel].rx_inc;
    // update state
    socket_state[channel].rx_rsr = rx_buf_size;
  }
  // now check unread rx buffer size
  if (rx_buf_size == 0) {
    // no unread data is available. Check status of socket
    enum W5100State status = (enum W5100State)w5100_read_sn_sr(channel);
    if (status == SNSR_LISTEN || status == SNSR_CLOSED ||
        status == SNSR_CLOSE_WAIT) {
      // the remote has closed its connection
      state = SOCKET_EOF;
    } else {
      // connection up but no data is available
      state = SOCKET_NO_DATA;
    }
  }
  spi_end();
  return state;
}

/**
 * @brief Get number of unread bytes available in the RX buffer.
 *
 * @param channel target socket channel
 * @return socket_status_t
 */
socket_status_t socket_get_unread_rx_bytes(enum W5100SCH channel) {
  uint16_t rx_buf_size = socket_state[channel].rx_rsr;
  if (!ethernet_phy_state()) {
    return SOCKET_UNINITIALIZED;
  }
  // if local state copy reports 0 get new bytes from device
  if (rx_buf_size == 0) {
    spi_begin(w5100_spi_config);
    uint16_t rsr;
    w5100_read_rx_rsr(channel, &rsr);
    spi_end();
    // subtract the total number of bytes already read
    rx_buf_size = rsr - socket_state[channel].rx_inc;
    // update state rx bytes
    socket_state[channel].rx_rsr = rx_buf_size;
  }
  return rx_buf_size;
}

socket_status_t socket_peek(enum W5100SCH channel, uint8_t *buffer) {
  if (!ethernet_phy_state()) {
    return SOCKET_UNINITIALIZED;
  }
  assert(buffer != NULL);
  spi_begin(w5100_spi_config);
  spi_end();
}