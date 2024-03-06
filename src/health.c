#include "health.h"
#include "FreeRTOS.h"
#include "bme280.h"
#include "logger.h"
#include "stm32h7xx_hal.h"
#include "task.h"

static health_report_t health_report;

// devices
bme280_dev_t bme280;
static bme280_meas_t bme280_meas;

static enum HealthState fsm_tick(const enum HealthState state) {
  enum HealthState next_state;
  switch (state) {
    case HEALTH_INIT:
      bme280_init(&bme280);
      next_state = HEALTH_SERVICE;
      break;
    case HEALTH_RESET:
      // teardown for soft reboot
      bme280_reset(&bme280);
      next_state = HEALTH_INIT;
      break;
    case HEALTH_SERVICE:
      next_state = HEALTH_READ;
      break;
    case HEALTH_READ: {
      // read from alive sensors
      bme280_trigger_read(&bme280, &bme280_meas);
      health_report.humidity = bme280_meas.humidity;
      health_report.pressure = bme280_meas.pressure;
      health_report.temperature = bme280_meas.temperature;
      next_state = HEALTH_REPORT;
      break;
    }
    case HEALTH_REPORT:
      // export health alive telemetry
      next_state = HEALTH_SERVICE;
      break;
    default:
      next_state = HEALTH_NULL;
      break;
  }
  return next_state;
}

void health_main(void *pv_params) {
  const TickType_t delay = 100 / portTICK_PERIOD_MS;
  enum HealthState state = HEALTH_INIT;
  // get i2c2 handle and set bme280
  I2C_HandleTypeDef hi2c2 = *(I2C_HandleTypeDef *)pv_params;
  bme280.i2c = hi2c2;
  info("Starting health task FSM");
  while (1) {
    state = fsm_tick(state);
    vTaskDelay(delay);
  }
}
