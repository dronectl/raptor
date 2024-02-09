/**
 ******************************************************************************
 * @file         stm32h7xx_hal_msp.c
 * @brief        This file provides code for the MSP Initialization
 *               and de-Initialization codes.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include "hw_config.h"
#include "stm32h7xx_hal.h"

/**
 * Initializes the Global MSP.
 */
void HAL_MspInit(void) {
  __HAL_RCC_SYSCFG_CLK_ENABLE();
}

/**
 * @brief I2C MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hi2c: I2C handle pointer
 * @retval None
 */
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c) {
  GPIO_InitTypeDef gpio = {0};
  RCC_PeriphCLKInitTypeDef clk = {0};
  if (hi2c->Instance == HW_CONFIG_BME280_I2C) {
    clk.PeriphClockSelection = HW_CONFIG_BME280_I2C_CLK;
    clk.I2c123ClockSelection = RCC_I2C1235CLKSOURCE_D2PCLK1;
    HAL_RCCEx_PeriphCLKConfig(&clk);
    HW_CONFIG_BME280_GPIO_CLK_EN();
    gpio.Pin = HW_CONFIG_BME280_I2C_SDA_GPIO_PIN | HW_CONFIG_BME280_I2C_SCK_GPIO_PIN;
    gpio.Mode = GPIO_MODE_AF_OD;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    gpio.Alternate = HW_CONFIG_BME280_I2C_GPIO_AF;
    HAL_GPIO_Init(GPIOF, &gpio);
    HW_CONFIG_BME280_GPIO_CLK_EN();
  }
}

/**
 * @brief I2C MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hi2c: I2C handle pointer
 * @retval None
 */
void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c) {
  if (hi2c->Instance == HW_CONFIG_BME280_I2C) {
    __HAL_RCC_I2C2_CLK_DISABLE();
    HAL_GPIO_DeInit(HW_CONFIG_BME280_I2C_SCK_GPIO_PORT, HW_CONFIG_BME280_I2C_SDA_GPIO_PIN);
    HAL_GPIO_DeInit(HW_CONFIG_BME280_I2C_SCK_GPIO_PORT, HW_CONFIG_BME280_I2C_SCK_GPIO_PIN);
  }
}
