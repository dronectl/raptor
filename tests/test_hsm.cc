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
#include "mock_led.h"
#include "mock_uassert.h"
#include "mock_stm32h7xx.h"

extern "C" {
#include <stm32h7xx_hal.h>
#include "hsm.h"
}

class HsmTest : public ::testing::Test {
protected:
  MockUassert m_uassert;
  MockLED m_led;

  void SetUp() override {
    mock_uassert = &m_uassert;
    mock_led = &m_led;
  }

  void TearDown() override {
    mock_uassert = nullptr;
    mock_led = nullptr;
  }
};

TEST_F(HsmTest, HsmInitTest) {
  hsm_init(NULL);
}
