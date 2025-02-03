
#include "led.h"
#include "uassert.h"

#include <string.h>

void led_init(struct led_context *ctx, const struct led_init_context *init) {
  uassert(init != NULL);
  uassert(ctx != NULL);
  ctx->init = init;
  led_disable(ctx);
}

void led_enable(const struct led_context *ctx) {
  uassert(ctx != NULL);
  uassert(ctx->init != NULL);
  GPIO_PinState state = ctx->init->active_high ? GPIO_PIN_SET : GPIO_PIN_RESET;
  HAL_GPIO_WritePin(ctx->init->port, ctx->init->pin, state);
}

void led_disable(const struct led_context *ctx) {
  uassert(ctx != NULL);
  uassert(ctx->init != NULL);
  GPIO_PinState state = ctx->init->active_high ? GPIO_PIN_RESET : GPIO_PIN_SET;
  HAL_GPIO_WritePin(ctx->init->port, ctx->init->pin, state);
}

void led_toggle(const struct led_context *ctx) {
  uassert(ctx != NULL);
  uassert(ctx->init != NULL);
  HAL_GPIO_TogglePin(ctx->init->port, ctx->init->pin);
}

void led_periodic_toggle(struct led_context *ctx, const uint32_t toggle_rate_ms) {
  uassert(ctx != NULL);
  uassert(ctx->init != NULL);
  const uint32_t current_time = HAL_GetTick();
  const uint32_t elapsed = current_time - ctx->last_toggle_ms;
  if (elapsed >= toggle_rate_ms) {
    HAL_GPIO_TogglePin(ctx->init->port, ctx->init->pin);
    ctx->last_toggle_ms = current_time;
  }
}
