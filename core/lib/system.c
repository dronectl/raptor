
#include "system.h"
#include "stm32h7xx_nucleo.h"

void system_spinlock(void) {
  // fast red led blink
  while (1) {
    BSP_LED_Toggle(LED3);
    HAL_Delay(200);
  }
}
