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

void adc_init(void);
void adc_shutdown(void);
uint8_t adc_sample_lower(void);

#endif // __ADC_H__
