
#pragma once

#include <stdint.h>
#include "gmock/gmock.h"

class MockLED {
public:
  MOCK_METHOD(void, led_init, (struct led_context *, const struct led_init_context *));
  MOCK_METHOD(void, led_disable, (const struct led_context *));
  MOCK_METHOD(void, led_enable, (const struct led_context *));
  MOCK_METHOD(void, led_periodic_toggle, (struct led_context *, const uint32_t));
  MOCK_METHOD(void, led_toggle, (const struct led_context *));
};

MockLED *mock_led = nullptr;

// C-style wrapper functions for the mocks
extern "C" {
#include "led.h"

void led_init(struct led_context *ctx, const struct led_init_context *init) {
  return mock_led->led_init(ctx, init);
}

void led_disable(const struct led_context *ctx) {
  return mock_led->led_disable(ctx);
}

void led_enable(const struct led_context *ctx) {
  return mock_led->led_enable(ctx);
}

void led_toggle(const struct led_context *ctx) {
  return mock_led->led_toggle(ctx);
}

void led_periodic_toggle(struct led_context *ctx, const uint32_t rate_ms) {
  return mock_led->led_periodic_toggle(ctx, rate_ms);
}
}