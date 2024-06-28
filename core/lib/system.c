
#include "system.h"
#include "stm32h7xx_nucleo.h"

#define COUNTS 1000

__attribute__((section(".data.sys"))) static uint32_t tick_counter = 0;

void system_tick_indicator(void) {
  tick_counter++;
  if (tick_counter > COUNTS) {
    tick_counter = 0;
    BSP_LED_Toggle(LED1);
  }
}

void system_spinlock(void) {
  BSP_LED_On(LED3);
  while (1) {
  }
}
