
#include "main.h"
#include "system.h"
#include "ethernet/app_ethernet.h"
#include "hsm.h"
#include "led.h"
#include "logger.h"
#include "uassert.h"
#include "grpc.h"

#include <FreeRTOS.h>
#include <task.h>
#include <stdbool.h>

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

// Hardware driver module resources
static const struct led_init_ctx led_error_init = {
    .port = LED_RED_GPIO_Port,
    .pin = LED_RED_Pin,
    .active_high = true,
};

static const struct led_init_ctx led_idle_init = {
    .port = LED_GREEN_GPIO_Port,
    .pin = LED_GREEN_Pin,
    .active_high = true,
};

static const struct led_init_ctx led_run_init = {
    .port = LED_YELLOW_GPIO_Port,
    .pin = LED_YELLOW_Pin,
    .active_high = true,
};

static struct led_ctx led_error_ctx = {
    .init_ctx = &led_error_init,
    .last_toggle_ms = 0,
};

static struct led_ctx led_run_ctx = {
    .init_ctx = &led_run_init,
    .last_toggle_ms = 0,
};

static struct led_ctx led_idle_ctx = {
    .init_ctx = &led_idle_init,
    .last_toggle_ms = 0,
};

// Module contexts

const struct hsm_init_params hsm_init_params = {
    .led_ctxs = {
        [HSM_LED_ID_ERROR] = &led_error_ctx,
        [HSM_LED_ID_IDLE] = &led_idle_ctx,
        [HSM_LED_ID_RUN] = &led_run_ctx,
    },
};

static TaskHandle_t system_boostrap;

static void system_bootstrap_task(void __attribute__((unused)) * argument) {
  app_ethernet_init();
  logger_init(LOGGER_TRACE);
  hsm_init(&hsm_init_params);
  grpc_init();
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
