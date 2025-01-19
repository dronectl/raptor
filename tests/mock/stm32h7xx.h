
#pragma once

#include <stdint.h>

typedef struct
{
  uint32_t MODER;   /*!< GPIO port mode register,               Address offset: 0x00      */
  uint32_t OTYPER;  /*!< GPIO port output type register,        Address offset: 0x04      */
  uint32_t OSPEEDR; /*!< GPIO port output speed register,       Address offset: 0x08      */
  uint32_t PUPDR;   /*!< GPIO port pull-up/pull-down register,  Address offset: 0x0C      */
  uint32_t IDR;     /*!< GPIO port input data register,         Address offset: 0x10      */
  uint32_t ODR;     /*!< GPIO port output data register,        Address offset: 0x14      */
  int32_t BSRR;     /*!< GPIO port bit set/reset,               Address offset: 0x18      */
  uint32_t LCKR;    /*!< GPIO port configuration lock register, Address offset: 0x1C      */
  uint32_t AFR[2];  /*!< GPIO alternate function registers,     Address offset: 0x20-0x24 */
} GPIO_TypeDef;

typedef enum {
  GPIO_PIN_RESET = 0U,
  GPIO_PIN_SET
} GPIO_PinState;

uint32_t HAL_GetTick(void);
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);
void HAL_GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
