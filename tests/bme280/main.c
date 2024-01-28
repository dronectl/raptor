#include "bme280.h"
#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"
#include <math.h>

#if !defined(HSE_VALUE)
#define HSE_VALUE ((uint32_t)25000000) /*!< Value of the External oscillator in Hz */
#endif                                 /* HSE_VALUE */

#if !defined(CSI_VALUE)
#define CSI_VALUE ((uint32_t)4000000) /*!< Value of the Internal oscillator in Hz*/
#endif                                /* CSI_VALUE */

#if !defined(HSI_VALUE)
#define HSI_VALUE ((uint32_t)64000000) /*!< Value of the Internal oscillator in Hz*/
#endif                                 /* HSI_VALUE */

uint32_t SystemCoreClock = 64000000;
uint32_t SystemD2Clock = 64000000;
const uint8_t D1CorePrescTable[16] = {0, 0, 0, 0, 1, 2, 3, 4, 1, 2, 3, 4, 6, 7, 8, 9};
I2C_HandleTypeDef hi2c2;
bme280_dev_t bme280;
bme280_meas_t meas;

/**
 * @brief  Setup the microcontroller system
 *         Initialize the FPU setting and  vector table location
 *         configuration.
 * @param  None
 * @retval None
 */
void SystemInit(void) {
#if defined(DATA_IN_D2_SRAM)
  __IO uint32_t tmpreg;
#endif /* DATA_IN_D2_SRAM */

/* FPU settings ------------------------------------------------------------*/
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
  SCB->CPACR |= ((3UL << (10 * 2)) | (3UL << (11 * 2))); /* set CP10 and CP11 Full Access */
#endif
  /* Reset the RCC clock configuration to the default reset state ------------*/

  /* Increasing the CPU frequency */
  if (FLASH_LATENCY_DEFAULT > (READ_BIT((FLASH->ACR), FLASH_ACR_LATENCY))) {
    /* Program the new number of wait states to the LATENCY bits in the FLASH_ACR register */
    MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, (uint32_t)(FLASH_LATENCY_DEFAULT));
  }

  /* Set HSION bit */
  RCC->CR |= RCC_CR_HSION;

  /* Reset CFGR register */
  RCC->CFGR = 0x00000000;

  /* Reset HSEON, HSECSSON, CSION, HSI48ON, CSIKERON, PLL1ON, PLL2ON and PLL3ON bits */
  RCC->CR &= 0xEAF6ED7FU;

  /* Decreasing the number of wait states because of lower CPU frequency */
  if (FLASH_LATENCY_DEFAULT < (READ_BIT((FLASH->ACR), FLASH_ACR_LATENCY))) {
    /* Program the new number of wait states to the LATENCY bits in the FLASH_ACR register */
    MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, (uint32_t)(FLASH_LATENCY_DEFAULT));
  }

#if defined(D3_SRAM_BASE)
  /* Reset D1CFGR register */
  RCC->D1CFGR = 0x00000000;

  /* Reset D2CFGR register */
  RCC->D2CFGR = 0x00000000;

  /* Reset D3CFGR register */
  RCC->D3CFGR = 0x00000000;
#else
  /* Reset CDCFGR1 register */
  RCC->CDCFGR1 = 0x00000000;

  /* Reset CDCFGR2 register */
  RCC->CDCFGR2 = 0x00000000;

  /* Reset SRDCFGR register */
  RCC->SRDCFGR = 0x00000000;
#endif
  /* Reset PLLCKSELR register */
  RCC->PLLCKSELR = 0x02020200;

  /* Reset PLLCFGR register */
  RCC->PLLCFGR = 0x01FF0000;
  /* Reset PLL1DIVR register */
  RCC->PLL1DIVR = 0x01010280;
  /* Reset PLL1FRACR register */
  RCC->PLL1FRACR = 0x00000000;

  /* Reset PLL2DIVR register */
  RCC->PLL2DIVR = 0x01010280;

  /* Reset PLL2FRACR register */

  RCC->PLL2FRACR = 0x00000000;
  /* Reset PLL3DIVR register */
  RCC->PLL3DIVR = 0x01010280;

  /* Reset PLL3FRACR register */
  RCC->PLL3FRACR = 0x00000000;

  /* Reset HSEBYP bit */
  RCC->CR &= 0xFFFBFFFFU;

  /* Disable all interrupts */
  RCC->CIER = 0x00000000;

