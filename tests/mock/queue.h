
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"

typedef struct _StaticQueue_t {
  void *null;
} StaticQueue_t;

struct QueueDefinition; /* Using old naming convention so as not to break kernel aware debuggers. */
typedef struct QueueDefinition *QueueHandle_t;

QueueHandle_t xQueueCreate(const unsigned long queue_length, const unsigned long item_size);
QueueHandle_t xQueueCreateStatic(const unsigned long queue_length, const unsigned long item_size, void *buffer, StaticQueue_t *ctrl);
BaseType_t xQueueReceive(QueueHandle_t xQueue, void *const pvBuffer, TickType_t xTicksToWait);
BaseType_t xQueueSend(QueueHandle_t xQueue, const void *const pvItemToQueue, TickType_t xTicksToWait);