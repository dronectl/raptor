/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32H7XX_NUCLEO_CONF_H
#define STM32H7XX_NUCLEO_CONF_H
/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Nucleo pin and part number defines */
#define USE_NUCLEO_144
#define USE_NUCLEO_H723ZG

/* COM define */
#define USE_COM_LOG 0U
#define USE_BSP_COM_FEATURE 1U

/* IRQ priorities */
#define BSP_BUTTON_USER_IT_PRIORITY 15U

#define BUS_SPI1_BAUDRATE 18000000
#endif /* STM32H7XX_NUCLEO_CONF_H */
