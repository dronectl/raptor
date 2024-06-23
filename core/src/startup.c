#include "adc.h"
#include "i2c.h"
#include "sdmmc.h"
#include "spi.h"
#include "dma.h"
#include "gpio.h"
#include "tim.h"
#include "rtc.h"
#include "usart.h"
#include "usb_otg.h"
#include "fdcan.h"
#include "wwdg.h"
#include "app_ethernet.h"
#include "cmsis_os2.h"
#include "health.h"
#include "logger.h"
#include "scpi/server.h"
#include "main.h"
#include "stm32h723xx.h"
#include "stm32h7xx_hal.h"
#include "system.h"

ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
ADC_HandleTypeDef hadc3;
DMA_HandleTypeDef hdma_adc1;
DMA_HandleTypeDef hdma_adc2;
DMA_HandleTypeDef hdma_adc3;
FDCAN_HandleTypeDef hfdcan1;
I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;
SPI_HandleTypeDef hspi2;
RTC_HandleTypeDef hrtc;
SD_HandleTypeDef hsd1;
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim13;
UART_HandleTypeDef huart7;
UART_HandleTypeDef huart9;
UART_HandleTypeDef huart3;

// task attributes
const osThreadAttr_t genesis_attr = {
    .name = "genesis_task",
    .priority = osPriorityHigh7,
};
// task handles
static osThreadId_t genesis_handle;

static __NO_RETURN void genesis_task(void __attribute__((unused)) * argument) {
  system_status_t status = SYSTEM_OK;
  status = app_ethernet_init();
  if (status != SYSTEM_OK) {
    system_spinlock();
  }
  // logging depends on TCPIP initialization
  logger_init(LOGGER_TRACE);
  if (status != SYSTEM_OK) {
    system_spinlock();
  }
  info("Created logging task\n");
  status = health_init(hi2c2);
  if (status != SYSTEM_OK) {
    system_spinlock();
  }
  info("Created health task\n");
  status = scpi_init();
  if (status != SYSTEM_OK) {
    system_spinlock();
  }
  info("Created health task\n");
  osThreadExit();
}

/**
 * @brief Port of `MPU_Config` autogenerated by CubeMX
 * @warning DO NOT MODIFY
 *
 */
static void MPU_Init(void) {
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
   */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
   */
  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
  MPU_InitStruct.BaseAddress = 0x30000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_1KB;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
   */
  MPU_InitStruct.Number = MPU_REGION_NUMBER2;
  MPU_InitStruct.BaseAddress = 0x30004000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_16KB;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
  MPU_Init();
  SCB_EnableICache();
  SCB_EnableDCache();
  HAL_Init();
  SystemClock_Config();
  PeriphCommonClock_Config();
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM1_Init();
  MX_TIM13_Init();
  MX_RTC_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_ADC3_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_SPI2_Init();
  MX_UART7_Init();
  MX_UART9_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_HS_USB_Init();
  MX_FDCAN1_Init();
  // MX_SDMMC1_SD_Init();
#ifndef RAPTOR_DEBUG
  MX_WWDG1_Init();
#endif // RAPTOR_DEBUG
  osKernelInitialize();
  genesis_handle = osThreadNew(genesis_task, NULL, &genesis_attr);
  if (genesis_handle != NULL) {
  }
  osKernelStart();
  while (1) {
  }
}
