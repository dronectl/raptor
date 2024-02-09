
#include "hw_config.h"
#include "hx711.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_gpio.h"

#define SIGN_BIT 0x800000
#define NEGATIVE_PAD 0xff000000
#define DEFAULT_GAIN 1.0f
#define DEFAULT_OFFSET (int32_t)0

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
  HAL_GPIO_WritePin(HW_CONFIG_HX711_PD_SCK_GPIO_PORT, HW_CONFIG_HX711_PD_SCK_GPIO_PIN, GPIO_PIN_RESET);
  delay_us(1);
  HAL_GPIO_WritePin(HW_CONFIG_HX711_PD_SCK_GPIO_PORT, HW_CONFIG_HX711_PD_SCK_GPIO_PIN, GPIO_PIN_SET);
  delay_us(60);
  return HX711_OK;
}

hx711_status_t hx711_reset(void) {
  hx711_shutdown();
  HAL_GPIO_WritePin(HW_CONFIG_HX711_PD_SCK_GPIO_PORT, HW_CONFIG_HX711_PD_SCK_GPIO_PIN, GPIO_PIN_RESET);
  return HX711_OK;
}

/**
 * @brief Initialize ADC and GPIOs
 */
hx711_status_t hx711_init(hx711_settings_t *settings) {
  // set default scale and offset parameters
  settings->gain = DEFAULT_GAIN;
  settings->offset = DEFAULT_OFFSET;
  settings->input_select = HX711_CHA_GAIN128;
  // boot hx711
  hx711_reset();
  return HX711_OK;
}

hx711_status_t hx711_read(float *data, hx711_settings_t *settings) {
  int32_t dout = 0;
  wait_ready();
  // read next conversion (Big Endian)
  for (uint8_t i = 24; i > 0; i--) {
    HAL_GPIO_WritePin(HW_CONFIG_HX711_PD_SCK_GPIO_PORT, HW_CONFIG_HX711_PD_SCK_GPIO_PIN, GPIO_PIN_SET);
    delay_us(1);
    dout |= HAL_GPIO_ReadPin(HW_CONFIG_HX711_DOUT_GPIO_PORT, HW_CONFIG_HX711_DOUT_GPIO_PIN) << i;
    HAL_GPIO_WritePin(HW_CONFIG_HX711_PD_SCK_GPIO_PORT, HW_CONFIG_HX711_PD_SCK_GPIO_PIN, GPIO_PIN_RESET);
    delay_us(1);
  }
  // add correct signed padding
  if (dout & SIGN_BIT) {
    dout |= NEGATIVE_PAD;
  }
  // configure next ADC SOC gain and channel select
  for (int i = 0; i < (int)settings->input_select; i++) {
    HAL_GPIO_WritePin(HW_CONFIG_HX711_PD_SCK_GPIO_PORT, HW_CONFIG_HX711_PD_SCK_GPIO_PIN, GPIO_PIN_SET);
    delay_us(1);
    HAL_GPIO_WritePin(HW_CONFIG_HX711_PD_SCK_GPIO_PORT, HW_CONFIG_HX711_PD_SCK_GPIO_PIN, GPIO_PIN_RESET);
    delay_us(1);
  }
  // convert sample and return
  *data = (float)(dout - settings->offset) / settings->gain;
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
  while (!HAL_GPIO_ReadPin(HW_CONFIG_HX711_DOUT_GPIO_PORT, HW_CONFIG_HX711_DOUT_GPIO_PIN)) {
  }
}
