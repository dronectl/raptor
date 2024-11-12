
#include "led.h"
#include "uassert.h"
#include "stm32h7xx_hal_gpio.h"

void led_init(const struct led_handle *ctx) {
  uassert(ctx != NULL);
  led_disable(ctx);
}

void led_enable(const struct led_handle *ctx) {
  uassert(ctx != NULL);
  GPIO_PinState state = ctx->active_high ? GPIO_PIN_SET : GPIO_PIN_RESET;
  HAL_GPIO_WritePin(ctx->port, ctx->pin, state);
}

void led_disable(const struct led_handle *ctx) {
  uassert(ctx != NULL);
  GPIO_PinState state = ctx->active_high ? GPIO_PIN_RESET : GPIO_PIN_SET;
  HAL_GPIO_WritePin(ctx->port, ctx->pin, state);
}

void led_toggle(const struct led_handle *ctx) {
  uassert(ctx != NULL);
  HAL_GPIO_TogglePin(ctx->port, ctx->pin);
}

