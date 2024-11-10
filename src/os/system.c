
#include "system.h"
#include "stm32h7xx_nucleo.h"
#include "ethernet/app_ethernet.h"
#include "health.h"
#include "logger.h"
#include "scpi/server.h"
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

__attribute__((section(".data.sys"))) static uint32_t tick_counter = 0;
static TaskHandle_t system_boostrap;

static void system_bootstrap_task(void __attribute__((unused)) * argument) {
  system_status_t status = SYSTEM_OK;
  status = app_ethernet_init();
  if (status != SYSTEM_OK) {
    system_spinlock();
  }
  logger_init(LOGGER_TRACE);
  if (status != SYSTEM_OK) {
    system_spinlock();
  }
  status = health_init(hi2c2);
  if (status != SYSTEM_OK) {
    system_spinlock();
  }
  status = scpi_init();
  if (status != SYSTEM_OK) {
    system_spinlock();
  }
  vTaskDelete(system_boostrap);
}

void system_health_indicator(void) {
  tick_counter++;
  if (tick_counter > 1000) {
    tick_counter = 0;
    BSP_LED_Toggle(LED1);
  }
}

void system_spinlock(void) {
  BSP_LED_On(LED3);
  while (1) {
    HAL_Delay(100);
    BSP_LED_Toggle(LED3);
  }
}

void system_boot(void) {
  BaseType_t ret = xTaskCreate(system_bootstrap_task, "bootstrap", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 20, &system_boostrap);
  if (ret != pdPASS) {
    system_spinlock();
  }
  vTaskStartScheduler();
  // clang-format off
  while (1) { }
  // clang-format on
}
