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

TEST_F(HsmTest, HsmInitTest) {
  hsm_init(NULL);
}
