
#include "led.h"
#include "uassert.h"

#include <stm32h7xx_hal_gpio.h>
#include <string.h>

void led_init(const struct led_ctx *ctx) {
  uassert(ctx != NULL);
  led_disable(ctx);
}

void led_enable(const struct led_ctx *ctx) {
  uassert(ctx != NULL);
  GPIO_PinState state = ctx->init_ctx->active_high ? GPIO_PIN_SET : GPIO_PIN_RESET;
  HAL_GPIO_WritePin(ctx->init_ctx->port, ctx->init_ctx->pin, state);
}

void led_disable(const struct led_ctx *ctx) {
  uassert(ctx != NULL);
  GPIO_PinState state = ctx->init_ctx->active_high ? GPIO_PIN_RESET : GPIO_PIN_SET;
  HAL_GPIO_WritePin(ctx->init_ctx->port, ctx->init_ctx->pin, state);
}

void led_toggle(const struct led_ctx *ctx) {
  uassert(ctx != NULL);
  HAL_GPIO_TogglePin(ctx->init_ctx->port, ctx->init_ctx->pin);
}

void led_periodic_toggle(struct led_ctx *ctx, const uint32_t toggle_rate_ms) {
  uassert(ctx != NULL);
  const uint32_t current_time = HAL_GetTick();
  const uint32_t elapsed = current_time - ctx->last_toggle_ms;
  if (elapsed >= toggle_rate_ms) {
    HAL_GPIO_TogglePin(ctx->init_ctx->port, ctx->init_ctx->pin);
    ctx->last_toggle_ms = current_time;
  }
}

