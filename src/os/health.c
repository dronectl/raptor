/**
 * @file health.c
 * @brief
 * @version 0.1
 * @date 2024-04
 *
 * @copyright Copyright © 2024 dronectl
 *
 */

#include "main.h"
#include "health.h"
#include "logger.h"
#include "stm32h7xx_hal.h" // IWYU pragma: export
#include "uassert.h"
#include <FreeRTOS.h>
#include <task.h>

#define SERVICE_BME280_LINK_DOWN_MSK (1 << 0)

/**
 * @brief Health FSM tick runner
 *
 * @param[in,out] ctx Health context
 */
static void fsm_do_tick(struct health_ctx *ctx);

static void led_handler(struct health_ctx *ctx) {
  struct led_handle *hled = ctx->service_bits != 0 ? &ctx->leds[HEALTH_STATUS_LED_ERR] : &ctx->leds[HEALTH_STATUS_LED_OK];
  if (ctx->service_bits != 0) {
    led_disable(&ctx->leds[HEALTH_STATUS_LED_OK]);
  } else {
    led_disable(&ctx->leds[HEALTH_STATUS_LED_ERR]);
  }
  led_toggle(hled);
}

static void fsm_do_tick(struct health_ctx *ctx) {
  enum health_states next_state = HEALTH_STATE_NULL;
  led_handler(ctx);
  switch (ctx->current_state) {
    case HEALTH_STATE_INIT:
      if (bme280_init(&ctx->bme280) != BME280_OK) {
        ctx->service_bits |= SERVICE_BME280_LINK_DOWN_MSK;
      } else {
        ctx->service_bits &= ~SERVICE_BME280_LINK_DOWN_MSK;
      }
      next_state = HEALTH_STATE_SERVICE;
      break;
    case HEALTH_STATE_RESET:
      // teardown for soft reboot
      bme280_reset(&ctx->bme280);
      next_state = HEALTH_STATE_INIT;
      break;
    case HEALTH_STATE_SERVICE:
      if (ctx->service_bits & SERVICE_BME280_LINK_DOWN_MSK) {
        // retry bme280 init
        if (bme280_init(&ctx->bme280) != BME280_OK) {
          ctx->service_bits |= SERVICE_BME280_LINK_DOWN_MSK;
        } else {
          ctx->service_bits &= ~SERVICE_BME280_LINK_DOWN_MSK;
        }
      }
      next_state = HEALTH_STATE_READ;
      break;
    case HEALTH_STATE_READ:
      if (!(ctx->service_bits & SERVICE_BME280_LINK_DOWN_MSK)) {
        bme280_trigger_read(&ctx->bme280, &ctx->telemetry.ambient_temperature, &ctx->telemetry.pressure, &ctx->telemetry.humidity);
      }
      next_state = HEALTH_STATE_REPORT;
      break;
    case HEALTH_STATE_REPORT:
      // export health alive telemetry
      next_state = HEALTH_STATE_SERVICE;
      break;
    case HEALTH_STATE_ERROR:
      next_state = HEALTH_STATE_ERROR;
      break;
    default:                 // intentional fallthrough
    case HEALTH_STATE_COUNT: // intentional fallthrough
    case HEALTH_STATE_NULL:
      uassert(0);
      break;
  }
  ctx->current_state = next_state;
}

/**
 * @brief Health task entry point.
 *
 * @param[in] I2C_HandleTypeDef for bme280 sensor
 */
static void health_main(void *argument) {
  struct health_ctx *ctx = (struct health_ctx *)argument;
  uassert(ctx != NULL);
  ctx->service_bits = 0;
  ctx->current_state = HEALTH_STATE_INIT;
  info("Starting health task FSM\n");
  while (1) {
    HAL_GPIO_TogglePin(BATT_PWR_RELAY_GPIO_Port, BATT_PWR_RELAY_Pin);
    fsm_do_tick(ctx);
    vTaskDelay(ctx->tick_rate_ms / portTICK_PERIOD_MS);
  }
}

/**
 * @brief Health module initialization
 *
 * @param[in] hi2c i2c handle for bme280 environment sensor
 * @return system status code
 */
void health_init(struct health_ctx *ctx) {
  TaskHandle_t health_handle = NULL;
  BaseType_t ret = xTaskCreate(health_main, "health_task", configMINIMAL_STACK_SIZE, ctx, 10, &health_handle);
  uassert(ret == pdPASS);
}
