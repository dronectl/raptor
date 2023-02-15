
#include "FreeRTOS.h"
#include "stm32f4xx.h"
#include "task.h"

/* configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must provide
an implementation of vApplicationGetIdleTaskMemory() to provide the memory that
is used by the Idle task. */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize) {
  /* If the buffers to be provided to the Idle task are declared inside this
  function then they must be declared static - otherwise they will be allocated
  on the stack and so not exists after this function exits. */
  static StaticTask_t xIdleTaskTCB;
  static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

  /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
  state will be stored. */
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

  /* Pass out the array that will be used as the Idle task's stack. */
  *ppxIdleTaskStackBuffer = uxIdleTaskStack;

  /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
  Note that, as the array is necessarily of type StackType_t,
  configMINIMAL_STACK_SIZE is specified in words, not bytes. */
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

/* configSUPPORT_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so
the application must provide an implementation of
vApplicationGetTimerTaskMemory() to provide the memory that is used by the Timer
service task. */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize) {
  /* If the buffers to be provided to the Timer task are declared inside this
  function then they must be declared static - otherwise they will be allocated
  on the stack and so not exists after this function exits. */
  static StaticTask_t xTimerTaskTCB;
  static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

  /* Pass out a pointer to the StaticTask_t structure in which the Timer
  task's state will be stored. */
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

  /* Pass out the array that will be used as the Timer task's stack. */
  *ppxTimerTaskStackBuffer = uxTimerTaskStack;

  /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
  Note that, as the array is necessarily of type StackType_t,
  configTIMER_TASK_STACK_DEPTH is specified in words, not bytes. */
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

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
                           (void *)100, tskIDLE_PRIORITY + 1, NULL);
  if (x_returned == pdPASS) {
  }
  /* Start the scheduler. */
  vTaskStartScheduler();
  for (;;) {
  }
  return 0;
}