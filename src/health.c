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

#ifndef RAPTOR_DEBUG
static IWDG_HandleTypeDef iwdg_handle;
// https://github.com/STMicroelectronics/STM32CubeH7/blob/master/Projects/NUCLEO-H723ZG/Examples/IWDG/IWDG_WindowMode/Src/main.c
static void configure_watchdog(void) {
  HAL_StatusTypeDef status;
  iwdg_handle.Instance = IWDG1;
  iwdg_handle.Init.Prescaler = IWDG_PRESCALER_16;
  iwdg_handle.Init.Reload = (32000 * 762) / (16 * 1000); /* 762 ms */
  iwdg_handle.Init.Window = (32000 * 400) / (16 * 1000); /* 400 ms */
  status = HAL_IWDG_Init(&iwdg_handle);
  // if (status != HAL_OK) {
  //   // TODO: Handle error
  //   // EHANDLE(status);
  // }
}

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
#ifndef RAPTOR_DEBUG
      configure_watchdog();
#endif // RAPTOR_DEBUG
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