/**
 * @file main.c
 * @author ztnel (christian911@sympatico.ca)
 * @brief
 * @version 0.1
 * @date 2024-01
 *
 * @copyright Copyright © 2024 Christian Sargusingh
 *
 */

#include "FreeRTOS.h"
#include "app_ethernet.h"
#include "config.h"
#include "ethernetif.h"
#include "health.h"
#include "hx711.h"
#include "logger.h"
#include "lwip/tcpip.h"
#include "netif/ethernet.h"
#include "stm32h723xx.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_nucleo.h"
#include "task.h"
#include <stdio.h>

// HAL structs
I2C_HandleTypeDef hi2c2;
// RTOS task structs
TaskHandle_t start_handle;
TaskHandle_t health_handle;
TaskHandle_t tcp_handle;
TaskHandle_t logger_handle;
TaskHandle_t link_handle;
TaskHandle_t dhcp_handle;
struct netif gnetif;

/**
 * @brief  Configure the MPU attributes
 * @param  None
 * @retval None
 */
static void mpu_config(void);

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
static void system_clock_config(void);

/**
 * @brief  CPU L1-Cache enable.
 * @param  None
 * @retval None
 */
static void cpu_cache_enable(void);

/**
 * @brief I2C2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C2_Init(void);

static void bsp_config(void);
static void genesis_task(void *pv_params);

/**
 * @brief  Setup the network interface
 * @param  None
 * @retval None
 */
static void netconfig_init(void);

int main(void) {
  BaseType_t x_return;
  mpu_config();
  cpu_cache_enable();
  HAL_Init();
  MX_I2C2_Init();
  system_clock_config();
  bsp_config();
  x_return = xTaskCreate(genesis_task, "genesis_task", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 24, &start_handle);
  configASSERT(start_handle);
  if (x_return != pdPASS) {
    vTaskDelete(start_handle);
  }
  vTaskStartScheduler();
  // will only reach here if rtos has insufficient memory
  for (;;)
    ;
}

static void genesis_task(void *pv_params) {
  BaseType_t x_return;
  tcpip_init(NULL, NULL);
  netconfig_init();
  logger_init(LOGGER_TRACE);
  x_return = xTaskCreate(logger_task, "logger_task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 10, &logger_handle);
  configASSERT(logger_handle);
  if (x_return != pdPASS) {
    vTaskDelete(logger_handle);
  }
  x_return = xTaskCreate(health_main, "health_task", configMINIMAL_STACK_SIZE, (void *)&hi2c2, tskIDLE_PRIORITY + 32, &health_handle);
  configASSERT(health_handle);
  if (x_return != pdPASS) {
    vTaskDelete(health_handle);
  }
  /* Delete the Init Thread */
  vTaskDelete(start_handle);
}

static void MX_I2C2_Init(void) {
  HAL_StatusTypeDef status;
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x00707CBB;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  status = HAL_I2C_Init(&hi2c2);
  if (status != HAL_OK) {
    return;
  }
  status = HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE);
  if (status != HAL_OK) {
    return;
  }
  status = HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0);
  if (status != HAL_OK) {
    return;
  }
}

static void netconfig_init(void) {
  ip_addr_t ipaddr;
  ip_addr_t netmask;
  ip_addr_t gw;

#if LWIP_DHCP
  ip_addr_set_zero_ip4(&ipaddr);
  ip_addr_set_zero_ip4(&netmask);
  ip_addr_set_zero_ip4(&gw);
#else
  /* IP address default setting */
  IP4_ADDR(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
  IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
  IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
#endif

  /* add the network interface */
  netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);

  /*  Registers the default network interface */
  netif_set_default(&gnetif);

  ethernet_link_status_updated(&gnetif);
  BaseType_t x_returned;
#if LWIP_NETIF_LINK_CALLBACK
  netif_set_link_callback(&gnetif, ethernet_link_status_updated);
  x_returned = xTaskCreate(ethernet_link_thread, "eth_link_task", configMINIMAL_STACK_SIZE, &gnetif,
                           tskIDLE_PRIORITY + 24, &link_handle);
  configASSERT(link_handle);
  if (x_returned != pdPASS) {
    vTaskDelete(link_handle);
  }
#endif

#if LWIP_DHCP
  x_returned = xTaskCreate(dhcp_task, "dhcp_task", configMINIMAL_STACK_SIZE, &gnetif,
                           tskIDLE_PRIORITY + 16, &dhcp_handle);
  configASSERT(dhcp_handle);
  if (x_returned != pdPASS) {
    vTaskDelete(dhcp_handle);
  }
#endif
}

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

static void cpu_cache_enable(void) {
  SCB_EnableICache();
  SCB_EnableDCache();
}

static void bsp_config(void) {
  BSP_LED_Init(LED2);
  BSP_LED_Init(LED3);
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
#endif // USE_FULL_ASSERT