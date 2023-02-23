/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    stm32f4xx_it.c
 * @brief   Interrupt Service Routines.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"
#include "stm32f4xx.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
static void __delay(void) {
  for (int i = 0; i < 100000; i++) {
  }
}

/* External variables --------------------------------------------------------*/

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
 * @brief This function handles Non maskable interrupt. Blink the NMI status
 * LED.
 */
void NMI_Handler(void) {
  uint8_t toggle = 0;
  while (1) {
    toggle ^= 1;
    GPIOD->BSRR = 1 << (12 + (16 * toggle));
    __delay();
  }
}

/**
 * @brief This function handles Hard fault interrupt. The system simply blinks
 * all status LEDS sequentially indicating the system has hit a
 * hard fault.
 */
void HardFault_Handler(void) {
  uint8_t toggle = 0;
  while (1) {
    toggle ^= 1;
    GPIOD->BSRR = 1 << (12 + (16 * toggle));
    __delay();
    GPIOD->BSRR = 1 << (13 + (16 * toggle));
    __delay();
    GPIOD->BSRR = 1 << (14 + (16 * toggle));
    __delay();
    GPIOD->BSRR = 1 << (15 + (16 * toggle));
    __delay();
  }
}

/**
 * @brief This function handles Memory management fault.
 */
void MemManage_Handler(void) {
  uint8_t toggle = 0;
  while (1) {
    toggle ^= 1;
    GPIOD->BSRR = 1 << (13 + (16 * toggle));
    __delay();
  }
}

/**
 * @brief This function handles Pre-fetch fault, memory access fault.
 */
void BusFault_Handler(void) {
  uint8_t toggle = 0;
  while (1) {
    toggle ^= 1;
    GPIOD->BSRR = 1 << (14 + (16 * toggle));
    __delay();
  }
}

/**
 * @brief This function handles Undefined instruction or illegal state.
 */
void UsageFault_Handler(void) {
  uint8_t toggle = 0;
  while (1) {
    toggle ^= 1;
    GPIOD->BSRR = 1 << (15 + (16 * toggle));
    __delay();
  }
}

/**
 * @brief This function handles Debug monitor.
 */
void DebugMon_Handler(void) {
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
 * @brief This function handles TIM1 update interrupt and TIM10 global
 * interrupt.
 */
void TIM1_UP_TIM10_IRQHandler(void) {
  /* USER CODE BEGIN TIM1_UP_TIM10_IRQn 0 */

  /* USER CODE END TIM1_UP_TIM10_IRQn 0 */
  /* USER CODE BEGIN TIM1_UP_TIM10_IRQn 1 */

  /* USER CODE END TIM1_UP_TIM10_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */