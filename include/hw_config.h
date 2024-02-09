#ifndef __HW_CONFIG_H__
#define __HW_CONFIG_H__

#include "stm32h723xx.h"

// Load cell amplifier bit-banged GPIO
#define HW_CONFIG_HX711_PD_SCK_GPIO_PORT GPIOC
#define HW_CONFIG_HX711_DOUT_GPIO_PORT GPIOC
#define HW_CONFIG_HX711_DOUT_GPIO_PIN GPIO_PIN_14
#define HW_CONFIG_HX711_PD_SCK_GPIO_PIN GPIO_PIN_15

// BME280 weather monitoring
#define HW_CONFIG_BME280_I2C I2C2
#define HW_CONFIG_BME280_I2C_CLK RCC_PERIPHCLK_I2C2
#define HW_CONFIG_BME280_I2C_SCK_GPIO_PORT GPIOF
#define HW_CONFIG_BME280_I2C_SCK_GPIO_PIN GPIO_PIN_0
#define HW_CONFIG_BME280_I2C_SDA_GPIO_PIN GPIO_PIN_1
#define HW_CONFIG_BME280_I2C_GPIO_AF GPIO_AF4_I2C2
#define HW_CONFIG_BME280_GPIO_CLK_EN __HAL_RCC_GPIOF_CLK_ENABLE
#define HW_CONFIG_BME280_I2C_CLK_EN __HAL_RCC_I2C2_CLK_ENABLE


#endif // __HW_CONFIG_H__
