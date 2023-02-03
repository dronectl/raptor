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
 * @brief Write data to taret circular socket tx buffer.
 *
 * @param channel target socket channel
 * @param data_offset write pointer offset
 * @param buffer data to be written
 * @param len size of data to be written
 */
static void send_data(enum W5100SCH channel, uint16_t data_offset,
                      const uint8_t *buffer, uint16_t len) {
  uint16_t ptr;
  uint16_t offset;
  uint16_t dst_addr;
  uint16_t size = w5100_read_sn_tx_size(channel);
  w5100_read_sn_tx_wr(channel, (uint8_t *)&ptr);
  ptr += data_offset;
  offset = ptr & w5100_get_tx_mask(channel);
  dst_addr = offset + w5100_get_tx_offset(channel);
  // check if we need to rollover to front of cbuf
  if (data_offset + len <= size) {
    w5100_write_bytes(dst_addr, buffer, len);
  } else {
    // rollover op required. first read to end of buffer
    size = size - offset;
    w5100_write_bytes(dst_addr, buffer, size);
    // increment data buffer by number of bytes written by the former write
    w5100_write_bytes(w5100_get_tx_offset(channel), buffer + size, len - size);
  }
  // increment write pointer by number of bytes written
  ptr += len;
  w5100_write_sn_tx_wr(channel, (uint8_t *)&ptr);
}

/**
 * @brief Read data from target circular socket RX buffer.
 *
 * @param channel target socket channel
 * @param src read pointer index
 * @param dst read buffer
 * @param len size of read (bytes)
 */
static void read_data(enum W5100SCH channel, uint16_t src, uint8_t *dst,
                      uint16_t len) {
  uint16_t size = w5100_get_rx_size(channel);
  uint16_t data_offset = src & w5100_get_rx_mask(channel);
  uint16_t src_ptr = w5100_get_rx_offset(channel) + data_offset;
  // check if we need to roll over to the front of the buffer
  if (data_offset + len <= size) {
    // no rollover required. read to len
    w5100_read_bytes(src_ptr, dst, len);
  } else {
    // rollover required. first read to end of buffer
    size = size - data_offset;
    w5100_read_bytes(src_ptr, dst, size);
    // then read the remaining bytes in len
    dst += size;
    w5100_read_bytes(w5100_get_rx_offset(channel), dst, len - size);
  }
}

/**
 * @brief Provision a socket for communication.
 *
 * @param channel socket channel [0-3]
 * @param protocol socket protocol
 * @param port socket port
 * @return socket_status_t SOCKET_BUSY if socket failed to be provisioned.
 * SOCKET_UNINITIALIZED if ethernet phy is uninitialized and SOCKET_OK on
 * success
 */
