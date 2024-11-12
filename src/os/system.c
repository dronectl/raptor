
#include "main.h"
#include "system.h"
#include "ethernet/app_ethernet.h"
#include "health.h"
#include "logger.h"
#include "scpi/server.h"
#include "uassert.h"
#include <FreeRTOS.h>
#include <task.h>

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern ADC_HandleTypeDef hadc3;
extern DMA_HandleTypeDef hdma_adc1;
extern DMA_HandleTypeDef hdma_adc2;
extern DMA_HandleTypeDef hdma_adc3;
extern FDCAN_HandleTypeDef hfdcan1;
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
extern SPI_HandleTypeDef hspi2;
extern RTC_HandleTypeDef hrtc;
extern SD_HandleTypeDef hsd1;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim13;
extern UART_HandleTypeDef huart7;
extern UART_HandleTypeDef huart9;
extern UART_HandleTypeDef huart3;

static struct health_ctx health_context = {
    .bme280 = {
        .i2c = &hi2c2,
    },
    .tick_rate_ms = HEALTH_TASK_DEFAULT_TICK_RATE_MS,
    .leds = {
        [HEALTH_STATUS_LED_OK] = {.active_high = true, .port = LED_GREEN_GPIO_Port, .pin = LED_GREEN_Pin},
        [HEALTH_STATUS_LED_ERR] = {.active_high = true, .port = LED_RED_GPIO_Port, .pin = LED_RED_Pin},
        [HEALTH_STATUS_LED_EVENT] = {.active_high = true, .port = LED_YELLOW_GPIO_Port, .pin = LED_YELLOW_Pin},
    }};

static TaskHandle_t system_boostrap;

static void system_bootstrap_task(void __attribute__((unused)) * argument) {
  system_status_t status = SYSTEM_OK;
  status = app_ethernet_init();
  uassert(status == SYSTEM_OK);
  logger_init(LOGGER_TRACE);
  health_init(&health_context);
  vTaskDelete(system_boostrap);
}

void system_boot(void) {
  BaseType_t ret = xTaskCreate(system_bootstrap_task, "bootstrap", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 20, &system_boostrap);
  uassert(ret == pdPASS);
  vTaskStartScheduler();
  // clang-format off
  uassert(0); // should never reach here
  // clang-format on
}
