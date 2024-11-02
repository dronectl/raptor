/**
 * @file health.c
 * @brief
 * @version 0.1
 * @date 2024-04
 *
 * @copyright Copyright © 2024 dronectl
 *
 */

#ifndef __HEALTH_H__
#define __HEALTH_H__

#include "system.h"
#include "stm32h7xx.h" // IWYU pragma: export

system_status_t health_init(I2C_HandleTypeDef hi2c);

#endif // __HEALTH_H__
