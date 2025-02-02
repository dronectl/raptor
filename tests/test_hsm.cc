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

class HsmTestFixture : public ::testing::Test {
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

TEST_F(HsmTestFixture, HsmTaskStart) {
  GPIO_TypeDef mock_port;
  struct hsm_init_context hsm_init_ctx = {
    .led_init_ctx = {
      {
        .port = &mock_port,
        .pin = 1,
        .active_high=true,
      }
    },
    .num_led_init_ctx = 1
  };
  struct system_task_context task_ctx = {
    .name = "name",
    .priority = 13,
    .stack_size = 230,
    .init_ctx = &hsm_init_ctx
  };
  struct hsm_context *ctx = test_hsm_get_context();
  TaskFunction_t hsm_main = test_hsm_get_main();
  QueueHandle_t queue_handle;
  BaseType_t base = pdPASS;

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
  EXPECT_CALL(*mock_uassert, assert_handler(::testing::_, ::testing::_, ::testing::_)).Times(0);
  EXPECT_CALL(*mock_led, led_init(&ctx->led_ctx[0], &hsm_init_ctx.led_init_ctx[0])).Times(1);

  hsm_start(&task_ctx);

  EXPECT_EQ(ctx->hsm_tick_rate_ms, HSM_DEFAULT_TICK_RATE_MS) << "hsm tick rate not initialized to default";
  EXPECT_EQ(ctx->event_queue, queue_handle) << "event queue not initialized";
  EXPECT_EQ(ctx->current_state, HSM_STATE_RESET) << "current state not set to reset state";
  EXPECT_EQ(ctx->next_state, HSM_STATE_RESET) << "next state not set to reset state";
  EXPECT_EQ(ctx->enter_timestamp, 0) << "enter timestamp not reset to 0";
}

TEST_F(HsmTestFixture, HsmEventPostSuccess){
  // event post success
  enum hsm_event event = HSM_EVENT_ABORT;
  EXPECT_CALL(*mock_freertos, xQueueGenericSend(::testing::_, &event, 0, queueSEND_TO_BACK)).Times(1).WillOnce(::testing::Return(pdPASS));
  EXPECT_CALL(*mock_logger, logger_out);
  EXPECT_CALL(*mock_uassert, assert_handler(::testing::_, ::testing::_, ::testing::_)).Times(0);
  EXPECT_EQ(HSM_STATUS_OK, hsm_post_event(&event, 0)) << "hsm event post reported failure";
}

TEST_F(HsmTestFixture, HsmEventPostFail) {
  // queue full with blocking
  enum hsm_event event = HSM_EVENT_RUN;
  EXPECT_CALL(*mock_freertos, xQueueGenericSend(::testing::_, &event, 10, queueSEND_TO_BACK)).Times(1).WillOnce(::testing::Return(errQUEUE_FULL));
  EXPECT_CALL(*mock_logger, logger_out);
  EXPECT_CALL(*mock_uassert, assert_handler(::testing::_, ::testing::_, ::testing::_)).Times(0);
  EXPECT_EQ(HSM_STATUS_EVE_QUEUE_FULL, hsm_post_event(&event, 10)) << "hsm event post reported success";
}

TEST_F(HsmTestFixture, HsmEventPostISRSuccess){
  // event post from isr success
  enum hsm_event event = HSM_EVENT_ABORT;
  bool req_ctx_switch;
  EXPECT_CALL(*mock_freertos, xQueueGenericSendFromISR(::testing::_, &event, ::testing::_, queueSEND_TO_BACK))
    .Times(1)
    .WillOnce(::testing::DoAll(
      ::testing::SetArgPointee<2>(pdTRUE),
      ::testing::Return(pdPASS)
    ));
  EXPECT_CALL(*mock_uassert, assert_handler(::testing::_, ::testing::_, ::testing::_)).Times(0);
  EXPECT_EQ(HSM_STATUS_OK, hsm_post_event_isr(&event, &req_ctx_switch)) << "hsm event post reported failure";
  EXPECT_TRUE(req_ctx_switch);
}

TEST_F(HsmTestFixture, HsmEventPostISRFail) {
  // queue full from isr
  enum hsm_event event = HSM_EVENT_RUN;
  bool req_ctx_switch;
  EXPECT_CALL(*mock_freertos, xQueueGenericSendFromISR(::testing::_, &event, ::testing::_, queueSEND_TO_BACK))
    .Times(1)
    .WillOnce(::testing::DoAll(
      ::testing::SetArgPointee<2>(pdFALSE),
      ::testing::Return(errQUEUE_FULL)
    ));
  EXPECT_CALL(*mock_uassert, assert_handler(::testing::_, ::testing::_, ::testing::_)).Times(0);
  EXPECT_EQ(HSM_STATUS_EVE_QUEUE_FULL, hsm_post_event_isr(&event, &req_ctx_switch)) << "hsm event post did not succeed";
  EXPECT_FALSE(req_ctx_switch);
}

TEST(HsmTest, HSMGetState) {
  struct hsm_context *ctx = test_hsm_get_context();
  ctx->current_state = HSM_STATE_RUN;
  EXPECT_EQ(ctx->current_state, hsm_get_current_state()) << "fetched hsm state does not match current state";
}