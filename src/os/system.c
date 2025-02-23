
#include "main.h"
#include "system.h"
#include "ethernet/app_ethernet.h"
#include "hsm.h"
#include "logger.h"
#include "uassert.h"

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
  .num_led_init_ctx = 3
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

static struct system_context ctx = {0};

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
  vTaskDelete(ctx.system_boostrap);
}

void system_boot(void) {
  BaseType_t ret = xTaskCreate(system_bootstrap_task, "bootstrap", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 20, &ctx.system_boostrap);
  uassert(ret == pdPASS);
  vTaskStartScheduler();
  uassert(0);
}

enum register_status system_write_uuid(const uint32_t uuid) {
  ctx.uuid = uuid;
  return REGISTER_STATUS_OK;
}

enum register_status system_read_uuid(uint32_t* uuid) {
  enum register_status status = REGISTER_STATUS_OP_ERR;
  if (uuid != NULL) {
    *uuid = ctx.uuid;
    status = REGISTER_STATUS_OK;
  }
  return status;
}

enum register_status system_write_hw_version(const uint32_t hw_version) {
  ctx.hw_version = hw_version;
  return REGISTER_STATUS_OK;
}

enum register_status system_read_hw_version(uint32_t* hw_version) {
  enum register_status status = REGISTER_STATUS_OP_ERR;
  if (hw_version != NULL) {
    *hw_version = ctx.hw_version;
    status = REGISTER_STATUS_OK;
  }
  return status;
}

enum register_status system_write_fw_version(const uint32_t fw_version) {
  ctx.fw_version = fw_version;
  return REGISTER_STATUS_OK;
}

enum register_status system_read_fw_version(uint32_t* fw_version) {
  enum register_status status = REGISTER_STATUS_OP_ERR;
  if (fw_version != NULL) {
    *fw_version = ctx.fw_version;
    status = REGISTER_STATUS_OK;
  }
  return status;
}

enum register_status system_write_fw_commit_sha(const uint64_t fw_commit_sha) {
  ctx.fw_commit_sha = fw_commit_sha;
  return REGISTER_STATUS_OK;
}

enum register_status system_read_fw_commit_sha(uint64_t* fw_commit_sha) {
  enum register_status status = REGISTER_STATUS_OP_ERR;
  if (fw_commit_sha != NULL) {
    *fw_commit_sha = ctx.fw_commit_sha;
    status = REGISTER_STATUS_OK;
  }
  return status;
}
