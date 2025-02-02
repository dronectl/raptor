
#include "common.h"
#include "main.h"
#include "system.h"
#include "ethernet/app_ethernet.h"
#include "hsm.h"
#include "led.h"
#include "logger.h"
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

static const struct hsm_init_context hsm_init_ctx = {
  .led_init_ctx = {
    [HSM_LED_ID_ERROR] = { .port = LED_RED_GPIO_Port, .pin = LED_RED_Pin, .active_high = true },
    [HSM_LED_ID_IDLE] = { .port = LED_GREEN_GPIO_Port, .pin = LED_GREEN_Pin, .active_high = true },
    [HSM_LED_ID_RUN] = { .port = LED_YELLOW_GPIO_Port, .pin = LED_YELLOW_Pin, .active_high = true }
  },
};

static const struct logger_init_context logger_init_ctx = {
  .log_level = LOGGER_DEFAULT_LEVEL,
  .port = LOGGER_DEFAULT_PORT,
};


// order defines spawn order
static struct system_task system_task_registry[] = {
  // TODO: homogenize app ethernet initialization
  {
    .task_context = {
      .name = "ethif",
      .priority = tskIDLE_PRIORITY,
      .stack_size = configMINIMAL_STACK_SIZE,
      .init_ctx = NULL,
    },
    .start = app_ethernet_init
  },
  {
    .task_context = {
      .name = "logger",
      .priority = tskIDLE_PRIORITY + 1,
      .stack_size = configMINIMAL_STACK_SIZE,
      .init_ctx = &logger_init_ctx,
    },
    .start = logger_start 
  },
  {
    .task_context = {
      .name = "hsm",
      .priority = tskIDLE_PRIORITY + 20,
      .stack_size = configMINIMAL_STACK_SIZE,
      .init_ctx = &hsm_init_ctx,
    },
    .start = hsm_start
  }
};

static TaskHandle_t system_boostrap;

static void system_bootstrap_task(void __attribute__((unused)) * argument) {
  uint8_t task_count = 0;
  struct system_task *task = system_task_registry;
  for (; task < system_task_registry + SYSTEM_MAX_TASKS; task++) {
    if (task->start != NULL) {
      task->start(&task->task_context);
      task_count++;
    }
  }
  info("system boostrap spawned %u tasks", task_count);
  vTaskDelete(system_boostrap);
}

void system_boot(void) {
  BaseType_t ret = xTaskCreate(system_bootstrap_task, "bootstrap", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 20, &system_boostrap);
  uassert(ret == pdPASS);
  vTaskStartScheduler();
  uassert(0);
}
