/**
 * @file app_ethernet.h
 * @brief TCPIP stack powered by LWIP
 * @version 0.1
 * @date 2024-04
 *
 * @copyright Copyright © 2024 dronectl
 *
 */

#ifndef __APP_ETHERNET_H
#define __APP_ETHERNET_H

#include "system.h"

/**
 * @brief Initialize LWIP TCPIP network stack
 * @note CS: LWIP `tcpip_init` implements `LWIP_ASSERT` which implements its own spinlock
 */
system_status_t app_ethernet_init(void);

#endif /* __APP_ETHERNET_H */
