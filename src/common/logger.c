/**
 * @file logger.c
 * @brief
 * @version 0.1
 * @date 2024-04
 *
 * @copyright Copyright © 2024 dronectl
 *
 */

#include "common.h"
#include "uassert.h"
#include "logger.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"

#include <stdarg.h>
#include <string.h>

#define MAX_LOG_HEADER_LEN 21
#define MAX_LOG_OUT_LEN 270
#define MAX_LOG_MESSAGE_LEN (MAX_LOG_OUT_LEN - MAX_LOG_HEADER_LEN)
#define MAX_LOG_BUFFER_SIZE 3
#define LOG_HEADER_FMT "[ %9ld %5s ]\t"


/**
 * @brief Log message struct.
 *
 */
struct log_msg {
  uint32_t epoch;
  enum logger_level level;
  char message[MAX_LOG_MESSAGE_LEN];
};

static struct logger_context ctx = {0};

/**
 * @brief Get logger level string from enum
 *
 * @param[in] level to translate
 * @return string representation of logger level
 */
static const char *_get_level_str(const enum logger_level level) {
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

/**
 * @brief Construct log string message from log struct. This method builds the log header and
 * combines the formatted log message with the header.
 *
 * @param[in] client_fd client socket descriptor
 * @param[in] log log entry
 */
static int write_log(const int client_sd, const struct log_msg *log) {
  char buffer[MAX_LOG_OUT_LEN] = {0};
  int offset = snprintf(buffer, MAX_LOG_HEADER_LEN, LOG_HEADER_FMT, (long)log->epoch, _get_level_str(log->level));
  memcpy(&buffer[offset], log->message, min(strlen(log->message), (size_t)(MAX_LOG_MESSAGE_LEN - offset - 1)));
  return write(client_sd, buffer, strlen(buffer));
}

/**
 * @brief Logging server task runner
 *
 * @param[in] argument task argument (unused)
 */
static void log_server_task(void* __attribute__((unused)) argument) {
  int sock, size, client_fd;
  struct sockaddr_in address, remotehost;
  uassert(ctx.init != NULL);
  logger_set_level(ctx.init->log_level);
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    goto error;
  }
  address.sin_family = AF_INET;
  address.sin_port = htons(ctx.init->port);
  address.sin_addr.s_addr = INADDR_ANY;
  if (bind(sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
    goto error;
  }
  listen(sock, 5);
  while (1) {
    client_fd = accept(sock, (struct sockaddr *)&remotehost, (socklen_t *)&size);
    if (client_fd < 0) {
      taskYIELD();
      continue;
    }
    while (1) {
      struct log_msg log = {0};
      if (xQueueReceive(ctx.log_queue, &log, portMAX_DELAY) == pdPASS) {
        if (write_log(client_fd, &log) <= 0) {
          close(client_fd);
          break;
        }
      };
    }
  }
error:
  critical("Socket init failed with %i", errno);
  vTaskDelete(ctx.task_handle);
}

/**
 * @brief Set the program log level.
 *
 * @param[in] level target logging level
 */
void logger_set_level(const enum logger_level level) {
  ctx.log_level = level;
}

/**
 * @brief Get the program log level.
 *
 * @return enum Level
 */
enum logger_level logger_get_level(void) {
  return ctx.log_level;
}

/**
 * @brief Write logging message to buffer.
 *
 * @param[in] level log level enum
 * @param[in] fmt log string formatter
 * @param[in] ... variable arguments for string formatter
 */
void logger_out(const enum logger_level level, const char *fmt, ...) {
  va_list args;
  struct log_msg log = {0};
  if (logger_get_level() > level) {
    return;
  }
  log.epoch = xTaskGetTickCount();
  log.level = level;
  va_start(args, fmt);
  vsnprintf(log.message, MAX_LOG_MESSAGE_LEN - 1, fmt, args);
  va_end(args);
  if (ctx.log_queue != NULL) {
    xQueueSend(ctx.log_queue, &log, 0);
  }
}

void logger_start(const struct system_task_context *task_ctx) {
  // header guards
  uassert(task_ctx != NULL);
  uassert(task_ctx->init_ctx != NULL);

  // populate context
  ctx.init = task_ctx->init_ctx;
  ctx.log_queue = xQueueCreate(MAX_LOG_BUFFER_SIZE, sizeof(struct log_msg));
  uassert(ctx.log_queue != NULL);
  
  // start logger task
  BaseType_t ret = xTaskCreate(log_server_task, task_ctx->name, task_ctx->stack_size, NULL, task_ctx->priority, &ctx.task_handle);
  uassert(ret == pdPASS);
}
