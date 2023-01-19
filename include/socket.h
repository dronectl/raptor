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

#define SOCKET_OK (socket_status_t)0   // success code
#define SOCKET_ERR (socket_status_t)1  // generic failure
#define SOCKET_FULL (socket_status_t)2 // no available socket

socket_status_t socket_begin(enum W5100Proto protocol, uint16_t port,
                             enum W5100SCH *channel);
socket_status_t socket_close(enum W5100SCH channel);
enum W5100State socket_get_status(enum W5100SCH channel);
socket_status_t socket_connect(enum W5100SCH channel,
                               const ipv4_address_t *addr, uint16_t port);
socket_status_t socket_disconnect(enum W5100SCH channel);

socket_status_t socket_recv();

#endif // __SOCKET_H__