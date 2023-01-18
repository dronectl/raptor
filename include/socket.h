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

#include "w5100.h"
#include <stdint.h>

/**
 * @brief Ethernet Proxy API Status Codes
 *
 */
typedef uint8_t socket_status_t;

#define SOCKET_OK (socket_status_t)0   // success code
#define SOCKET_ERR (socket_status_t)1  // generic failure
#define SOCKET_FULL (socket_status_t)2 // no available socket

socket_status_t socket_begin(uint8_t protocol, uint16_t port,
                             enum W5100SCH *channel);
socket_status_t socket_close(enum W5100SCH channel);
socket_status_t socket_status(enum W5100SCH channel);
socket_status_t socket_connect(enum W5100SCH channel);
socket_status_t socket_disconnect(enum W5100SCH channel);

#endif // __SOCKET_H__