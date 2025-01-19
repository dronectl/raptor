
#pragma once
#include "FreeRTOS.h"

#define taskYIELD() \
  {}
#define tskIDLE_PRIORITY ((UBaseType_t)0U)

typedef struct tskTaskControlBlock {
} tskTCB;

typedef void (*TaskFunction_t)(void *);
typedef struct tskTaskControlBlock *TaskHandle_t;
extern TickType_t ticks;

TickType_t xTaskGetTickCount(void);
BaseType_t xTaskCreate(TaskFunction_t pxTaskCode, const char *const pcName, const configSTACK_DEPTH_TYPE usStackDepth, void *const pvParameters, UBaseType_t uxPriority, TaskHandle_t *const pxCreatedTask);
void vTaskDelete(TaskHandle_t xTaskToDelete);
void vTaskDelay(TickType_t ticks);