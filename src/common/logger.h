/**
 * @file logger.h
 * @brief
 * @version 0.1
 * @date 2024-04
 *
 * @copyright Copyright © 2024 dronectl
 *
 */

#ifndef __LOGGER_H__
#define __LOGGER_H__

#include "system.h"
#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>
#include <stdint.h>

#define LOGGER_DEFAULT_PORT 3000
#define LOGGER_DEFAULT_LEVEL (enum logger_level)1

enum logger_level {
  LOGGER_TRACE,
  LOGGER_INFO,
  LOGGER_WARNING,
  LOGGER_ERROR,
  LOGGER_CRITICAL,
  LOGGER_DISABLE
};

struct logger_init_context {
  const enum logger_level log_level;
  const uint16_t port;
};

struct logger_context {
  const struct logger_init_context *init;
  enum logger_level log_level;
  TaskHandle_t task_handle;
  QueueHandle_t log_queue;
};

enum logger_level logger_get_level(void);
void logger_set_level(const enum logger_level level);
void logger_out(const enum logger_level level, const char *fmt, ...);
void logger_start(const struct system_task_context *task_ctx);

#ifndef critical
#define critical(...) __CRITICAL(__VA_ARGS__, "")
#define __CRITICAL(fmt, ...) logger_out(LOGGER_CRITICAL, fmt, __VA_ARGS__)
#endif

#ifndef error
#define error(...) __ERROR(__VA_ARGS__, "")
#define __ERROR(fmt, ...) logger_out(LOGGER_ERROR, fmt, __VA_ARGS__)
#endif

#ifndef warning
#define warning(...) __WARNING(__VA_ARGS__, "")
#define __WARNING(fmt, ...) logger_out(LOGGER_WARNING, fmt, __VA_ARGS__)
#endif

#ifndef info
#define info(...) __INFO(__VA_ARGS__, "")
#define __INFO(fmt, ...) logger_out(LOGGER_INFO, fmt, __VA_ARGS__)
#endif

#ifndef trace
#define trace(...) __TRACE(__VA_ARGS__, "")
#define __TRACE(fmt, ...) logger_out(LOGGER_TRACE, fmt, __VA_ARGS__)
#endif

#endif // __LOGGER_H__
