
#include "utils.h"
#include "adc.h"
#include <avr/cpufunc.h>

/**
 * @brief
 *
 * @param cycles
 */
void delay_cycles(uint64_t cycles) {
  for (uint64_t i = 0; i < cycles; i++) {
    _NOP();
  };
}

/**
 * @brief Generate a random number by sampling the ADC.
 * NOTE: This should only be used at program startup to avoid interfering with
 * ADC operation.
 *
 * @return uint8_t randomly generated number
 */
uint8_t generate_random(void) {
  adc_init();
  delay_cycles(51);
  uint8_t rand = adc_sample_lower(ADC_CH1);
  adc_shutdown();
  return rand;
}