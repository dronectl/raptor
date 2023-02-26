
#include "FreeRTOS.h"
#include "config.h"
#include "stm32h723xx.h"
#include "task.h"

#ifdef RAPTOR_DEBUG
/**
 * @brief FreeRTOS task CPU usage statistics timer configuration.  Only used in
 * debug builds. TIM2 peripheral is used arbitrarily.
 *
 */
void vconfigure_rtos_stats_timer(void) {
  // Enable the clock for Timer 2
  RCC->APB1HENR |= RCC_APB1HENR_TIM23EN;
  // Set the prescaler for Timer 2
  TIM23->PSC = SystemCoreClock / 10000 - 1;
  // Configure Timer 2 in up-counting mode
  TIM23->CR1 &= ~TIM_CR1_DIR; // Up-counting mode
  TIM23->CR1 &= ~TIM_CR1_CMS; // Edge-aligned mode
  // Enable Timer 2
  TIM23->CR1 |= TIM_CR1_CEN;
}
/**
 * @brief FreeRTOS task CPU usage statistics helper. Fetches current counter
 * value.
 *
 * @return uint32_t timer2 counter register
 */
uint32_t vget_runtime_count(void) { return TIM23->CNT; }
#endif // RAPTOR_DEBUG

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
}

int main(void) {
  prvSetupHardware();
  /* Start the scheduler. */
  vTaskStartScheduler();
  for (;;)
    ;
  return 0;
}