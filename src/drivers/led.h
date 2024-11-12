/**
 * @file led.h
 * @author ztnel (christian911@sympatio.ca)
 * @brief General LED control module
 * @version 0.1
 * @date 2024-11
 *
 * @copyright Copyright © 2024 dronectl
 *
 */

#ifndef __LED_H__
#define __LED_H__

#include "stm32h723xx.h"
#include <stm32h7xx_hal.h>
#include <stdbool.h>

/**
 * struct led_handle - LED context handle
 */
struct led_handle {
  GPIO_TypeDef* port;
  uint16_t pin;
  bool active_high;
};

/**
 * @brief Initialize LED to inactive state
 *
 * @param[in] ctx LED context
 */
void led_init(const struct led_handle *ctx);

/**
 * @brief Set LED to active state
 *
 * @param[in] ctx LED context
 */
void led_enable(const struct led_handle *ctx);

/**
 * @brief Set LED to inactive state
 *
 * @param[in] ctx LED context
 */
void led_disable(const struct led_handle *ctx);

/**
 * @brief Toggle LED state
 *
 * @param[in] ctx LED context
 */
void led_toggle(const struct led_handle *ctx);

#endif // __LED_H__
