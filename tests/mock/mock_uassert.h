
#pragma once

#include <stdint.h>
#include "gmock/gmock.h"

class MockUassert {
public:
  MOCK_METHOD(void, assert_handler, (const uint32_t, const uint32_t *, const uint32_t *));
};

// Provide a way to access the mock globally
MockUassert *mock_uassert = nullptr;

// C-style wrapper functions for the mocks
extern "C" {
void assert_handler(const uint32_t line, const uint32_t *pc, const uint32_t *lr) {
  return mock_uassert->assert_handler(line, pc, lr);
}
}