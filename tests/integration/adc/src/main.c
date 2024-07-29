/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"
#include "tim.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define ADC_VREF (float)3.3
#define ADC_CHANNELS 2
#define ADC_DMA_BUFFER_LEN ADC_CHANNELS
#define ADC_OFFSET 0
#define ADC_RESOLUTION (1 << 16)
#define SPLIT_IDX (int)((ADC_DMA_BUFFER_LEN / 2) - 1)

#define ACCL1_BUFLEN 16
#define ACCL2_BUFLEN 512

#define UART_BATCH_SIZE 50
#define UART_MSG_LEN (UART_BATCH_SIZE * 6) + 10

volatile __attribute__((section(".dma_buffer"))) uint16_t adc_dma_buffer[ADC_DMA_BUFFER_LEN];

struct accumulator_l1_ctx {
  uint32_t cumulative;
  uint8_t head;
  uint16_t adc_raw_buffer[ACCL1_BUFLEN];
};

struct accumulator_l2_ctx {
  uint8_t read_idx;
  uint8_t write_idx;
  float voltages[ACCL2_BUFLEN];
};

static volatile struct accumulator_l1_ctx accumulator_l1 = {0};
static volatile struct accumulator_l2_ctx accumulator_l2 = {0};

void SystemClock_Config(void);

void adc_polling(void) {
  char message[10] = {0};
  uint32_t adc_raw = 0;
  while (1) {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    adc_raw = HAL_ADC_GetValue(&hadc1);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
    sprintf(message, "%hu\r\n", adc_raw);
    HAL_UART_Transmit(&huart3, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);
    HAL_Delay(1);
  }
}

void run_accumulator_l2() {
  volatile struct accumulator_l1_ctx *acc1 = &accumulator_l1;
  volatile struct accumulator_l2_ctx *acc2 = &accumulator_l2;
  if ((acc2->write_idx + 1) % ACCL2_BUFLEN != acc2->read_idx) {
    acc2->voltages[acc2->write_idx] = ((float)acc1->cumulative / (ACCL1_BUFLEN * ADC_RESOLUTION)) * ADC_VREF;
    acc2->write_idx = (acc2->write_idx + 1) % ACCL2_BUFLEN;
  }
}

void run_accumulator_l1() {
  volatile struct accumulator_l1_ctx *acc = &accumulator_l1;
  for (int i = 0; i < ADC_CHANNELS; i++) {
    uint16_t raw = adc_dma_buffer[i];
    acc->cumulative -= acc->adc_raw_buffer[acc->head];
    acc->cumulative += raw;
    acc->adc_raw_buffer[acc->head] = raw;
    acc->head = (acc->head + 1) % ACCL1_BUFLEN;
  }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
  run_accumulator_l1();
  run_accumulator_l2();
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc) {
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
}
void uart_batch_write() {
  char message[UART_MSG_LEN];
  int len = 0;
  char *ptr = message;
  memset(message, 0, UART_MSG_LEN);
  volatile struct accumulator_l2_ctx *acc2 = &accumulator_l2;
  for (int i = 0; i < UART_BATCH_SIZE; i++) {
    if (acc2->read_idx != acc2->write_idx) {
      len = sprintf(ptr, "%.3f", acc2->voltages[i]);
      ptr += len;
      if (i < UART_BATCH_SIZE - 1) {
        *ptr++ = ',';
      } else {
        *ptr++ = '\r';
        *ptr++ = '\n';
      }
      acc2->read_idx = (acc2->read_idx + 1) % ACCL2_BUFLEN;
    }
  }
  HAL_UART_Transmit(&huart3, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);
}

void adc_dma(void) {
  HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED);
  HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_dma_buffer, ADC_DMA_BUFFER_LEN);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
  char msg[10];
  while (1) {
    volatile struct accumulator_l2_ctx *acc2 = &accumulator_l2;
    if (acc2->read_idx != acc2->write_idx) {
      sprintf(msg, "%.4f\r\n", acc2->voltages[acc2->read_idx]);
      acc2->read_idx = (acc2->read_idx + 1) % ACCL2_BUFLEN;
    }
    HAL_UART_Transmit(&huart3, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
  }
}

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_USART3_UART_Init();
  MX_TIM2_Init();
  // adc_polling();
  adc_dma();
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
   */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
   */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {
  }

  /** Macro to configure the PLL clock source
   */
  __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48 | RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 275;
  RCC_OscInitStruct.PLL.PLLP = 1;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_1;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK) {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1) {
  }
  /* USER CODE END Error_Handler_Debug */
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
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
