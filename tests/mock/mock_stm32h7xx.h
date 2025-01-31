
#pragma once

#include <stdint.h>
#include "gmock/gmock.h"

class MockSTM32H7HAL {
public:
  MOCK_METHOD(uint32_t, HAL_GetTick, ());
};

// Provide a way to access the mock globally
MockSTM32H7HAL *mock_stm32_hal = nullptr;

// C-style wrapper functions for the mocks
extern "C" {
uint32_t HAL_GetTick(void) {
  return mock_stm32_hal->HAL_GetTick();
}
}