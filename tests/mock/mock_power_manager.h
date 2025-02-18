#pragma once

#include "gmock/gmock.h"

extern "C" {
#include "power_manager.h"
}

class MockPowerManager {
public:
  MOCK_METHOD(void, power_manager_init, (const struct power_manager_init_ctx *));
};

MockPowerManager *mock_power_manager = nullptr;

// C-style wrapper functions for the mocks
extern "C" {

void power_manager_init(const struct power_manager_init_ctx *init_ctx) {
  return mock_power_manager->power_manager_init(init_ctx);
}

}
