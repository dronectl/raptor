/**
 * @file logger.c
 * @brief TCP/IP log server. This implementation is built on CMSIS OS v2 and is fully thread safe.
 * @version 0.1
 * @date 2023-01
 *
 * @copyright Copyright © 2023 dronectl
 *
 */

#include "logger.h"
#include "cbuffer.h"
#include "cmsis_os2.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include <stdarg.h>
#include <string.h>

#define FUNC_NAME_MAX_LEN 20

/**
 * @brief Log message struct.
 *
 */
typedef struct log_t {
  uint32_t epoch;                     // current CPU epoch as reported by RTOS
  enum logger_level level;            // log level
  int line;                           // line number
  char func[FUNC_NAME_MAX_LEN];       // function name
  char message[MAX_LOGGING_LINE_LEN]; // post variable args injection
} log_t;

static int sock, size;
struct sockaddr_in address, remotehost;
osMutexId_t logger_mutex_id;
const osMutexAttr_t logger_mutex_attr = {
  .name = "logger_mutex",
  .cb_mem = NULL,
  .cb_size = 0U,
  .attr_bits = osMutexRecursive,
};
osMessageQueueId_t logger_queue_id;
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
 * @brief Initialize queues mutexes and TCPIP logging socket
 *
 * @param level logging level configuration
 */
void logger_init(const enum logger_level level) {
  logger_mutex_id = osMutexNew(&logger_mutex_attr);
  if (logger_mutex_id == NULL) {
    return;
  }
  logger_queue_id = osMessageQueueNew(MAX_LOGGING_CBUFFER_SIZE, sizeof(log_t), NULL);
  if (logger_queue_id == NULL) {
    return;
  }
  logger_set_level(level);
}

/**
 * @brief Set the program log level. Blocks indefinitely until resource is
 * available.
 *
 * @param level target logging level
 */
void logger_set_level(enum logger_level level) {
  osStatus_t status = osMutexAcquire(logger_mutex_id, 10);
  if (status == osOK) {
    _level = level;
  }
  osMutexRelease(logger_mutex_id);
}

/**
 * @brief Get the program log level. Blocks indefintely until resource is
 * available.
 *
 * @return enum Level
 */
enum logger_level logger_get_level(void) {
  enum logger_level level = LOGGER_TRACE;
  osStatus_t status = osMutexAcquire(logger_mutex_id, 10);
  if (status == osOK) {
    level = _level;
  }
  osMutexRelease(logger_mutex_id);
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
  if (logger_get_level() > level) {
    return;
  }
  log.epoch = osKernelGetTickCount();
  log.level = level;
  log.line = line;
  memcpy(log.func, func, FUNC_NAME_MAX_LEN);
  va_start(args, fmt);
  vsnprintf(log.message, MAX_LOGGING_LINE_LEN, fmt, args);
  va_end(args);
  osMessageQueuePut(logger_queue_id, &log, 0, 0);
}

__NO_RETURN void logger_main(void *argument) {
  log_t log;
  int client_fd;
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    __BKPT(0);
  }
  address.sin_family = AF_INET;
  address.sin_port = htons(LOGGING_PORT);
  address.sin_addr.s_addr = INADDR_ANY;
  if (bind(sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
    __BKPT(0);
  }
  listen(sock, 5);
  info("Initialized logger");
  while (1) {
    client_fd = accept(sock, (struct sockaddr *)&remotehost, (socklen_t *)&size);
    while (1) {
      osMessageQueueGet(logger_queue_id, &log, NULL, 0);
      if (client_fd < 0) {
        continue;
      }
      char log_buffer[MAX_LOGGING_LINE_LEN] = {0};
      build_log_string(&log, log_buffer, MAX_LOGGING_LINE_LEN);
      ssize_t bytes_sent = send(client_fd, log_buffer, strlen(log_buffer), 0);
      if (bytes_sent <= 0) {
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
