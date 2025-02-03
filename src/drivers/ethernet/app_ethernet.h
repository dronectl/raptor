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
 * 
 * @param[in] task_ctx task context
 */
void app_ethernet_init(const struct system_task_context *task_ctx);

#endif /* __APP_ETHERNET_H */
