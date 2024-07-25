
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"

typedef struct QueueDefinition {
  uint8_t ok;
} QueueDefinition;
typedef struct QueueDefinition *QueueHandle_t;

struct queue_context {
  bool is_created;
  unsigned long queue_length;
  unsigned long item_size;
};

extern struct queue_context queue_ctx;

void reset_queue_ctx(void);
QueueHandle_t xQueueCreate(const unsigned long queue_length, const unsigned long item_size);
BaseType_t xQueueReceive(QueueHandle_t xQueue, void *const pvBuffer, TickType_t xTicksToWait);
BaseType_t xQueueSend(QueueHandle_t xQueue, const void *const pvItemToQueue, TickType_t xTicksToWait);