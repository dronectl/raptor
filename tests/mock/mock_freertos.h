#pragma once

#include "gmock/gmock.h"

extern "C" {
#include "FreeRTOS.h"
#include "queue.h"
}

class MockFreeRTOS {
public:
  MOCK_METHOD(void, vTaskDelay, (const TickType_t));
  MOCK_METHOD(BaseType_t, xQueueReceive, (QueueHandle_t, void *const, TickType_t));
  MOCK_METHOD(BaseType_t, xQueueGenericSend, ( QueueHandle_t, const void * const, TickType_t, const BaseType_t));
	MOCK_METHOD(QueueHandle_t, xQueueGenericCreateStatic, (const UBaseType_t, const UBaseType_t, uint8_t *, StaticQueue_t *, const uint8_t));
  MOCK_METHOD(BaseType_t, xTaskCreate, (TaskFunction_t, const char * const, const configSTACK_DEPTH_TYPE, void * const, UBaseType_t, TaskHandle_t * const));
};

MockFreeRTOS *mock_freertos = nullptr;

// C-style wrapper functions for the mocks
extern "C" {

void vTaskDelay(const TickType_t xTicksToDelay) {
  return mock_freertos->vTaskDelay(xTicksToDelay);
}

BaseType_t xQueueReceive(QueueHandle_t xQueue, void *const pvBuffer, TickType_t xTicksToWait) {
  return mock_freertos->xQueueReceive(xQueue, pvBuffer, xTicksToWait);
}

BaseType_t xQueueGenericSend(QueueHandle_t xQueue, const void * const pvItemToQueue, TickType_t xTicksToWait, const BaseType_t xCopyPosition) {
  return mock_freertos->xQueueGenericSend(xQueue, pvItemToQueue, xTicksToWait, xCopyPosition);
}

QueueHandle_t xQueueGenericCreateStatic(const UBaseType_t uxQueueLength, const UBaseType_t uxItemSize, uint8_t *pucQueueStorage, StaticQueue_t *pxStaticQueue, const uint8_t ucQueueType) {
  return mock_freertos->xQueueGenericCreateStatic(uxQueueLength, uxItemSize, pucQueueStorage, pxStaticQueue, ucQueueType);
}

BaseType_t xTaskCreate(TaskFunction_t pxTaskCode, const char * const pcName, const configSTACK_DEPTH_TYPE usStackDepth, void * const pvParameters, UBaseType_t uxPriority, TaskHandle_t * const pxCreatedTask) {
  return mock_freertos->xTaskCreate(pxTaskCode, pcName, usStackDepth, pvParameters, uxPriority, pxCreatedTask);
}
}

