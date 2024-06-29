/**
 * @file logger.c
 * @brief
 * @version 0.1
 * @date 2024-04
 *
 * @copyright Copyright © 2024 dronectl
 *
 */

#include "logger.h"
#include "FreeRTOSConfig.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <stdarg.h>
#include <string.h>

#define MAX_LOGGING_LINE_LEN 300
#define MAX_LOG_BUFFER_SIZE 10
#define LOG_HEADER_FMT "[ %9ld %5s ]\t"

/**
 * @brief Log message struct.
 *
 */
struct log_msg {
  uint32_t epoch;                     // current CPU epoch
  enum logger_level level;            // log level
  char message[MAX_LOGGING_LINE_LEN]; // post variable args injection
};

// auxiliary ram for buffered logs
__attribute__((section(".ram_d4"))) static uint8_t log_queue_stack[MAX_LOG_BUFFER_SIZE * sizeof(struct log_msg)];

static enum logger_level _level = LOGGER_DISABLE;
static StaticQueue_t log_static_queue;
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
static void build_log_string(const struct log_msg *log, char *buffer, const size_t size) {
  const char *level_str = _get_level_str(log->level);
  int offset = snprintf(buffer, size, LOG_HEADER_FMT, (long)log->epoch, level_str);
  int len = strlen(log->message);
  // overflow check and clamp len
  if (len + offset > (int)size) {
    len = size - offset - 1; // leave room for null terminator
  }
  // intentional overwrite of previous null character
  memcpy(buffer + offset, log->message, len);
}

/**
 * @brief Logging server task. Listen on port 3000 for connections and stream logs out to client.
 *
 */
static void log_server_task(void __attribute__((unused)) * argument) {
  int sock, size, client_fd;
  struct sockaddr_in address, remotehost;
  struct log_msg log;
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
    char log_buffer[MAX_LOGGING_LINE_LEN];
    while (1) {
      memset(log_buffer, '\0', sizeof(log_buffer));
      memset(&log, 0, sizeof(log));
      xQueueReceive(log_queue, &log, portMAX_DELAY);
      build_log_string(&log, log_buffer, MAX_LOGGING_LINE_LEN);
      ssize_t bytes_sent = write(client_fd, log_buffer, strlen(log_buffer));
      if (bytes_sent <= 0) {
        close(client_fd);
        break;
      }
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
  struct log_msg log = {0};
  va_list args;
  // filter output by log level
  if (logger_get_level() > level) {
    return;
  }
  log.epoch = xTaskGetTickCount();
  log.level = level;
  // variable arguments must be processed here otherwise they will go out of scope
  va_start(args, fmt);
  vsnprintf(log.message, MAX_LOGGING_LINE_LEN, fmt, args);
  va_end(args);
  xQueueSendToBack(log_queue, (void *)&log, 20);
}

/**
 * @brief Initialize queues semaphores TCPIP logging socket and start logging task.
 *
 * @param level logging level configuration
 */
system_status_t logger_init(const enum logger_level level) {
  /* Create the queue, storing the returned handle in the xQueue variable. */
  log_queue = xQueueCreateStatic(MAX_LOG_BUFFER_SIZE, sizeof(struct log_msg), log_queue_stack, &log_static_queue);
  if (log_queue == NULL) {
    return SYSTEM_MOD_FAIL;
  }
  logger_set_level(level);
  BaseType_t ret = xTaskCreate(log_server_task, "log_task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &log_task_handle);
  if (ret != pdPASS) {
    return SYSTEM_MOD_FAIL;
  }
  return SYSTEM_OK;
}
