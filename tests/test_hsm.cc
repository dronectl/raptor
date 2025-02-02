/**
 * @file test_hsm.cc
 * @author ztnel (christian911@sympatico.ca)
 * @brief HSM unittests
 * @version 0.1
 * @date 2025-01
 *
 * @copyright Copyright © 2025 Christian Sargusingh
 *
 */

#include <gtest/gtest.h>

#include "mock_logger.h"
#include "mock_dtc.h"
#include "mock_led.h"
#include "mock_uassert.h"
#include "mock_stm32h7xx.h"
#include "mock_freertos.h"
#include "mock_esc_engine.h"
#include "mock_power_manager.h"

extern "C" {
#include <stm32h7xx_hal.h>
#include "hsm.h"
}

class HsmTest : public ::testing::Test {
protected:
  MockLogger m_logger;
  MockUassert m_uassert;
  MockDTC m_dtc;
  MockLED m_led;
  MockSTM32H7HAL m_stm32_hal;
  MockFreeRTOS m_freertos;
  MockEscEngine m_esc_engine;
  MockPowerManager m_power_manager;

  void SetUp() override {
    mock_logger = &m_logger;
    mock_uassert = &m_uassert;
    mock_led = &m_led;
    mock_stm32_hal = &m_stm32_hal;
    mock_freertos = &m_freertos;
    mock_dtc = &m_dtc;
    mock_esc_engine = &m_esc_engine;
    mock_power_manager = &m_power_manager;
  }

  void TearDown() override {
    mock_logger = nullptr;
    mock_uassert = nullptr;
    mock_led = nullptr;
    mock_stm32_hal = nullptr;
    mock_freertos = nullptr;
    mock_dtc = nullptr;
    mock_esc_engine = nullptr;
    mock_power_manager = nullptr;
  }
};

TEST_F(HsmTest, HsmTaskStart) {
  struct system_task_context task_ctx = {
    .name = "name",
    .priority = 13,
    .stack_size = 230,
  };
  struct hsm_context *ctx = test_hsm_get_context();
  TaskFunction_t hsm_main = test_hsm_get_main();
  QueueHandle_t queue_handle;
  BaseType_t base;

  EXPECT_CALL(
    *mock_freertos,
    xQueueGenericCreateStatic(sizeof(ctx->event_queue_buffer), sizeof(enum hsm_event), ctx->event_queue_buffer, &ctx->event_queue_ctrl, queueQUEUE_TYPE_BASE))
    .Times(1)
    .WillOnce(::testing::Return(queue_handle));
  EXPECT_CALL(
    *mock_freertos,
    xTaskCreate(hsm_main, task_ctx.name, task_ctx.stack_size, NULL, task_ctx.priority, &ctx->task_handle))
    .Times(1)
    .WillOnce(::testing::Return(base));

  hsm_start(&task_ctx);

  EXPECT_EQ(ctx->hsm_tick_rate_ms, HSM_DEFAULT_TICK_RATE_MS) << "hsm tick rate not initialized to default";
  EXPECT_EQ(ctx->event_queue, queue_handle) << "event queue not initialized";
  EXPECT_EQ(ctx->current_state, HSM_STATE_RESET) << "current state not set to reset state";
  EXPECT_EQ(ctx->next_state, HSM_STATE_RESET) << "next state not set to reset state";
  EXPECT_EQ(ctx->enter_timestamp, 0) << "enter timestamp not reset to 0";
}
