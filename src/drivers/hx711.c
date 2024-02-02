
#include "hx711.h"
#include "stm32h723xx.h"
#include <stdint.h>

#define SIGN_BIT 0x800000
#define NEGATIVE_PAD 0xff000000

/**
 * @brief Microsecond delay function using hardware timer
 *
 * @param micros number of microsecond ticks to delay
 */
static void delay_us(uint8_t micros);

/**
 * @brief Wait for HX711 DOUT line to be pulled down indicating sample is ready
 */
static void wait_ready(void);

hx711_status_t hx711_shutdown(void) {
  GPIOF->BSRR &= ~GPIO_BSRR_BS1;
  delay_us(1);
  GPIOF->BSRR |= GPIO_BSRR_BS1;
  delay_us(60);
  return HX711_OK;
}

hx711_status_t hx711_reset(void) {
  hx711_shutdown();
  GPIOF->BSRR &= ~GPIO_BSRR_BS1;
  return HX711_OK;
}

/**
 * @brief Initialize ADC and GPIOs
 */
hx711_status_t hx711_init(void) {
  // configure DOUT and PCK GPIOs
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
  // boot hx711
  hx711_reset();
  return HX711_OK;
}

hx711_status_t hx711_read(float *data, hx711_config_t *config) {
  int32_t dout = 0;
  wait_ready();
  // read next conversion (Big Endian)
  for (uint8_t i = 24; i > 0; i--) {
    GPIOF->BSRR |= GPIO_BSRR_BS1;
    delay_us(1);
    dout |= ((GPIOF->ODR & GPIO_ODR_OD0_Msk) << i);
    GPIOF->BSRR &= ~GPIO_BSRR_BR1;
    delay_us(1);
  }
  // add correct signed padding
  if (dout & SIGN_BIT) {
    dout |= NEGATIVE_PAD;
  }
  // configure next ADC SOC gain and channel select
  for (int i = 0; i < (int)config->input_select; i++) {
    GPIOF->BSRR |= GPIO_BSRR_BS1;
    delay_us(1);
    GPIOF->BSRR &= ~GPIO_BSRR_BR1;
    delay_us(1);
  }
  // convert sample and return
  *data = (float)(dout - config->offset) / config->scale;
  return HX711_OK;
}

static void delay_us(uint8_t micros) {
  TIM2->CR1 |= TIM_CR1_CEN;
  for (uint8_t i = 0; i < micros; i++) {
    while (!(TIM2->SR & TIM_SR_UIF)) {
    }
    TIM2->SR &= ~TIM_SR_UIF;
  }
  TIM2->CR1 &= ~TIM_CR1_CEN;
}

static void wait_ready(void) {
  while (!(GPIOF->ODR & GPIO_ODR_OD0_Msk)) {
  }
}
