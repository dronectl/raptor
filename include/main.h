/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#include "stm32h7xx_hal.h"
#include "stm32h7xx_nucleo.h"

/*Static IP ADDRESS*/
#define IP_ADDR0 ((uint8_t)192U)
#define IP_ADDR1 ((uint8_t)168U)
#define IP_ADDR2 ((uint8_t)0U)
#define IP_ADDR3 ((uint8_t)10U)

/*NETMASK*/
#define NETMASK_ADDR0 ((uint8_t)255U)
#define NETMASK_ADDR1 ((uint8_t)255U)
#define NETMASK_ADDR2 ((uint8_t)255U)
#define NETMASK_ADDR3 ((uint8_t)0U)

/*Gateway Address*/
#define GW_ADDR0 ((uint8_t)192U)
#define GW_ADDR1 ((uint8_t)168U)
#define GW_ADDR2 ((uint8_t)0U)
#define GW_ADDR3 ((uint8_t)1U)

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* Private defines -----------------------------------------------------------*/
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define ESC_UART_RX_Pin GPIO_PIN_6
#define ESC_UART_RX_GPIO_Port GPIOF
#define ESC_UART_TX_Pin GPIO_PIN_7
#define ESC_UART_TX_GPIO_Port GPIOF
#define ADC_BLDC_TSENSE_Pin GPIO_PIN_8
#define ADC_BLDC_TSENSE_GPIO_Port GPIOF
#define ADC_BATT_TSENSE_Pin GPIO_PIN_9
#define ADC_BATT_TSENSE_GPIO_Port GPIOF
#define ADC_ESC_TSENSE_Pin GPIO_PIN_10
#define ADC_ESC_TSENSE_GPIO_Port GPIOF
#define MCO_Pin GPIO_PIN_0
#define MCO_GPIO_Port GPIOH
#define ADC_BLDC_ISENSE_Pin GPIO_PIN_0
#define ADC_BLDC_ISENSE_GPIO_Port GPIOC
#define RMII_MDC_Pin GPIO_PIN_1
#define RMII_MDC_GPIO_Port GPIOC
#define ADC_AUX1_TSENSE_Pin GPIO_PIN_2
#define ADC_AUX1_TSENSE_GPIO_Port GPIOC
#define ADC_AUX2_TSENSE_Pin GPIO_PIN_3
#define ADC_AUX2_TSENSE_GPIO_Port GPIOC
#define ADC_BLDC_VSENSE_Pin GPIO_PIN_0
#define ADC_BLDC_VSENSE_GPIO_Port GPIOA
#define RMII_REF_CLK_Pin GPIO_PIN_1
#define RMII_REF_CLK_GPIO_Port GPIOA
#define RMII_MDIO_Pin GPIO_PIN_2
#define RMII_MDIO_GPIO_Port GPIOA
#define ADC_LC1_P_Pin GPIO_PIN_4
#define ADC_LC1_P_GPIO_Port GPIOA
#define ADC_LC1_N_Pin GPIO_PIN_5
#define ADC_LC1_N_GPIO_Port GPIOA
#define RMII_CRS_DV_Pin GPIO_PIN_7
#define RMII_CRS_DV_GPIO_Port GPIOA
#define RMII_RXD0_Pin GPIO_PIN_4
#define RMII_RXD0_GPIO_Port GPIOC
#define RMII_RXD1_Pin GPIO_PIN_5
#define RMII_RXD1_GPIO_Port GPIOC
#define LED_GREEN_Pin GPIO_PIN_0
#define LED_GREEN_GPIO_Port GPIOB
#define ADC_LC2_P_Pin GPIO_PIN_11
#define ADC_LC2_P_GPIO_Port GPIOF
#define ADC_LC2_N_Pin GPIO_PIN_12
#define ADC_LC2_N_GPIO_Port GPIOF
#define ADC_LC3_P_Pin GPIO_PIN_13
#define ADC_LC3_P_GPIO_Port GPIOF
#define ADC_LC3_N_Pin GPIO_PIN_14
#define ADC_LC3_N_GPIO_Port GPIOF
#define SH_UART_RX_Pin GPIO_PIN_0
#define SH_UART_RX_GPIO_Port GPIOG
#define SH_UART_TX_Pin GPIO_PIN_1
#define SH_UART_TX_GPIO_Port GPIOG
#define ESC_PWM_Pin GPIO_PIN_9
#define ESC_PWM_GPIO_Port GPIOE
#define BATT_CHARGE_Pin GPIO_PIN_14
#define BATT_CHARGE_GPIO_Port GPIOE
#define BATT_PWR_RELAY_Pin GPIO_PIN_15
#define BATT_PWR_RELAY_GPIO_Port GPIOE
#define ESTOP_RELAY_Pin GPIO_PIN_10
#define ESTOP_RELAY_GPIO_Port GPIOB
#define AUX_PWR_RELAY_Pin GPIO_PIN_11
#define AUX_PWR_RELAY_GPIO_Port GPIOB
#define RMII_TXD1_Pin GPIO_PIN_13
#define RMII_TXD1_GPIO_Port GPIOB
#define LED_RED_Pin GPIO_PIN_14
#define LED_RED_GPIO_Port GPIOB
#define STLK_VCP_RX_Pin GPIO_PIN_8
#define STLK_VCP_RX_GPIO_Port GPIOD
#define STLK_VCP_TX_Pin GPIO_PIN_9
#define STLK_VCP_TX_GPIO_Port GPIOD
#define USB_FS_PWR_EN_Pin GPIO_PIN_10
#define USB_FS_PWR_EN_GPIO_Port GPIOD
#define USB_FS_OVCR_Pin GPIO_PIN_7
#define USB_FS_OVCR_GPIO_Port GPIOG
#define USB_FS_VBUS_Pin GPIO_PIN_9
#define USB_FS_VBUS_GPIO_Port GPIOA
#define USB_FS_ID_Pin GPIO_PIN_10
#define USB_FS_ID_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define ESC_FDCAN_RX_Pin GPIO_PIN_0
#define ESC_FDCAN_RX_GPIO_Port GPIOD
#define ESC_FDCAN_TX_Pin GPIO_PIN_1
#define ESC_FDCAN_TX_GPIO_Port GPIOD
#define RMII_TX_EN_Pin GPIO_PIN_11
#define RMII_TX_EN_GPIO_Port GPIOG
#define RMII_TXD0_Pin GPIO_PIN_13
#define RMII_TXD0_GPIO_Port GPIOG
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define LED_YELLOW_Pin GPIO_PIN_1
#define LED_YELLOW_GPIO_Port GPIOE

#endif /* __MAIN_H */