socket_status_t socket_begin(enum W5100SCH channel, enum W5100Proto protocol,
                             uint16_t port) {
  enum W5100State socket_status;
  if (!ethernet_phy_state()) {
    return SOCKET_UNINITIALIZED;
  }
  // find an unused socket and attempt to provision first available
  spi_begin(w5100_spi_config);
  socket_status = (enum W5100State)w5100_read_sn_sr(channel);
  switch (socket_status) {
    case SNSR_CLOSED:
      goto make;
    case SNSR_LAST_ACK:
      goto closemake;
    case SNSR_TIME_WAIT:
      goto closemake;
    case SNSR_FIN_WAIT:
      goto closemake;
    case SNSR_CLOSING:
      goto closemake;
    default:
      return SOCKET_ERR;
  }
  spi_end();
  // failed to provision socket
  return SOCKET_BUSY;
closemake:
  w5100_exec_sock_cmd(channel, SOCK_CLOSE);
make:
  w5100_write_sn_mr(channel, (const uint8_t)protocol);
  w5100_write_sn_ir(channel, 0xFF); // clear interrupt register
  w5100_write_sn_port(channel, (const uint8_t *)&port);
  w5100_exec_sock_cmd(channel, SOCK_OPEN);
  socket_state[channel].rx_inc = 0;
  socket_state[channel].rx_rd = 0;
  socket_state[channel].rx_rsr = 0;
  socket_state[channel].tx_fsr = 0;
  spi_end();
  return SOCKET_OK;
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
  // bytes received
  uint16_t rx_rsr = socket_state[channel].rx_rsr;
  spi_begin(w5100_spi_config);
  if (rx_rsr < size) {
    uint16_t rsr;
    w5100_read_rx_rsr(channel, &rsr);
    // compute unread rx buffer size
    rx_rsr = rsr - socket_state[channel].rx_inc;
    // update rsr state to reflect unread rx buffer size
    socket_state[channel].rx_rsr = rx_rsr;
  }
  // now check unread rx buffer size
  if (rx_rsr == 0) {
    // no unread data is available. Check status of socket and categorize error
    enum W5100State status = (enum W5100State)w5100_read_sn_sr(channel);
    if (status == SNSR_LISTEN || status == SNSR_CLOSED ||
        status == SNSR_CLOSE_WAIT) {
      // the remote has closed its connection
      state = SOCKET_EOF;
      goto cleanup;
    } else {
      // connection up but no data is available
      state = SOCKET_NO_DATA;
      goto cleanup;
    }
  }
  // clamp rx receive bytes to `size` (do not receive more than requested)
  if (rx_rsr > size)
    rx_rsr = size;
  read_data(channel, socket_state[channel].rx_rd, buffer, size);
  // incremement read address by bytes read
  socket_state[channel].rx_rd += rx_rsr;
  // decrement read bytes by number of bytes read
  socket_state[channel].rx_rsr -= rx_rsr;
  uint16_t inc = socket_state[channel].rx_inc + rx_rsr;
  // REVIEW: clamp incremement count to 250?
  if (inc >= 250 || socket_state[channel].rx_rsr == 0) {
    // receive step complete set socket to SOCK_RECV and reset increment
    socket_state[channel].rx_inc = 0;
    // move socket read pointer
    w5100_write_sn_rx_rd(channel, (uint8_t *)socket_state[channel].rx_rd);
    w5100_exec_sock_cmd(channel, SOCK_RECV);
  } else {
    socket_state[channel].rx_inc = inc;
  }
cleanup:
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

/**
 * @brief Get first byte in receive queue
 *
 * @param channel target socket channel
 * @param buffer byte buffer
 * @return socket_status_t
 */
socket_status_t socket_peek(enum W5100SCH channel, uint8_t *buffer) {
  if (!ethernet_phy_state()) {
    return SOCKET_UNINITIALIZED;
  }
  assert(buffer != NULL);
  uint16_t ptr = socket_state[channel].rx_rd;
  uint16_t mask = w5100_get_rx_mask(channel);
  uint16_t offset = w5100_get_rx_offset(channel);
  spi_begin(w5100_spi_config);
  w5100_read_byte((ptr & mask) + offset, buffer);
  spi_end();
  return SOCKET_OK;
}

/**
 * @brief Send data to target socket. Blocks until target socket tx buffer has
 * the space to write the buffer and verifies the data dispatch was successful.
 *
 * @param channel target socket channel
 * @param buffer buffer to write
 * @param len size of buffer
 * @return socket_status_t
 */
socket_status_t socket_send(enum W5100SCH channel, const uint8_t *buffer,
                            uint16_t len) {
  uint16_t freesize;
  enum W5100State state;
  socket_status_t status = SOCKET_OK;
  assert(buffer != NULL);
  uint16_t max_size = w5100_get_tx_size(channel);

  // clamp bytes to send by max tx buffer size
  if (len > max_size) {
    len = max_size;
  }

  // only start write once free buffer space is available
  // TODO: do not hold spi bus while waiting for FSR
  spi_begin(w5100_spi_config);
  do {
    w5100_read_tx_fsr(channel, &freesize);
    state = (enum W5100State)w5100_read_sn_sr(channel);
    if ((state != SNSR_ESTABLISHED) && (state != SNSR_CLOSE_WAIT)) {
      // REVIEW: close if in bad state?
      goto cleanup;
    }
  } while (freesize < len);

  // write data
  send_data(channel, 0, buffer, len);
  w5100_exec_sock_cmd(channel, SOCK_SEND);

  // read interrupt register to verify send operation completion
  while ((w5100_read_sn_ir(channel) & SNIR_SENDOK) != SNIR_SENDOK) {
    // check if the socket closed during transaction
    if (w5100_read_sn_sr(channel) == SNSR_CLOSED) {
      status = SOCKET_EOF;
      goto cleanup;
    }
  }
  // clear SEND_OK interrupt flag
  w5100_write_sn_ir(channel, SNIR_SENDOK);
cleanup:
  spi_end();
  return status;
}

/**
 * @brief Get number of bytes available for transmission on target socket. If
 * the socket is not in ESTABLISHED or CLOSE_WAIT return 0
 *
 * @param channel target socket
 * @return uint16_t number of bytes available for transmit
 */
uint16_t socket_send_available(enum W5100SCH channel) {
  enum W5100State state;
  uint16_t freesize;
  spi_begin(w5100_spi_config);
  w5100_read_tx_fsr(channel, &freesize);
  state = (enum W5100State)w5100_read_sn_sr(channel);
  spi_end();
  if ((state == SNSR_ESTABLISHED) || (state == SNSR_CLOSE_WAIT)) {
    return freesize;
  }
  return 0;
}

/**
 * @brief Configure target socket to communicate to destination using UDP.
 *
 * @param channel target socket
 * @param addr destination ipv4 address
 * @param port destination port
 * @return socket_status_t
 */
socket_status_t socket_start_udp(enum W5100SCH channel,
                                 const ipv4_address_t *addr, uint16_t port) {
  assert(addr != NULL);
  spi_begin(w5100_spi_config);
  w5100_write_sn_dipr(channel, addr->bytes);
  w5100_write_sn_dport(channel, (uint8_t *)&port);
  spi_end();
  return SOCKET_OK;
}

/**
 * @brief Initiate a UDP send on target socket
 *
 * @param channel target socket
 * @return socket_status_t
 */
socket_status_t socket_send_udp(enum W5100SCH channel) {
  socket_status_t status = SOCKET_OK;
  spi_begin(w5100_spi_config);
  w5100_exec_sock_cmd(channel, SOCK_SEND);
  uint8_t ir;
  do {
    ir = w5100_read_sn_ir(channel);
    if (ir & SNIR_TIMEOUT) {
      w5100_write_sn_ir(channel, SNIR_SENDOK | SNIR_TIMEOUT);
      status = SOCKET_EOF;
      goto cleanup;
    }
  } while ((ir & SNIR_SENDOK) != SNIR_SENDOK);
cleanup:
  spi_end();
  return status;
}

socket_status_t socket_parse_dest_udp_addr(enum W5100SCH channel,
                                           ipv4_address_t *addr,
                                           uint16_t *port) {
  assert(addr != NULL);
  assert(port != NULL);
  // TODO: Should be flushing bytes from previous packet here
  if (socket_get_unread_rx_bytes(channel) > 0) {
    uint8_t temp[8];
    socket_status_t ret = socket_recv(channel, temp, 8);
    if (ret == SOCKET_OK) {
      addr->bytes[0] = temp[0];
      addr->bytes[1] = temp[1];
      addr->bytes[2] = temp[2];
      addr->bytes[3] = temp[3];
      *port = (uint16_t)temp[4];
    }
    return ret;
  }
  return SOCKET_ERR;
}

/**
 * @brief UDP socket buffer for send.
 *
 * @param channel target socket
 * @param offset data offset
 * @param buffer data to transmit
 * @param len length of data (bytes)
 * @return socket_status_t
 */
socket_status_t socket_buffer_data(enum W5100SCH channel, uint16_t offset,
                                   const uint8_t *buffer, uint16_t len) {
  uint16_t freesize;
  spi_begin(w5100_spi_config);
  w5100_read_tx_fsr(channel, &freesize);
  // clamp length by instantantaneous fsr
  if (len > freesize) {
    len = freesize;
  }
  send_data(channel, offset, buffer, len);
  spi_end();
  return SOCKET_OK;
}
