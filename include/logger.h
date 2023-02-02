/**
 * @file logger.h
 * @brief
 * @version 0.1
 * @date 2023-01
 *
 * @copyright Copyright © 2023 dronectl
 *
 */

#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <stdio.h>

#ifndef MAX_LOGGING_LINE_LEN
#define MAX_LOGGING_LINE_LEN 100
#endif
#ifndef MAX_LOGGING_CBUFFER_SIZE
#define MAX_LOGGING_CBUFFER_SIZE 10
#endif

#ifndef LOGGING_LEVEL
#define LOGGING_LEVEL 0
#endif

// logging levels
enum logger_level {
  LOGGER_TRACE,
  LOGGER_INFO,
  LOGGER_WARNING,
  LOGGER_ERROR,
  LOGGER_CRITICAL,
  LOGGER_DISABLE
};

void logger_init(void);
enum logger_level logger_get_level(void);
void logger_set_level(enum logger_level level);
void logger_out(enum logger_level level, const char *func, int line,
                const char *fmt, ...);
void logger_flush(void);

#ifndef critical
#define critical(...) __CRITICAL(__VA_ARGS__, "")
#define __CRITICAL(fmt, ...)                                                   \
  logger_out(LOGGER_CRITICAL, __func__, __LINE__, fmt, __VA_ARGS__)
#endif

#ifndef error
#define error(...) __ERROR(__VA_ARGS__, "")
#define __ERROR(fmt, ...)                                                      \
  logger_out(LOGGER_ERROR, __func__, __LINE__, fmt, __VA_ARGS__)
#endif

#ifndef warning
#define warning(...) __WARNING(__VA_ARGS__, "")
#define __WARNING(fmt, ...)                                                    \
  logger_out(LOGGER_WARNING, __func__, __LINE__, fmt, __VA_ARGS__)
#endif

#ifndef info
#define info(...) __INFO(__VA_ARGS__, "")
#define __INFO(fmt, ...)                                                       \
  logger_out(LOGGER_INFO, __func__, __LINE__, fmt, __VA_ARGS__)
#endif

#ifndef trace
#define trace(...) __TRACE(__VA_ARGS__, "")
#define __TRACE(fmt, ...)                                                      \
  logger_out(LOGGER_TRACE, __func__, __LINE__, fmt, __VA_ARGS__)
#endif

#endif // __LOGGER_H__
