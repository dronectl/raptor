/**
 * @file adc.h
 * @brief
 * @version 0.1
 * @date 2023-01
 *
 * @copyright Copyright © 2023 dronectl
 *
 */

#ifndef __ADC_H__
#define __ADC_H__

#include <stdint.h>

enum ADCChannel {
  ADC_CH0 = 0x0,
  ADC_CH1 = 0x1,
  ADC_CH2 = 0x2,
  ADC_CH3 = 0x3,
  ADC_CH4 = 0x4,
  ADC_CH5 = 0x5,
  ADC_CH6 = 0x6,
  ADC_CH7 = 0x7,
  ADC_CH8 = 0x8,
  ADC_1V1 = 0xE,
  ADC_GND = 0xF
};

void adc_init(void);
void adc_shutdown(void);
uint8_t adc_sample_lower(enum ADCChannel channel);

#endif // __ADC_H__
