

#include "FreeRTOS.h"
#include "app_ethernet.h"
#include "config.h"
#include "ethernetif.h"
#include "lwip/tcpip.h"
#include "netif/ethernet.h"
#include "stm32h723xx.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_nucleo.h"
#include "task.h"
#include "hx711.h"

TaskHandle_t start_handle;
TaskHandle_t tcp_handle;
TaskHandle_t link_handle;
TaskHandle_t dhcp_handle;
struct netif gnetif;

/* Private function prototypes -----------------------------------------------*/
static void start_task(void *pv_params);
static void netconfig_init(void);
static void system_clock_config(void);
static void bsp_config(void);
static void mpu_config(void);
static void cpu_cache_enable(void);

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

int main(void) {
  /* Configure the MPU attributes as Device memory for ETH DMA descriptors */
  mpu_config();

  /* Enable the CPU Cache */
  cpu_cache_enable();

  /* STM32H7xx HAL library initialization:
       - Configure the SysTick to generate an interrupt each 1 msec
       - Set NVIC Group Priority to 4
       - Low Level Initialization
     */
  HAL_Init();

  /* Configure the system clock to 520 MHz */
  system_clock_config();
  /* Configure the LEDs ...*/
  bsp_config();
  BaseType_t x_returned;
  x_returned =
      xTaskCreate(start_task, "start_task", configMINIMAL_STACK_SIZE * 2, NULL,
                  tskIDLE_PRIORITY + 24, &start_handle);
  configASSERT(start_handle);
  if (x_returned != pdPASS) {
    vTaskDelete(start_handle);
  }
  vTaskStartScheduler();
}

void start_task(void *pv_params) {
  BaseType_t x_returned =
      xTaskCreate(hx711_task, "hx711_task", configMINIMAL_STACK_SIZE, NULL,
                  tskIDLE_PRIORITY + 32, &tcp_handle);
  configASSERT(tcp_handle);
  if (x_returned != pdPASS) {
    vTaskDelete(tcp_handle);
  }
  for (;;) {
    /* Delete the Init Thread */
    vTaskDelete(start_handle);
  }
}

static void bsp_config(void) {
  BSP_LED_Init(LED2);
  BSP_LED_Init(LED3);
}

/**
 * @brief  System Clock Configuration
 *         The system Clock is configured as follow :
 *            System Clock source            = PLL (HSE BYPASS)
 *            SYSCLK(Hz)                     = 520000000 (CPU Clock)
 *            HCLK(Hz)                       = 260000000 (AXI and AHBs Clock)
 *            AHB Prescaler                  = 2
 *            D1 APB3 Prescaler              = 2 (APB3 Clock  130MHz)
 *            D2 APB1 Prescaler              = 2 (APB1 Clock  130MHz)
 *            D2 APB2 Prescaler              = 2 (APB2 Clock  130MHz)
 *            D3 APB4 Prescaler              = 2 (APB4 Clock  130MHz)
 *            HSE Frequency(Hz)              = 8000000
 *            PLL_M                          = 4
 *            PLL_N                          = 260
 *            PLL_P                          = 1
 *            PLL_Q                          = 4
 *            PLL_R                          = 2
 *            VDD(V)                         = 3.3
 *            Flash Latency(WS)              = 3
 * @param  None
 * @retval None
 */
static void system_clock_config(void) {
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  HAL_StatusTypeDef ret = HAL_OK;

  /*!< Supply configuration update enable */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
  /* The voltage scaling allows optimizing the power consumption when the device
     is clocked below the maximum system frequency, to update the voltage
     scaling value regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);
  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {
  }
  /* Enable D2 domain SRAM1 Clock (0x30000000 AXI)*/
  __HAL_RCC_D2SRAM1_CLK_ENABLE();
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
  RCC_OscInitStruct.CSIState = RCC_CSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 260;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  RCC_OscInitStruct.PLL.PLLP = 1;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_1;
  ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
  if (ret != HAL_OK) {
    while (1)
      ;
  }
  /* Select PLL as system clock source and configure  bus clocks dividers */
  RCC_ClkInitStruct.ClockType =
      (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_D1PCLK1 |
       RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;
  ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3);
  if (ret != HAL_OK) {
    while (1)
      ;
  }
  /*
    Note : The activation of the I/O Compensation Cell is recommended with
    communication  interfaces (GPIO, SPI, FMC, OSPI ...)  when  operating at
    high frequencies(please refer to product datasheet) The I/O Compensation
    Cell activation  procedure requires :
          - The activation of the CSI clock
          - The activation of the SYSCFG clock
          - Enabling the I/O Compensation Cell : setting bit[0] of register
    SYSCFG_CCCSR
  */
  __HAL_RCC_CSI_ENABLE();
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  HAL_EnableCompensationCell();
}

/**
 * @brief  Configure the MPU attributes
 * @param  None
 * @retval None
 */
static void mpu_config(void) {
  MPU_Region_InitTypeDef MPU_InitStruct;
  /* Disable the MPU */
  HAL_MPU_Disable();
  /* Configure the MPU as Strongly ordered for not defined regions */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x00;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Configure the MPU attributes as Device not cacheable
     for ETH DMA descriptors */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x30000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_1KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Configure the MPU attributes as Normal Non Cacheable
     for LwIP RAM heap which contains the Tx buffers */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x30004000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_16KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER2;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
 * @brief  CPU L1-Cache enable.
 * @param  None
 * @retval None
 */
static void cpu_cache_enable(void) {
  /* Enable I-Cache */
  SCB_EnableICache();
  /* Enable D-Cache */
  SCB_EnableDCache();
}

#ifdef USE_FULL_ASSERT

/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */

  /* Infinite loop */
  while (1) {
  }
}
#endif
