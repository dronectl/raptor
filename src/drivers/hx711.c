
#include "hx711.h"
#include "FreeRTOS.h"
#include "stm32h723xx.h"
#include "task.h"

static void delay_us(uint8_t micros) {
  TIM2->CR1 |= TIM_CR1_CEN;
  for (uint8_t i = 0; i < micros; i++) {
    while (!(TIM2->SR & TIM_SR_UIF)) {
    };
    TIM2->SR &= ~TIM_SR_UIF;
  }
  TIM2->CR1 &= ~TIM_CR1_CEN;
}

static void wait_ready(void) {
  while (!(GPIOF->ODR & GPIO_ODR_OD0_Msk)) {
    vTaskDelay(1UL);
  }
}

void hx711_boot(void) {
  GPIOF->BSRR |= GPIO_BSRR_BR1;
}

void hx711_shutdown(void) {
  GPIOF->BSRR |= GPIO_BSRR_BR1;
  GPIOF->BSRR |= GPIO_BSRR_BS1;
  delay_us(60);
}

void hx711_init(void) {
  // configure SDA and SCK gpios
  RCC->AHB4ENR &= ~(RCC_AHB4ENR_GPIOFEN_Msk);
  RCC->AHB4ENR |= RCC_AHB4ENR_GPIOFEN;
  GPIOF->MODER &= ~(GPIO_MODER_MODE0_Msk | GPIO_MODER_MODE1_Msk);
  GPIOF->MODER |= GPIO_MODER_MODE1_0;
  GPIOF->PUPDR &= ~(GPIO_PUPDR_PUPD0_Msk);
  GPIOF->PUPDR |= GPIO_PUPDR_PUPD0_0;
  // configure timer 2 for microsecond precision
  RCC->APB1LENR |= RCC_APB1LENR_TIM2EN;
  TIM2->CR1 &= ~TIM_CR1_CEN;
  TIM2->PSC = SystemCoreClock / (1000000 - 1);
  TIM2->ARR = 0x01;
  TIM2->EGR = TIM_EGR_UG;
  hx711_boot();
}

uint32_t hx711_read(void) {
  uint32_t dout = 0x0;
  wait_ready();
  taskENTER_CRITICAL();
  for (uint8_t i = 24; i > 0; i--) {
    delay_us(1);
    GPIOF->BSRR |= GPIO_BSRR_BS1;
    dout |= ((GPIOF->ODR & GPIO_ODR_OD0_Msk) << i);
    delay_us(1);
    GPIOF->BSRR |= GPIO_BSRR_BR1;
  }
  // prepare next read cycle (2 additional pulses keeps ch. A @ Gain: 128)
  for (uint8_t i = 0; i < 2; i++) {
    delay_us(1);
    GPIOF->BSRR |= GPIO_BSRR_BS1;
    delay_us(1);
    GPIOF->BSRR |= GPIO_BSRR_BR1;
  }
  taskEXIT_CRITICAL();
  return dout;
}

void hx711_task(void *pv_params) {
  hx711_init();
  // TODO: support 80Hz mode
  const TickType_t delay = 100 / portTICK_PERIOD_MS;
  // uint32_t measurement = 0;
  while (1) {
    // measurement = hx711_read();
    vTaskDelay(delay);
  }
}