#if (STM32H7_DEV_ID == 0x450UL)
  /* dual core CM7 or single core line */
  if ((DBGMCU->IDCODE & 0xFFFF0000U) < 0x20000000U) {
    /* if stm32h7 revY*/
    /* Change  the switch matrix read issuing capability to 1 for the AXI SRAM target (Target 7) */
    *((__IO uint32_t *)0x51008108) = 0x000000001U;
  }
#endif /* STM32H7_DEV_ID */

#if defined(DATA_IN_D2_SRAM)
  /* in case of initialized data in D2 SRAM (AHB SRAM), enable the D2 SRAM clock (AHB SRAM clock) */
#if defined(RCC_AHB2ENR_D2SRAM3EN)
  RCC->AHB2ENR |= (RCC_AHB2ENR_D2SRAM1EN | RCC_AHB2ENR_D2SRAM2EN | RCC_AHB2ENR_D2SRAM3EN);
#elif defined(RCC_AHB2ENR_D2SRAM2EN)
  RCC->AHB2ENR |= (RCC_AHB2ENR_D2SRAM1EN | RCC_AHB2ENR_D2SRAM2EN);
#else
  RCC->AHB2ENR |= (RCC_AHB2ENR_AHBSRAM1EN | RCC_AHB2ENR_AHBSRAM2EN);
#endif /* RCC_AHB2ENR_D2SRAM3EN */

  tmpreg = RCC->AHB2ENR;
  (void)tmpreg;
#endif /* DATA_IN_D2_SRAM */

#if defined(DUAL_CORE) && defined(CORE_CM4)
  /* Configure the Vector Table location add offset address for cortex-M4 ------------------*/
#if defined(USER_VECT_TAB_ADDRESS)
  SCB->VTOR = VECT_TAB_BASE_ADDRESS | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal D2 AXI-RAM or in Internal FLASH */
#endif                                                 /* USER_VECT_TAB_ADDRESS */

#else
  /*
   * Disable the FMC bank1 (enabled after reset).
   * This, prevents CPU speculation access on this bank which blocks the use of FMC during
   * 24us. During this time the others FMC master (such as LTDC) cannot use it!
   */
  FMC_Bank1_R->BTCR[0] = 0x000030D2;

  /* Configure the Vector Table location -------------------------------------*/
#if defined(USER_VECT_TAB_ADDRESS)
  SCB->VTOR = VECT_TAB_BASE_ADDRESS | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal D1 AXI-RAM or in Internal FLASH */
#endif /* USER_VECT_TAB_ADDRESS */

#endif /*DUAL_CORE && CORE_CM4*/
}

/**
 * @brief  Update SystemCoreClock variable according to Clock Register Values.
 *         The SystemCoreClock variable contains the core clock , it can
 *         be used by the user application to setup the SysTick timer or configure
 *         other parameters.
 *
 * @note   Each time the core clock changes, this function must be called
 *         to update SystemCoreClock variable value. Otherwise, any configuration
 *         based on this variable will be incorrect.
 *
 * @note   - The system frequency computed by this function is not the real
 *           frequency in the chip. It is calculated based on the predefined
 *           constant and the selected clock source:
 *
 *           - If SYSCLK source is CSI, SystemCoreClock will contain the CSI_VALUE(*)
 *           - If SYSCLK source is HSI, SystemCoreClock will contain the HSI_VALUE(**)
 *           - If SYSCLK source is HSE, SystemCoreClock will contain the HSE_VALUE(***)
 *           - If SYSCLK source is PLL, SystemCoreClock will contain the CSI_VALUE(*),
 *             HSI_VALUE(**) or HSE_VALUE(***) multiplied/divided by the PLL factors.
 *
 *         (*) CSI_VALUE is a constant defined in stm32h7xx_hal.h file (default value
 *             4 MHz) but the real value may vary depending on the variations
 *             in voltage and temperature.
 *         (**) HSI_VALUE is a constant defined in stm32h7xx_hal.h file (default value
 *             64 MHz) but the real value may vary depending on the variations
 *             in voltage and temperature.
 *
 *         (***)HSE_VALUE is a constant defined in stm32h7xx_hal.h file (default value
 *              25 MHz), user has to ensure that HSE_VALUE is same as the real
 *              frequency of the crystal used. Otherwise, this function may
 *              have wrong result.
 *
 *         - The result of this function could be not correct when using fractional
 *           value for HSE crystal.
 * @param  None
 * @retval None
 */
