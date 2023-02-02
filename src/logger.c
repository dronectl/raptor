/**
 * @file logger.c
 * @brief
 * @version 0.1
 * @date 2023-01
 *
 * @copyright Copyright © 2023 dronectl
 *
 */
#include "logger.h"
#include "cbuffer.h"
#include <stdarg.h>
#include <string.h>

// program log level (disable by default)
static enum logger_level _level = LOGGER_DISABLE;

static char buffer[MAX_LOGGING_CBUFFER_SIZE][MAX_LOGGING_LINE_LEN];
static cbuffer_t cb;

/**
 * @brief Get the string representation of log level enum
 *
 * @param level log level enum
 * @return char*
 */
static const char *_get_level_str(enum logger_level level) {
  switch (level) {
    case LOGGER_TRACE:
      return "TRACE";
    case LOGGER_INFO:
      return "INFO";
    case LOGGER_WARNING:
      return "WARN";
    case LOGGER_ERROR:
      return "ERR";
    default:
      return "CRIT";
  }
}

void logger_init(void) {
  // initialize cbuffer
  cbuffer_init(&cb, buffer, MAX_LOGGING_LINE_LEN, MAX_LOGGING_CBUFFER_SIZE);
  logger_set_level((enum logger_level)LOGGING_LEVEL);
}

/**
 * @brief Set the program log level
 *
 * @param level target logging level
 */
void logger_set_level(enum logger_level level) { _level = level; }

/**
 * @brief Get the program log level
 *
 * @return enum Level
 */
enum logger_level logger_get_level(void) { return _level; }

/**
 * @brief Perform log filtration and standard log construction parsing variable
 * arguments
 *
 * @param level log level enum
 * @param func macro expanded log containing function name
 * @param line macro expanded log line
 * @param fmt log string formatter
 * @param ... variable arguments for string formatter
 */
void logger_out(const enum logger_level level, const char *func, const int line,
                const char *fmt, ...) {
  int inc;
  va_list args;
  char buffer[MAX_LOGGING_LINE_LEN];
  // filter output by log level
  if (logger_get_level() > level) {
    return;
  }
  const char *level_str = _get_level_str(level);
  inc = snprintf(buffer, MAX_LOGGING_LINE_LEN, "[ %5s ] %s:%i\t", level_str,
                 func, line);
  va_start(args, fmt);
  // write from header index
  // TODO: implement buffer overflow handler (add truncating)
  vsnprintf(buffer + inc, MAX_LOGGING_LINE_LEN - inc, fmt, args);
  va_end(args);
  // write to buffer
  cbuffer_write(&cb, buffer);
}

void logger_flush(void) {
  char *next;
  while ((next = (char *)cbuffer_get(&cb)) != NULL) {
    printf("%s\n", next);
  }
}