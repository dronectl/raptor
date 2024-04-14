
#include "main.h"
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
#include "app_ethernet.h"
#include "cmsis_os2.h"
#include "ethernetif.h"
#include "health.h"
#include "logger.h"
#include "lwip/tcpip.h"
#include "main.h"
#include "netif/ethernet.h"
#include "stm32h723xx.h"
#include "stm32h7xx_hal.h"

ETH_DMADescTypeDef DMARxDscrTab[ETH_RX_DESC_CNT] __attribute__((section(".RxDecripSection")));
ETH_DMADescTypeDef DMATxDscrTab[ETH_TX_DESC_CNT] __attribute__((section(".TxDecripSection")));

// task attributes
const osThreadAttr_t genesis_attr = {
    .name = "genesis_attr",
    .priority = osPriorityNormal,
};
const osThreadAttr_t health_attr = {
    .name = "health_attr",
    .priority = osPriorityNormal1,
};
const osThreadAttr_t logger_attr = {
    .name = "logger_attr",
    .priority = osPriorityLow,
};
const osThreadAttr_t link_attr = {
    .name = "link_attr",
    .priority = osPriorityNormal,
};
const osThreadAttr_t dhcp_attr = {
    .name = "dhcp_attr",
    .priority = osPriorityNormal,
};

// task handles
static osThreadId_t genesis_handle;
static osThreadId_t health_handle;
static osThreadId_t logger_handle;
static osThreadId_t link_handle;
static osThreadId_t dhcp_handle;
struct netif gnetif;

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

static __NO_RETURN void genesis_task(void *argument);
static void netconfig_init(void);

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
  MPU_Config();
  SCB_EnableICache();
  SCB_EnableDCache();
  HAL_Init();
  SystemClock_Config();
  PeriphCommonClock_Config();
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_FDCAN1_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_SPI2_Init();
  MX_SDMMC1_SD_Init();
  MX_TIM1_Init();
  MX_UART7_Init();
  MX_UART9_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_HS_USB_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_ADC3_Init();
  MX_RTC_Init();
#ifndef RAPTOR_DEBUG
  MX_WWDG1_Init();
#endif // RAPTOR_DEBUG
  MX_TIM13_Init();
  osKernelInitialize();
  genesis_handle = osThreadNew(genesis_task, NULL, &genesis_attr);
  if (genesis_handle != NULL) {
  }
  osKernelStart();
  while (1) {
  }
}

static __NO_RETURN void genesis_task(void __attribute__((unused)) * argument) {
  tcpip_init(NULL, NULL);
  netconfig_init();
  logger_init(LOGGER_TRACE);
  logger_handle = osThreadNew(logger_task, NULL, &logger_attr);
  if (logger_handle == NULL) {
  }
  info("Created logging task");
  health_handle = osThreadNew(health_main, NULL, &health_attr);
  if (health_handle == NULL) {
  }
  info("Created health task");
  osThreadExit();
}

/**
 * @brief  Setup the network interface
 * @param  None
 * @retval None
 */
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
#if LWIP_NETIF_LINK_CALLBACK
  netif_set_link_callback(&gnetif, ethernet_link_status_updated);
  link_handle = osThreadNew(ethernet_link_thread, &gnetif, &link_attr);
  if (link_handle == NULL) {
  }
#endif

#if LWIP_DHCP
  dhcp_handle = osThreadNew(dhcp_task, &gnetif, &dhcp_attr);
  if (dhcp_handle == NULL) {
  }
#endif
}
