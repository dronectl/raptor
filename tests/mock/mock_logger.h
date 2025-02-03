
#pragma once

#include "gmock/gmock.h"

extern "C" {
#include "logger.h"
}

class MockLogger {
public:
  MOCK_METHOD(void, logger_start, (const struct system_task_context *));
  MOCK_METHOD(enum logger_level, logger_get_level, ());
  MOCK_METHOD(void, logger_set_level, (const enum logger_level));
  MOCK_METHOD(void, logger_out, (const enum logger_level level, const char *fmt, va_list args));
};

MockLogger *mock_logger = nullptr;

// C-style wrapper functions for the mocks
extern "C" {

void logger_start(const struct system_task_context *task_ctx) {
  return mock_logger->logger_start(task_ctx);
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
