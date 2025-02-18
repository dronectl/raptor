/**
 * @file led.h
 * @author ztnel (christian911@sympatico.ca)
 * @brief General LED control module
 * @version 0.1
 * @date 2024-11
 *
 * @copyright Copyright © 2024 dronectl
 *
 */

#ifndef __LED_H__
#define __LED_H__

#include <stdint.h>
#include <stm32h7xx.h>
#include <stdbool.h>

struct led_init_context {
  GPIO_TypeDef *port;
  uint16_t pin;
  bool active_high;
};

struct led_context {
  const struct led_init_context *init;
  uint32_t last_toggle_ms;
};

/**
 * @brief Initialize LED to inactive state
 *
 * @param[in] ctx LED context
 */
void led_init(struct led_context *ctx, const struct led_init_context *init);

/**
 * @brief Set LED to active state
 *
 * @param[in] ctx LED context
 */
void led_enable(const struct led_context *ctx);

/**
 * @brief Set LED to inactive state
 *
 * @param[in] ctx LED context
 */
void led_disable(const struct led_context *ctx);

/**
 * @brief Toggle LED state
 *
 * @param[in] ctx LED context
 */
void led_toggle(const struct led_context *ctx);

/**
 * @brief Toggle LED state at a fixed rate. Requires periodic call to update state.
 *
 * @param[in] ctx LED context
 * @param[in] rate_ms toggle rate (ms)
 */
void led_periodic_toggle(struct led_context *ctx, const uint32_t rate_ms);

#endif // __LED_H__