void SystemCoreClockUpdate(void) {
  uint32_t pllp, pllsource, pllm, pllfracen, hsivalue, tmp;
  uint32_t common_system_clock;
  float_t fracn1, pllvco;

  /* Get SYSCLK source -------------------------------------------------------*/

  switch (RCC->CFGR & RCC_CFGR_SWS) {
    case RCC_CFGR_SWS_HSI: /* HSI used as system clock source */
      common_system_clock = (uint32_t)(HSI_VALUE >> ((RCC->CR & RCC_CR_HSIDIV) >> 3));
      break;

    case RCC_CFGR_SWS_CSI: /* CSI used as system clock  source */
      common_system_clock = CSI_VALUE;
      break;

    case RCC_CFGR_SWS_HSE: /* HSE used as system clock  source */
      common_system_clock = HSE_VALUE;
      break;

    case RCC_CFGR_SWS_PLL1: /* PLL1 used as system clock  source */

      /* PLL_VCO = (HSE_VALUE or HSI_VALUE or CSI_VALUE/ PLLM) * PLLN
      SYSCLK = PLL_VCO / PLLR
      */
      pllsource = (RCC->PLLCKSELR & RCC_PLLCKSELR_PLLSRC);
      pllm = ((RCC->PLLCKSELR & RCC_PLLCKSELR_DIVM1) >> 4);
      pllfracen = ((RCC->PLLCFGR & RCC_PLLCFGR_PLL1FRACEN) >> RCC_PLLCFGR_PLL1FRACEN_Pos);
      fracn1 = (float_t)(uint32_t)(pllfracen * ((RCC->PLL1FRACR & RCC_PLL1FRACR_FRACN1) >> 3));

      if (pllm != 0U) {
        switch (pllsource) {
          case RCC_PLLCKSELR_PLLSRC_HSI: /* HSI used as PLL clock source */

            hsivalue = (HSI_VALUE >> ((RCC->CR & RCC_CR_HSIDIV) >> 3));
            pllvco = ((float_t)hsivalue / (float_t)pllm) * ((float_t)(uint32_t)(RCC->PLL1DIVR & RCC_PLL1DIVR_N1) + (fracn1 / (float_t)0x2000) + (float_t)1);

            break;

          case RCC_PLLCKSELR_PLLSRC_CSI: /* CSI used as PLL clock source */
            pllvco = ((float_t)CSI_VALUE / (float_t)pllm) * ((float_t)(uint32_t)(RCC->PLL1DIVR & RCC_PLL1DIVR_N1) + (fracn1 / (float_t)0x2000) + (float_t)1);
            break;

          case RCC_PLLCKSELR_PLLSRC_HSE: /* HSE used as PLL clock source */
            pllvco = ((float_t)HSE_VALUE / (float_t)pllm) * ((float_t)(uint32_t)(RCC->PLL1DIVR & RCC_PLL1DIVR_N1) + (fracn1 / (float_t)0x2000) + (float_t)1);
            break;

          default:
            hsivalue = (HSI_VALUE >> ((RCC->CR & RCC_CR_HSIDIV) >> 3));
            pllvco = ((float_t)hsivalue / (float_t)pllm) * ((float_t)(uint32_t)(RCC->PLL1DIVR & RCC_PLL1DIVR_N1) + (fracn1 / (float_t)0x2000) + (float_t)1);
            break;
        }
        pllp = (((RCC->PLL1DIVR & RCC_PLL1DIVR_P1) >> 9) + 1U);
        common_system_clock = (uint32_t)(float_t)(pllvco / (float_t)pllp);
      } else {
        common_system_clock = 0U;
      }
      break;

    default:
      common_system_clock = (uint32_t)(HSI_VALUE >> ((RCC->CR & RCC_CR_HSIDIV) >> 3));
      break;
  }

    /* Compute SystemClock frequency --------------------------------------------------*/
#if defined(RCC_D1CFGR_D1CPRE)
  tmp = D1CorePrescTable[(RCC->D1CFGR & RCC_D1CFGR_D1CPRE) >> RCC_D1CFGR_D1CPRE_Pos];

  /* common_system_clock frequency : CM7 CPU frequency  */
  common_system_clock >>= tmp;

  /* SystemD2Clock frequency : CM4 CPU, AXI and AHBs Clock frequency  */
  SystemD2Clock = (common_system_clock >> ((D1CorePrescTable[(RCC->D1CFGR & RCC_D1CFGR_HPRE) >> RCC_D1CFGR_HPRE_Pos]) & 0x1FU));

#else
  tmp = D1CorePrescTable[(RCC->CDCFGR1 & RCC_CDCFGR1_CDCPRE) >> RCC_CDCFGR1_CDCPRE_Pos];

  /* common_system_clock frequency : CM7 CPU frequency  */
  common_system_clock >>= tmp;

  /* SystemD2Clock frequency : AXI and AHBs Clock frequency  */
  SystemD2Clock = (common_system_clock >> ((D1CorePrescTable[(RCC->CDCFGR1 & RCC_CDCFGR1_HPRE) >> RCC_CDCFGR1_HPRE_Pos]) & 0x1FU));

#endif

#if defined(DUAL_CORE) && defined(CORE_CM4)
  SystemCoreClock = SystemD2Clock;
#else
  SystemCoreClock = common_system_clock;
#endif /* DUAL_CORE && CORE_CM4 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
  __disable_irq();
  while (1) {
  }
}

