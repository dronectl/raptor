
#pragma once

#include <stdint.h>
#include "gmock/gmock.h"

class MockLogger {
public:
  MOCK_METHOD(void, logger_init, (const enum logger_level));
  MOCK_METHOD(enum logger_level, logger_get_level, ());
  MOCK_METHOD(void, logger_set_level, (const enum logger_level));
  MOCK_METHOD(void, logger_out, (const enum logger_level level, const char *fmt, va_list args));
};

MockLogger *mock_logger = nullptr;

// C-style wrapper functions for the mocks
extern "C" {
#include "logger.h"

void logger_init(const enum logger_level level) {
  return mock_logger->logger_init(level);
}

enum logger_level logger_get_level(void) {
  return mock_logger->logger_get_level();
}

void logger_set_level(const enum logger_level level) {
  return mock_logger->logger_set_level(level);
}

void logger_out(const enum logger_level level, const char *fmt, ...) {
  va_list args;
  return mock_logger->logger_out(level, fmt, args);
}
}