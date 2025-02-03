#pragma once

#include "gmock/gmock.h"

extern "C" {
#include "esc_engine.h"
}

class MockEscEngine {
public:
  MOCK_METHOD(void, esc_engine_init, (const struct esc_engine_init_ctx *));
};

MockEscEngine *mock_esc_engine = nullptr;

// C-style wrapper functions for the mocks
extern "C" {

void esc_engine_init(const struct esc_engine_init_ctx *init_ctx) {
  return mock_esc_engine->esc_engine_init(init_ctx);
}

}

