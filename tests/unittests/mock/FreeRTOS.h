
#pragma once
#include <stdint.h>

#define pdPASS 1
#define portMAX_DELAY 0xffffffffUL
#define configSTACK_DEPTH_TYPE uint16_t
#define configMINIMAL_STACK_SIZE ((uint16_t)512)

typedef long BaseType_t;
typedef uint32_t TickType_t;
typedef unsigned long UBaseType_t;
