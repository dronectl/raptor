/**
 * @file adc.c
 * @brief
 * @version 0.1
 * @date 2023-01
 *
 * @copyright Copyright © 2023 dronectl
 *
 */

#include "adc.h"
#include <avr/cpufunc.h>
#include <avr/io.h>

/**
 * @brief Initialize ADC. For now use default settings.
 *
 */
void adc_init(void) {
  // startup the adc subsystem by disabling power reduction
  PRR &= ~(1 << PRADC);
  // enable adc
  ADCSRA |= (1 << ADEN);
}

/**
 * @brief for random number generation.
 *
 * @return uint8_t
 */
uint8_t adc_sample_lower(void) {
  // start conversion manually
  ADCSRA |= (1 << ADSC);
  while (ADCSRA & (1 << ADSC)) {
    _NOP();
  }
  return ADCL;
}

void adc_shutdown(void) {
  // TODO: ensure conversion is not in progress
  ADCSRA &= ~(1 << ADEN);
  PRR |= (1 << PRADC);
}
