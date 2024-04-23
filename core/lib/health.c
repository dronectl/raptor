/**
 * @file health.c
 * @brief
 * @version 0.1
 * @date 2024-04
 *
 * @copyright Copyright © 2024 dronectl
 *
 */

#include "cmsis_os2.h"
#include "bme280.h"
#include "health.h"
#include "logger.h"
#include "stm32h7xx_hal.h" // IWYU pragma: export

const osThreadAttr_t health_attr = {
    .name = "health_task",
    .priority = osPriorityNormal1,
};
static osThreadId_t health_handle;

/**
 * @brief Health FSM States
 *
 */
enum state {
  STATE_NULL,    // unknown and null state
  STATE_RESET,   // reset state
  STATE_INIT,    // initialization state
  STATE_REPORT,  // alive reporting
  STATE_READ,    // sensor read
  STATE_SERVICE, // service watchdog
};

/**
 * @brief Low frequency alive data
 *
 */
struct health_report {
  float timestamp;
  uint8_t status;
  float temperature;
  float humidity;
  float pressure;
};

static struct health_report report;
// devices
static bme280_dev_t bme280;
static bme280_meas_t bme280_meas;

/**
 * @brief Health FSM runner.
 *
 * @param[in] current_state current FSM state
 */
static enum state fsm_tick(const enum state current_state) {
  enum state next_state;
  switch (current_state) {
    case STATE_INIT:
      // bme280_init(&bme280);
      next_state = STATE_SERVICE;
      break;
    case STATE_RESET:
      // teardown for soft reboot
      // bme280_reset(&bme280);
      next_state = STATE_INIT;
      break;
    case STATE_SERVICE:
      next_state = STATE_READ;
      break;
    case STATE_READ:
      // read from alive sensors
      // bme280_trigger_read(&bme280, &bme280_meas);
      report.humidity = bme280_meas.humidity;
      report.pressure = bme280_meas.pressure;
      report.temperature = bme280_meas.temperature;
      next_state = STATE_REPORT;
      break;
    case STATE_REPORT:
      // export health alive telemetry
      next_state = STATE_SERVICE;
      break;
    default:
      next_state = STATE_NULL;
      break;
  }
  return next_state;
}

/**
 * @brief Health task entry point.
 *
 * @param[in] I2C_HandleTypeDef for bme280 sensor
 */
static __NO_RETURN void health_main(void *argument) {
  enum state current_state = STATE_INIT;
  // get i2c2 handle and set bme280
  I2C_HandleTypeDef hi2c2 = *(I2C_HandleTypeDef *)argument;
  bme280.i2c = hi2c2;
  info("Starting health task FSM");
  while (1) {
    current_state = fsm_tick(current_state);
    osDelay(1000);
  }
}

/**
 * @brief Health module initialization
 *
 * @param[in] hi2c i2c handle for bme280 environment sensor
 * @return system status code
 */
system_status_t health_init(I2C_HandleTypeDef hi2c) {
  health_handle = osThreadNew(health_main, &hi2c, &health_attr);
  if (health_handle == NULL) {
    return SYSTEM_MOD_FAIL;
  }
  return SYSTEM_OK;
}
