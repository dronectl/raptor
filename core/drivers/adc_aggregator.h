/**
 * @file adc_aggregator.h
 * @author ztnel (christian911@sympatico.ca)
 * @brief ADC measurement aggregator
 * @version 0.1
 * @date 2024-10
 *
 * @copyright Copyright © 2024 Christian Sargusingh
 *
 */

#ifndef __ADC_AGGREGATOR_H__
#define __ADC_AGGREGATOR_H__

#include <stdint.h>
#include "stm32h7xx_hal_adc.h"

#define ADC_AGGREGATOR_MAX_CHANNEL_BUFLEN 20
#define ADC_AGGREGATOR_CHANNELS 5

struct adc_aggregator_channel {
  struct {
    float mean;
    float max;
    float min;
  } stats;
  uint8_t head;
  uint8_t tail;
  float measurements[ADC_AGGREGATOR_MAX_CHANNEL_BUFLEN];
};

struct adc_aggregator_buffer {
  struct adc_aggregator_channel channels[ADC_AGGREGATOR_CHANNELS];
};

void adc_aggregator_get_measurements();

#endif // __ADC_AGGREGATOR_H__
