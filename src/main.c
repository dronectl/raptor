
#include "FreeRTOS.h"
#include "stm32f4xx.h"
#include "task.h"

void vspinlock(void *pv_params) {
  configASSERT(((uint32_t)pv_params) > 10);
  uint32_t x_delay = (uint32_t)pv_params;
  // Configue LEDs
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN; // Enable the clock of port D of the
  GPIOD->MODER |= GPIO_MODER_MODER12_0;
  GPIOD->MODER |= GPIO_MODER_MODER13_0;
  GPIOD->MODER |= GPIO_MODER_MODER14_0;
  GPIOD->MODER |= GPIO_MODER_MODER15_0;
  for (;;) {
    // Turn on LEDs
    GPIOD->BSRR = 1 << 12;
    vTaskDelay(x_delay);
    GPIOD->BSRR = 1 << 13;
    vTaskDelay(x_delay);
    GPIOD->BSRR = 1 << 14;
    vTaskDelay(x_delay);
    GPIOD->BSRR = 1 << 15;
    vTaskDelay(x_delay);
    GPIOD->BSRR = 1 << (12 + 16);
    vTaskDelay(x_delay);
    GPIOD->BSRR = 1 << (13 + 16);
    vTaskDelay(x_delay);
    GPIOD->BSRR = 1 << (14 + 16);
    vTaskDelay(x_delay);
    GPIOD->BSRR = 1 << (15 + 16);
    vTaskDelay(x_delay);
  }
}

int main(void) {
  BaseType_t x_returned;
  SystemInit();
  x_returned = xTaskCreate(vspinlock, "spinlock", configMINIMAL_STACK_SIZE,
                           (void *)100, tskIDLE_PRIORITY, NULL);
  if (x_returned == pdPASS) {
  }
  /* Start the scheduler. */
  vTaskStartScheduler();
  for (;;) {
  }
  return 0;
}