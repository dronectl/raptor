#include "health.h"
#include "bme280.h"
#include "cmsis_os2.h"
#include "logger.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_nucleo.h"

static health_report_t health_report;
bme280_dev_t bme280;
static bme280_meas_t bme280_meas;

#ifndef RAPTOR_DEBUG
static void service_watchdog(void) {
  HAL_StatusTypeDef status;
  status = HAL_IWDG_Refresh(&iwdg_handle);
  // if (status != HAL_OK)
  // TODO: Handle error
  // EHANDLE(status);
}
#endif // RAPTOR_DEBUG

static enum HealthState fsm_tick(const enum HealthState state) {
  enum HealthState next_state;
  switch (state) {
    case HEALTH_INIT:
      // initialize drivers
      bme280_init(&bme280);
      next_state = HEALTH_SERVICE;
      break;
    case HEALTH_RESET:
      // teardown for soft reboot
      bme280_reset(&bme280);
      next_state = HEALTH_INIT;
      break;
    case HEALTH_SERVICE:
      // service watchdog / others
#ifndef RAPTOR_DEBUG
      service_watchdog();
#endif // RAPTOR_DEBUG
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

__NO_RETURN void health_main(void *argument) {
  enum HealthState state = HEALTH_INIT;
  // get i2c2 handle and set bme280
  I2C_HandleTypeDef hi2c = *(I2C_HandleTypeDef *)argument;
  bme280.i2c = hi2c;
  // info("Starting health task FSM");
  while (1) {
    BSP_LED_Toggle(LED1);
    // state = fsm_tick(state);
    osDelay(1000);
    // info("toggling");
  }
}
