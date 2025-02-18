
#ifndef __UHCI_H__
#define __UHCI_H__

#include "system.h"

#include <FreeRTOS.h>
#include <task.h>
#include <lwip/inet.h>

enum uhci_stream_control {
  UHCI_STREAM_START = 0,
  UHCI_STREAM_WAIT,
  UHCI_STREAM_STOP,
  UHCI_STREAM_COUNT,
};

enum uhci_status_code {
  UHCI_STATUS_OK = 0,
  UHCI_STATUS_ERROR,
  UHCI_STATUS_COUNT,
};

/**
 * struct uhci_session_handle - Session state data
 */
struct uhci_stream_session_handle {
  int client_fd;
  uint16_t port;
  char ip_address[INET_ADDRSTRLEN];
  uint16_t stream_key;
  uint16_t sequence_number;
};

struct uhci_context {
  TaskHandle_t task_handle;
  struct uhci_stream_session_handle stream_handle;
};

/**
 * @brief Start uhci server task
 *
 * @param[in] task_ctx Task context
 */
void uhci_start(const struct system_task_context *task_ctx);

/**
 * @brief Set the stream control state
 *
 * @param[in] control Stream control state
 * @return UHCI_STATUS_OK on success
 */
enum uhci_status_code uhci_set_stream_control(const enum uhci_stream_control control);

#endif // __UHCI_H__
