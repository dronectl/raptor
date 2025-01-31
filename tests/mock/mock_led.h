
#pragma once

#include <stdint.h>
#include "gmock/gmock.h"

class MockLED {
public:
  MOCK_METHOD(void, led_init, (const struct led_ctx *));
  MOCK_METHOD(void, led_disable, (const struct led_ctx *));
  MOCK_METHOD(void, led_enable, (const struct led_ctx *));
  MOCK_METHOD(void, led_periodic_toggle, (struct led_ctx *, const uint32_t));
  MOCK_METHOD(void, led_toggle, (const struct led_ctx *));
};

MockLED *mock_led = nullptr;

// C-style wrapper functions for the mocks
extern "C" {
#include "led.h"

void led_init(const struct led_ctx *ctx) {
  return mock_led->led_init(ctx);
}

void led_disable(const struct led_ctx *ctx) {
  return mock_led->led_disable(ctx);
}

void led_enable(const struct led_ctx *ctx) {
  return mock_led->led_enable(ctx);
}

void led_toggle(const struct led_ctx *ctx) {
  return mock_led->led_toggle(ctx);
}

void led_periodic_toggle(struct led_ctx *ctx, const uint32_t rate_ms) {
  return mock_led->led_periodic_toggle(ctx, rate_ms);
}
}