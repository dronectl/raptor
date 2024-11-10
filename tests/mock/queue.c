
#include <stdlib.h>
#include "queue.h"

struct queue_context queue_ctx = {0};

void reset_queue_ctx(void) {
  queue_ctx.is_created = false;
  queue_ctx.queue_length = 0;
  queue_ctx.item_size = 0;
}

QueueHandle_t xQueueCreate(const unsigned long queue_length, const unsigned long item_size) {
  queue_ctx.is_created = true;
  queue_ctx.queue_length = queue_length;
  queue_ctx.item_size = item_size;
  return (QueueHandle_t)NULL;
}

BaseType_t xQueueReceive(QueueHandle_t xQueue, void *const pvBuffer, TickType_t xTicksToWait) {
  return 1;
}

BaseType_t xQueueSend(QueueHandle_t xQueue, const void *const pvItemToQueue, TickType_t xTicksToWait) {
  return 1;
}
