
#include "FreeRTOS.h"
#include "stm32f4xx.h"
#include "task.h"

// task parameters
static uint16_t delay_ms = 100;

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
 * implementation of vApplicationGetIdleTaskMemory() to provide the memory that
 * is used by the Idle task. */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize) {
  /* If the buffers to be provided to the Idle task are declared inside this
   * function then they must be declared static - otherwise they will be
   * allocated on the stack and so not exists after this function exits. */
  static StaticTask_t xIdleTaskTCB;
  static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

  /* Pass out a pointer to the StaticTask_t structure in which the Idle
   * task's state will be stored. */
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

  /* Pass out the array that will be used as the Idle task's stack. */
  *ppxIdleTaskStackBuffer = uxIdleTaskStack;

  /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
   * Note that, as the array is necessarily of type StackType_t,
   * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vApplicationIdleHook(void) {
  /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
  to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
  task.  It is essential that code added to this hook function never attempts
  to block in any way (for example, call xQueueReceive() with a block time
  specified, or call vTaskDelay()).  If the application makes use of the
  vTaskDelete() API function (as this demo application does) then it is also
  important that vApplicationIdleHook() is permitted to return to its calling
  function, because it is the responsibility of the idle task to clean up
  memory allocated by the kernel to any task that has since been deleted. */
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName) {
  (void)pcTaskName;
  (void)pxTask;

  /* Run time stack overflow checking is performed if
  configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
  function is called if a stack overflow is detected. */
  taskDISABLE_INTERRUPTS();
  for (;;)
    ;
}

static void prvSetupHardware(void) {
  /* Setup STM32 system (clock, PLL and Flash configuration) */
  SystemInit();
  // Configue LEDs
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN; // Enable the clock of port D of the
  GPIOD->MODER |= GPIO_MODER_MODER12_0;
  GPIOD->MODER |= GPIO_MODER_MODER13_0;
  GPIOD->MODER |= GPIO_MODER_MODER14_0;
  GPIOD->MODER |= GPIO_MODER_MODER15_0;
}

/**
 * @brief This is to provide the memory that is used by the RTOS daemon/time
 * task.
 *
 * If configUSE_STATIC_ALLOCATION is set to 1, so the application must provide
 * an implementation of vApplicationGetTimerTaskMemory() to provide the memory
 * that is used by the RTOS daemon/time task.
 */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize) {
  /* If the buffers to be provided to the Timer task are declared inside this
   * function then they must be declared static - otherwise they will be
   * allocated on the stack and so not exists after this function exits. */
  static StaticTask_t xTimerTaskTCB;
  static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

  /* Pass out a pointer to the StaticTask_t structure in which the Idle
   * task's state will be stored. */
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

  /* Pass out the array that will be used as the Timer task's stack. */
  *ppxTimerTaskStackBuffer = uxTimerTaskStack;

  /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
   * Note that, as the array is necessarily of type StackType_t,
   * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

void vspinlock(void *pv_params) {
  uint16_t delay = *((uint16_t *)pv_params);
  for (;;) {
    GPIOD->BSRR = 1 << 12;
    vTaskDelay(pdMS_TO_TICKS(delay));
    GPIOD->BSRR = 1 << 13;
    vTaskDelay(pdMS_TO_TICKS(delay));
    GPIOD->BSRR = 1 << 14;
    vTaskDelay(pdMS_TO_TICKS(delay));
    GPIOD->BSRR = 1 << 15;
    vTaskDelay(pdMS_TO_TICKS(delay));
    GPIOD->BSRR = 1 << (12 + 16);
    vTaskDelay(pdMS_TO_TICKS(delay));
    GPIOD->BSRR = 1 << (13 + 16);
    vTaskDelay(pdMS_TO_TICKS(delay));
    GPIOD->BSRR = 1 << (14 + 16);
    vTaskDelay(pdMS_TO_TICKS(delay));
    GPIOD->BSRR = 1 << (15 + 16);
    vTaskDelay(pdMS_TO_TICKS(delay));
  }
}

int main(void) {
  TaskHandle_t xHandle = NULL;
  BaseType_t x_returned;
  prvSetupHardware();
  x_returned = xTaskCreate(vspinlock, "spinlock", configMINIMAL_STACK_SIZE,
                           &delay_ms, tskIDLE_PRIORITY + 1, &xHandle);
  configASSERT(xHandle);
  if (x_returned != pdPASS) {
    vTaskDelete(xHandle);
  }
  /* Start the scheduler. */
  vTaskStartScheduler();
  for (;;)
    ;
  return 0;
}