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
#include "FreeRTOS.h"
#include "cbuffer.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "queue.h"
#include "semphr.h"
#include <stdarg.h>
#include <string.h>

#define FUNC_NAME_MAX_LEN 20
#define QUEUE_ITEM_SIZE sizeof(log_t)

/**
 * @brief Log message struct.
 *
 */
typedef struct log_t {
  TickType_t epoch;                   // current CPU epoch
  enum logger_level level;            // log level
  int line;                           // line number
  char func[FUNC_NAME_MAX_LEN];       // function name
  char message[MAX_LOGGING_LINE_LEN]; // post variable args injection
} log_t;

static int sock, size;
struct sockaddr_in address, remotehost;
static SemaphoreHandle_t mutex_handle;
static QueueHandle_t queue_handle;
static enum logger_level _level = LOGGER_DISABLE;

/**
 * @brief Get the string representation of log level enum
 *
 * @param level log level enum
 * @return char*
 */
static const char *_get_level_str(enum logger_level level);

/**
 * @brief Construct log string message from log struct. This method builds the log header and
 * combines the formatted log message with the header.
 *
 */
static void build_log_string(log_t *log, char *buffer, size_t size);

/**
 * @brief Initialize queues semaphores and TCPIP logging socket
 *
 * @param level logging level configuration
 */
void logger_init(const enum logger_level level) {
  /* Create a mutex type semaphore. */
  mutex_handle = xSemaphoreCreateMutex();
  if (mutex_handle == NULL) {
    return;
  }
  /* Create the queue, storing the returned handle in the xQueue variable. */
  queue_handle = xQueueCreate(MAX_LOGGING_CBUFFER_SIZE, QUEUE_ITEM_SIZE);
  if (queue_handle == NULL) {
    return;
  }
  logger_set_level(level);
  /* create a TCP socket */
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    return;
  }
  address.sin_family = AF_INET;
  address.sin_port = htons(LOGGING_PORT);
  address.sin_addr.s_addr = INADDR_ANY;
  if (bind(sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
    return;
  }
  listen(sock, 5);
}

/**
 * @brief Set the program log level. Blocks indefinitely until resource is
 * available.
 *
 * @param level target logging level
 */
void logger_set_level(enum logger_level level) {
  if (xSemaphoreTake(mutex_handle, portMAX_DELAY) == pdTRUE) {
    _level = level;
    xSemaphoreGive(mutex_handle);
  }
}

/**
 * @brief Get the program log level. Blocks indefintely until resource is
 * available.
 *
 * @return enum Level
 */
enum logger_level logger_get_level(void) {
  enum logger_level level = LOGGER_TRACE;
  if (xSemaphoreTake(mutex_handle, portMAX_DELAY) == pdTRUE) {
    level = _level;
    xSemaphoreGive(mutex_handle);
  }
  return level;
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
void logger_out(const enum logger_level level, const char *func, const int line, const char *fmt, ...) {
  log_t log = {0};
  va_list args;
  // filter output by log level
  if (logger_get_level() > level) {
    return;
  }
  log.epoch = xTaskGetTickCount();
  log.level = level;
  log.line = line;
  memcpy(log.func, func, FUNC_NAME_MAX_LEN);
  // variable arguments must be processed here otherwise they will go out of scope
  va_start(args, fmt);
  vsnprintf(log.message, MAX_LOGGING_LINE_LEN, fmt, args);
  va_end(args);
  // write log struct by reference to queue, block for 20 ticks if queue is full
  xQueueSendToBack(queue_handle, (void *)&log, 20);
}

void logger_task(void *pv_params) {
  log_t log;
  int client_fd;
  while (1) {
    client_fd = accept(sock, (struct sockaddr *)&remotehost, (socklen_t *)&size);
    while (1) {
      xQueueReceive(queue_handle, &log, portMAX_DELAY);
      if (client_fd < 0) {
        continue;
      }
      char log_buffer[MAX_LOGGING_LINE_LEN] = {0};
      build_log_string(&log, log_buffer, MAX_LOGGING_LINE_LEN);
      ssize_t bytes_sent = send(client_fd, log_buffer, strlen(log_buffer), 0);
      /* Check for errors or client disconnect */
      if (bytes_sent <= 0) {
        /* Connection closed by client */
        close(client_fd);
        break;
      }
    }
  }
}

static const char *_get_level_str(enum logger_level level) {
  switch (level) {
    case LOGGER_TRACE:
      return "TRACE";
    case LOGGER_INFO:
      return "INFO";
    case LOGGER_WARNING:
      return "WARNING";
    case LOGGER_ERROR:
      return "ERROR";
    default:
      return "CRITICAL";
  }
}

static void build_log_string(log_t *log, char *buffer, size_t size) {
  const char *level_str = _get_level_str(log->level);
  int offset = snprintf(buffer, size, "[ %9ld ] %s:%i [ %5s ] \t", (long)log->epoch, log->func, log->line, level_str);
  int len = strlen(log->message);
  // overflow check and clamp len
  if (len + offset > (int)size) {
    len = size - offset - 1; // leave room for null terminator
  }
  // intentional overwrite of previous null character
  memcpy(buffer + offset, log->message, len);
  buffer[len + offset] = '\n';
  buffer[len + offset + 1] = '\0';
}
