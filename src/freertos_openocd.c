/**
 * @file freertos_openocd.c
 * @author ztnel (christian911@sympatico.ca)
 * @brief This file enables FreeRTOS task stats collection by OpenOCD. This is only required for
 * DEBUG builds and can be removed for production releases
 * @version 0.1
 * @date 2024-01
 *
 * @copyright Copyright © 2024 dronectl
 *
 */
#include "FreeRTOS.h"

#ifdef __GNUC__
#define USED __attribute__((used))
#else
#define USED
#endif

const volatile int USED uxTopUsedPriority = configMAX_PRIORITIES - 1;
