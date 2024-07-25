#include "task.h"

TickType_t ticks = 0;

BaseType_t xTaskCreate(TaskFunction_t pxTaskCode, const char *const pcName, const configSTACK_DEPTH_TYPE usStackDepth, void *const pvParameters, UBaseType_t uxPriority, TaskHandle_t *const pxCreatedTask) {
  return 1;
}

TickType_t xTaskGetTickCount(void) {
  return ticks;
}

void vTaskDelete(TaskHandle_t xTaskToDelete) {
}