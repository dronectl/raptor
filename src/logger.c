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

// program log level (disable by default)
static enum logger_level _level = LOGGER_DISABLE;

static cbuffer_t log_cbuf;

/**
 * @brief Get the string representation of log level enum
 *
 * @param level log level enum
 * @return char*
 */
static char *_get_level_str(enum logger_level level) {
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
  log_cbuf.elem_size = MAX_CBUFFER_SIZE;
  log_cbuf.head = 0;
  log_cbuf.tail = 0;
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
  char buffer[MAX_LOGGING_LINE_LEN];
  va_list args;
  // filter output by log level
  if (logger_get_level() > level) {
    return;
  }
  sprintf(buffer, "[%s]\t %s:%i ", _get_level_str(level), func, line);
  va_start(args, fmt);
  usart_println(buffer);
  vsprintf(buffer, fmt, args);
  va_end(args);
  printf(buffer);
}