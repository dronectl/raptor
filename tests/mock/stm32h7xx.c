
#include "stm32h7xx.h"

static volatile uint32_t tick;

uint32_t HAL_GetTick(void) {
  return tick;
}

GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
  return GPIOx->IDR & (1 << GPIO_Pin) ? GPIO_PIN_SET : GPIO_PIN_RESET;
}

void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState) {
  if (PinState == GPIO_PIN_SET) {
    GPIOx->ODR |= (1 << GPIO_Pin);
  } else {
    GPIOx->ODR &= ~(1 << GPIO_Pin);
  }
}

void HAL_GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
  GPIOx->ODR ^= (1 << GPIO_Pin);
}
