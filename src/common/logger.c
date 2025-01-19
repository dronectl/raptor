/**
 * @file logger.c
 * @brief
 * @version 0.1
 * @date 2024-04
 *
 * @copyright Copyright © 2024 dronectl
 *
 */

#include "uassert.h"
#include "logger.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <stdarg.h>
#include <string.h>

#define MAX_LOG_HEADER_LEN 21
#define MAX_LOG_OUT_LEN 270
#define MAX_LOG_MESSAGE_LEN (MAX_LOG_OUT_LEN - MAX_LOG_HEADER_LEN)
#define MAX_LOG_BUFFER_SIZE 3
#define LOG_HEADER_FMT "[ %9ld %5s ]\t"

#define min(a, b) a > b ? b : a

/**
 * @brief Log message struct.
 *
 */
struct log_msg {
  uint32_t epoch;
  enum logger_level level;
  char message[MAX_LOG_MESSAGE_LEN];
};

static enum logger_level _level = LOGGER_INFO;
static QueueHandle_t log_queue;
static TaskHandle_t log_task_handle;

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
 * @param[in] log log entry
 * @param[in,out] buffer input write buffer
 * @param[in] size size of buffer
 */
static int write_log(const int client_fd, const struct log_msg *log) {
  char buffer[MAX_LOG_OUT_LEN] = {0};
  int offset = snprintf(buffer, MAX_LOG_HEADER_LEN, LOG_HEADER_FMT, (long)log->epoch, _get_level_str(log->level));
  memcpy(&buffer[offset], log->message, min(strlen(log->message), (size_t)(MAX_LOG_MESSAGE_LEN - offset - 1)));
  return write(client_fd, buffer, strlen(buffer));
}

/**
 * @brief Logging server task. Listen on port 3000 for connections and stream logs out to client.
 *
 */
static void log_server_task(void __attribute__((unused)) * argument) {
  int sock, size, client_fd;
  struct sockaddr_in address, remotehost;
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    goto error;
  }
  address.sin_family = AF_INET;
  address.sin_port = htons(LOGGER_DEFAULT_PORT);
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
      if (xQueueReceive(log_queue, &log, portMAX_DELAY) == pdPASS) {
        if (write_log(client_fd, &log) <= 0) {
          close(client_fd);
          break;
        }
      };
    }
  }
error:
  critical("Socket init failed with %i", errno);
  vTaskDelete(log_task_handle);
}

/**
 * @brief Set the program log level.
 *
 * @param level target logging level
 */
void logger_set_level(const enum logger_level level) {
  _level = level;
}

/**
 * @brief Get the program log level.
 *
 * @return enum Level
 */
enum logger_level logger_get_level(void) {
  return _level;
}

/**
 * @brief Write logging message to buffer.
 *
 * @param level log level enum
 * @param func macro expanded log containing function name
 * @param line macro expanded log line
 * @param fmt log string formatter
 * @param ... variable arguments for string formatter
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
  if (log_queue != NULL) {
    xQueueSend(log_queue, &log, 0);
  }
}

/**
 * @brief Initialize queues semaphores TCPIP logging socket and start logging task.
 *
 * @param level logging level configuration
 */
void logger_init(const enum logger_level level) {
  log_queue = xQueueCreate(MAX_LOG_BUFFER_SIZE, sizeof(struct log_msg));
  uassert(log_queue != NULL);
  logger_set_level(level);
  BaseType_t ret = xTaskCreate(log_server_task, "log_task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &log_task_handle);
  uassert(ret == pdPASS);
}