/**
 * @brief This function handles System tick timer.
 */
void SysTick_Handler(void) { HAL_IncTick(); }

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
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if (hi2c->Instance == I2C2) {
    /** Initializes the peripherals clock
     */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2C2;
    PeriphClkInitStruct.I2c123ClockSelection = RCC_I2C1235CLKSOURCE_D2PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
      Error_Handler();
    }
    __HAL_RCC_GPIOF_CLK_ENABLE();
    /**I2C2 GPIO Configuration
    PF0     ------> I2C2_SDA
    PF1     ------> I2C2_SCL
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
    /* Peripheral clock enable */
    __HAL_RCC_I2C2_CLK_ENABLE();
  }
}

/**
 * @brief I2C MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hi2c: I2C handle pointer
 * @retval None
 */
void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c) {
  if (hi2c->Instance == I2C2) {
    /* Peripheral clock disable */
    __HAL_RCC_I2C2_CLK_DISABLE();
    /**I2C2 GPIO Configuration
    PF0     ------> I2C2_SDA
    PF1     ------> I2C2_SCL
    */
    HAL_GPIO_DeInit(GPIOF, GPIO_PIN_0);
    HAL_GPIO_DeInit(GPIOF, GPIO_PIN_1);
  }
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  /** Supply configuration update enable
   */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
  /** Configure the main internal regulator output voltage
   */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {
  }
  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = 64;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
    Error_Handler();
  }
}

/**
 * @brief I2C2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C2_Init(void) {
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x00707CBB;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK) {
    Error_Handler();
  }
  /** Configure Analogue filter
   */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK) {
    Error_Handler();
  }
  /** Configure Digital filter
   */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK) {
    Error_Handler();
  }
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
  __HAL_RCC_GPIOF_CLK_ENABLE();
}

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_I2C2_Init();
  bme280.i2c = hi2c2;
  bme280_init(&bme280);
  while (1) {
    bme280_read(&bme280, &meas);
    HAL_Delay(100);
  }
}
