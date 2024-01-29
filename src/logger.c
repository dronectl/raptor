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
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "queue.h"
#include "semphr.h"
#include <stdarg.h>
#include <string.h>
#include "main.h"
#include "cbuffer.h"

#define FUNC_NAME_MAX_LEN 20

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

static int is_initialized = 0;
static SemaphoreHandle_t mutex_handle;
static QueueHandle_t queue_handle;
// program log level (disable by default)
static enum logger_level _level = LOGGER_DISABLE;

#define QUEUE_ITEM_SIZE sizeof(log_t)

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
      return "WARNING";
    case LOGGER_ERROR:
      return "ERROR";
    default:
      return "CRITICAL";
  }
}

/**
 * @brief Initialize the IPC logging queue, and UDP server/client backend.
 *
 */
static void logger_init(void) {

  /* Create a mutex type semaphore. */
  mutex_handle = xSemaphoreCreateMutex();
  if (mutex_handle == NULL) {
    /* TODO: handle failure */
    return;
  }
  /* Create the queue, storing the returned handle in the xQueue variable. */
  queue_handle = xQueueCreate(MAX_LOGGING_CBUFFER_SIZE, QUEUE_ITEM_SIZE);
  if (queue_handle == NULL) {
    /* TODO: The queue could not be created. */
    return;
  }
  logger_set_level((enum logger_level)LOGGING_LEVEL);
#ifdef RAPTOR_DEBUG
  /* create a TCP socket */
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    return;
  }
  /* bind to port 80 at any interface */
  address.sin_family = AF_INET;
  address.sin_port = htons(LOGGING_PORT);
  address.sin_addr.s_addr = INADDR_ANY;
  if (bind(sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
    return;
  }
  /* listen for incoming connections (TCP listen backlog = 5) */
  listen(sock, 5);
#else
  // setup logging client to log aggregator server.
#endif // RAPTOR_DEBUG
  is_initialized = 1;
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
  enum logger_level level;
  if (xSemaphoreTake(mutex_handle, portMAX_DELAY) == pdTRUE) {
    level = _level;
    xSemaphoreGive(mutex_handle);
  }
  return level;
}

/**
 * @brief Construct log string message from log struct. This method builds the log header and
 * combines the formatted log message with the header.
 *
 */
static void build_log_string(log_t *log) {
  int inc;
  char buffer[MAX_LOGGING_LINE_LEN];
  // TODO: implement overflow checks
  // copy log message to temp buffer
  size_t len = strlen(log->message);
  memcpy(buffer, log->message, len);
  // add newline and string terminator
  buffer[len] = '\n';
  buffer[len + 1] = '\0';
  const char *level_str = _get_level_str(log->level);
  // write the logging header message
  inc = snprintf(log->message, MAX_LOGGING_LINE_LEN, "[ %5s ] %s:%i\t", level_str, log->func,
                 log->line);
  // finally append the logging message up to the maximum line length
  memcpy(log->message + (uint8_t)inc, buffer, MAX_LOGGING_LINE_LEN - inc);
}

/**
 * @brief 
 * Heartbeat test logger
 * 
 */
void heartbeat_logger(){

  
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
void logger_out(const enum logger_level level, const char *func, const int line, const char *fmt,
                ...) {
  log_t log;
  va_list args;
  if (!is_initialized) {
    return;
  }
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
  // TODO: implement buffer overflow handler (add truncating)
  vsnprintf(log.message, MAX_LOGGING_LINE_LEN, fmt, args);
  va_end(args);
  // write log struct by reference to queue, block for 20 ticks if queue is full
  xQueueSendToBack(queue_handle, (void *)&log, 20);
}

void logger_task(void *pv_params) {
  log_t log;
  int client_fd;

  logger_init();

  while (1) {
    client_fd = accept(sock, (struct sockaddr *)&remotehost, (socklen_t *)&size);
    /* NOTE: If INCLUDE_vTaskSuspend is set to '1' then specifying the block
     * time as portMAX_DELAY will cause the task to block indefinitely (without
     * a timeout).*/
    while (1) {
      xQueueReceive(queue_handle, &log, portMAX_DELAY);
      if (client_fd < 0) {
        // Error in accepting connection
        continue;
      }
      build_log_string(&log);
      ssize_t bytes_sent = send(client_fd, &log.message, strlen(log.message), 0);
      /* Check for errors or client disconnect */
      if (bytes_sent <= 0) {
        /* Connection closed by client */
        close(client_fd);
        break;
      }
    }
  }
}