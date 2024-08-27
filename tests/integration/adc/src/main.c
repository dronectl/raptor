#include "main.h"
#include "adc.h"
#include "dma.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_adc.h"
#include "stm32h7xx_hal_gpio.h"
#include "usart.h"
#include "gpio.h"
#include "tim.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define ADC_VREF (float)3.3
#define ADC_CHANNELS (uint8_t)1
#define ADC_DMA_BUFFER_LEN 100
#define ADC_OFFSET 0
#define ADC_RESOLUTION (1 << 16)

#define ACC_STATUS_DATA_READY (1 << 0)

#define ACC_BUFLEN (uint8_t)(ADC_DMA_BUFFER_LEN / ADC_CHANNELS)

#define UART_BATCH_SIZE 20
#define UART_HEADER_LEN 20
#define UART_MSG_LEN (UART_BATCH_SIZE * 6 * 2) + 20

volatile __attribute__((section(".dma_buffer"))) uint16_t adc_dma_buffer[ADC_CHANNELS * ADC_DMA_BUFFER_LEN] = {0};

struct accumulator_ctx {
  uint8_t status;
  uint32_t timestamp;
  volatile uint16_t *dma_buffer_head;
};

struct packet {
  uint32_t timestamp;   // ms
  uint32_t sample_rate; // Hz
  uint32_t samples;
  float ch1[100];
  float ch2[100];
};

static volatile struct accumulator_ctx accumulator = {0};

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

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
  accumulator.dma_buffer_head = &adc_dma_buffer[ADC_DMA_BUFFER_LEN / 2];
  accumulator.status |= ACC_STATUS_DATA_READY;
  accumulator.timestamp = HAL_GetTick();
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc) {
  HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_12);
  accumulator.dma_buffer_head = &adc_dma_buffer[0];
  accumulator.status |= ACC_STATUS_DATA_READY;
  accumulator.timestamp = HAL_GetTick();
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc) {
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
  HAL_ADC_Stop_DMA(hadc);
}

void uart_batch_write(struct packet *payload) {
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
  char message[10] = {0};
  int len = 0;
  char *ptr = message;
  // write header
  // write body
  for (int i = 0; i < payload->samples; i++) {
    len = sprintf(ptr, "%.3f\r\n", payload->ch1[i]);
    HAL_UART_Transmit(&huart3, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);
  }
  *ptr++ = '\r';
  *ptr++ = '\n';
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
}

void process_dma_stream(volatile struct accumulator_ctx *acc, struct packet *payload) {
  payload->timestamp = acc->timestamp; // todo add timestamp
  payload->sample_rate = 1000;
  for (uint8_t i = 0; i < (ADC_DMA_BUFFER_LEN / 2); i++) {
    const float sample = (float)acc->dma_buffer_head[i];
    payload->ch1[i] = (sample / ADC_RESOLUTION) * ADC_VREF;
  }
  payload->samples = ADC_DMA_BUFFER_LEN / 2;
}

void adc_dma(void) {
  HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED);
  HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_dma_buffer, ADC_DMA_BUFFER_LEN);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
  while (1) {
    if (accumulator.status & ACC_STATUS_DATA_READY) {
      struct packet payload = {0};
      process_dma_stream(&accumulator, &payload);
      uart_batch_write(&payload);
      accumulator.status &= ~ACC_STATUS_DATA_READY;
    }
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
