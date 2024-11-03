/**
 * @file aggregator.c
 * @author ztnel (christian911@sympatico.ca)
 * @brief ADC measurement aggregator
 * @version 0.1
 * @date 2024-10
 *
 * @copyright Copyright © 2024 Christian Sargusingh
 *
 */

#include "adc_aggregator.h"

volatile __attribute__((section(".dma_buffer"))) uint16_t adc_dma_buffer[ADC_CHANNELS * ADC_DMA_BUFFER_LEN] = {0};

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {

}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc) {

}

