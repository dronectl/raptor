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
  _NOP();
  // enable adc
  ADCSRA |= (1 << ADEN);
  // use AVCC
  ADMUX |= (1 << REFS0);
  // Set the prescaler to 128
  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
  // save power by disabling input to adc 0
  DIDR0 |= (1 << ADC0D);
}

/**
 * @brief for random number generation. BUG: Sampling not working.
 *
 * @return uint8_t
 */
uint8_t adc_sample_lower(enum ADCChannel channel) {
  // enable ADC0 input
  DIDR0 &= ~(1 << ADC0D);
  // sample from ADC0
  ADMUX = (ADMUX & 0xF0) | (uint8_t)channel;
  // start conversion manually
  ADCSRA |= (1 << ADSC);
  while (ADCSRA & (1 << ADSC)) {
    _NOP();
  }
  // disable ADC0 input
  DIDR0 |= (1 << ADC0D);
  return ADCL;
}

void adc_shutdown(void) {
  // disable ADC then shutdown
  ADCSRA &= ~(1 << ADEN);
  PRR |= (1 << PRADC);
}
