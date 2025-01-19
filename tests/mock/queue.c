
#include <stdlib.h>
#include "queue.h"

QueueHandle_t queue_ctx = {0};

QueueHandle_t xQueueCreate(const unsigned long queue_length, const unsigned long item_size) {
  return queue_ctx;
}

BaseType_t xQueueReceive(QueueHandle_t xQueue, void *const pvBuffer, TickType_t xTicksToWait) {
  return 1;
}

BaseType_t xQueueSend(QueueHandle_t xQueue, const void *const pvItemToQueue, TickType_t xTicksToWait) {
  return 1;
}

QueueHandle_t xQueueCreateStatic(const unsigned long queue_length, const unsigned long item_size, void *buffer, StaticQueue_t *ctrl) {
  return queue_ctx;
}