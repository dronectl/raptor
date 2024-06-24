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
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include <cmsis_os2.h>
#include <stdarg.h>
#include <string.h>

#define MAX_LOGGING_LINE_LEN 300
#define MAX_LOGGING_CBUFFER_SIZE 10
#define LOG_HEADER_FMT "%ld::%i::%s - "

/**
 * @brief Log message struct.
 *
 */
struct log_msg {
  uint32_t epoch;                     // current CPU epoch
  enum logger_level level;            // log level
  int line;                           // line number
  char message[MAX_LOGGING_LINE_LEN]; // post variable args injection
};

const osThreadAttr_t logger_attr = {
    .name = "logger_task",
    .priority = osPriorityLow,
};

static osMessageQueueAttr_t attrs = {
    .name = "log_queue",
};

static osMessageQueueId_t queue_id;
static osThreadId_t logger_handle;
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
 * @param[in] log log entry
 * @param[in,out] buffer input write buffer
 * @param[in] size size of buffer
 */
static void build_log_string(const struct log_msg *log, char *buffer, const size_t size) {
  const char *level_str = _get_level_str(log->level);
  int offset = snprintf(buffer, size, LOG_HEADER_FMT, (long)log->epoch, log->line, level_str);
  int len = strlen(log->message);
  // overflow check and clamp len
  if (len + offset > (int)size) {
    len = size - offset - 1; // leave room for null terminator
  }
  // intentional overwrite of previous null character
  memcpy(buffer + offset, log->message, len);
  buffer[len + offset + 1] = '\0';
}

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
      return "WARNING";
    case LOGGER_ERROR:
      return "ERROR";
    default:
      return "CRITICAL";
  }
}

static __NO_RETURN void logger_task(void __attribute__((unused)) * argument) {
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
    // task blocking
    client_fd = accept(sock, (struct sockaddr *)&remotehost, (socklen_t *)&size);
    info("Accepted connection with fd: %d\n", client_fd);
    if (client_fd < 0) {
      osThreadYield();
      continue;
    }
    while (1) {
      osMessageQueueGet(queue_id, &log, NULL, osWaitForever);
      char log_buffer[MAX_LOGGING_LINE_LEN] = {0};
      build_log_string(&log, log_buffer, MAX_LOGGING_LINE_LEN);
      ssize_t bytes_sent = write(client_fd, log_buffer, strlen(log_buffer));
      /* Check for errors or client disconnect */
      if (bytes_sent <= 0) {
        /* Connection closed by client */
        close(client_fd);
        break;
      }
    }
  }
error:
  critical("Socket init failed with %i", errno);
  osThreadExit();
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
void logger_out(const enum logger_level level, const int line, const char *fmt, ...) {
  struct log_msg log = {0};
  va_list args;
  // filter output by log level
  if (logger_get_level() > level) {
    return;
  }
  log.epoch = osKernelGetTickCount();
  log.level = level;
  log.line = line;
  // variable arguments must be processed here otherwise they will go out of scope
  va_start(args, fmt);
  vsnprintf(log.message, MAX_LOGGING_LINE_LEN, fmt, args);
  va_end(args);
  osMessageQueuePut(queue_id, (void *)&log, 0, 0);
}

/**
 * @brief Initialize queues semaphores TCPIP logging socket and start logging task.
 *
 * @param level logging level configuration
 */
system_status_t logger_init(const enum logger_level level) {
  /* Create the queue, storing the returned handle in the xQueue variable. */
  queue_id = osMessageQueueNew(MAX_LOGGING_CBUFFER_SIZE, sizeof(struct log_msg), &attrs);
  if (queue_id == NULL) {
    return SYSTEM_MOD_FAIL;
  }
  logger_set_level(level);
  logger_handle = osThreadNew(logger_task, NULL, &logger_attr);
  if (logger_handle == NULL) {
    return SYSTEM_MOD_FAIL;
  }
  return SYSTEM_OK;
}
