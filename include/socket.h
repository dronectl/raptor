/**
 * @file socket.h
 * @brief
 * @version 0.1
 * @date 2023-01
 *
 * @copyright Copyright © 2023 dronectl
 *
 */

#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "ethernet.h"
#include "w5100.h"
#include <stdint.h>

/**
 * @brief Socket API Status Codes
 *
 */
typedef uint8_t socket_status_t;

#define SOCKET_OK (socket_status_t)0      // success code
#define SOCKET_ERR (socket_status_t)1     // generic failure
#define SOCKET_BUSY (socket_status_t)2    // requested socket is already active
#define SOCKET_NO_DATA (socket_status_t)3 // no data available in RX buffer
#define SOCKET_EOF (socket_status_t)4     // socket closed during operation
#define SOCKET_UNINITIALIZED (socket_status_t)5 // socket uninitialized

socket_status_t socket_begin(enum W5100SCH channel, enum W5100Proto protocol,
                             uint16_t port);
socket_status_t socket_close(enum W5100SCH channel);
socket_status_t socket_listen(enum W5100SCH channel);
enum W5100State socket_get_status(enum W5100SCH channel);
socket_status_t socket_connect(enum W5100SCH channel,
                               const ipv4_address_t *addr, uint16_t port);
socket_status_t socket_disconnect(enum W5100SCH channel);
socket_status_t socket_recv(enum W5100SCH channel, uint8_t *buffer,
                            uint16_t size);
socket_status_t socket_get_unread_rx_bytes(enum W5100SCH channel);
socket_status_t socket_peek(enum W5100SCH channel, uint8_t *buffer);
socket_status_t socket_send(enum W5100SCH channel, const uint8_t *buffer,
                            uint16_t len);
uint16_t socket_send_available(enum W5100SCH channel);
socket_status_t socket_parse_dest_udp_addr(enum W5100SCH channel,
                                           ipv4_address_t *addr,
                                           uint16_t *port);
socket_status_t socket_start_udp(enum W5100SCH channel,
                                 const ipv4_address_t *addr, uint16_t port);
socket_status_t socket_send_udp(enum W5100SCH channel);
socket_status_t socket_buffer_data(enum W5100SCH channel, uint16_t offset,
                                   const uint8_t *buffer, uint16_t len);
#endif // __SOCKET_H__