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

#include "led.h"
#include "bme280.h"
#include "stm32h7xx.h" // IWYU pragma: export

#define HEALTH_TASK_DEFAULT_TICK_RATE_MS (uint16_t)1000

enum health_status_leds {
  HEALTH_STATUS_LED_OK = 0,
  HEALTH_STATUS_LED_ERR,
  HEALTH_STATUS_LED_EVENT,
  HEALTH_STATUS_LED_COUNT
};

enum health_states {
  HEALTH_STATE_NULL,    // unknown and null state
  HEALTH_STATE_RESET,   // reset state
  HEALTH_STATE_INIT,    // initialization state
  HEALTH_STATE_REPORT,  // alive reporting
  HEALTH_STATE_READ,    // sensor read
  HEALTH_STATE_SERVICE, // service watchdog
  HEALTH_STATE_ERROR, // error state
  HEALTH_STATE_COUNT // number of health states
};

struct health_telemetry {
  float humidity; // %
  float pressure; // KPa
  float ambient_temperature; // ˚C
};

struct health_ctx {
  uint16_t service_bits;
  uint16_t tick_rate_ms;
  enum health_states current_state;
  struct health_telemetry telemetry;
  I2C_HandleTypeDef *i2c;
  struct led_handle leds[HEALTH_STATUS_LED_COUNT];
  struct bme280_dev bme280;
};

void health_init(struct health_ctx *ctx);

#endif // __HEALTH_H__
