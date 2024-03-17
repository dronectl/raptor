/**
 * @file pwm.c
 * @author raptor
 * @brief
 * @version 0.1
 * @date 2024-03
 *
 * @copyright Copyright © 2024 dronectl
 *
 */
#include "pwm.h"

/**
 * @brief GPIO Initialization Function for PWM pin
 * @param None
 * @retval None
 */
static void PWM_PE9_GPIO_Init(void) {
  // Configure GPIO pin for PWM
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOE_CLK_ENABLE();
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
}

/**
 * @brief Apply handle TIM and Output compare TIM config to channel 1 (PE9)
 */
pwm_status_t pwm_tim_channel_1_init(TIM_HandleTypeDef *htimer, TIM_OC_InitTypeDef *tim_output_cmp_cfg, pwm_t *pwm_init_params) {

  if (pwm_init_params->duty_cycle < 0 && pwm_init_params->duty_cycle > 100) {
    // duty cycle must be percentage
    return PWM_ERR;
  }

  memset(htimer, 0, sizeof(TIM_HandleTypeDef));

  __HAL_RCC_TIM1_CLK_ENABLE();
  htimer->Instance = TIM1;
  htimer->Init.Period = pwm_init_params->period;
  htimer->Init.Prescaler = pwm_init_params->prescaler;
  if (HAL_TIM_PWM_Init(htimer) != HAL_OK) {
    return PWM_ERR;
  }

  memset(tim_output_cmp_cfg, 0, sizeof(TIM_OC_InitTypeDef));

  if (pwm_init_params->tim_oc_mode == PWM_OC_DEFAULT) {
    tim_output_cmp_cfg->OCMode = TIM_OCMODE_PWM1;
  } else if (pwm_init_params->tim_oc_mode == PWM_OC_COMBINDED) {
    tim_output_cmp_cfg->OCMode = TIM_OCMODE_COMBINED_PWM1;
  } else if (pwm_init_params->tim_oc_mode == PWM_OC_ASYMETRIC) {
    tim_output_cmp_cfg->OCMode = TIM_OCMODE_ASSYMETRIC_PWM1;
  }

  tim_output_cmp_cfg->OCMode = TIM_OCMODE_PWM1;
  if (pwm_init_params->polarity == true) {
    tim_output_cmp_cfg->OCPolarity = TIM_OCPOLARITY_HIGH;
  } else {
    tim_output_cmp_cfg->OCPolarity = TIM_OCPOLARITY_LOW;
  }

  tim_output_cmp_cfg->Pulse = (htimer->Init.Period * pwm_init_params->duty_cycle) / 100;
  if (HAL_TIM_PWM_ConfigChannel(htimer, tim_output_cmp_cfg, TIM_CHANNEL_1) != HAL_OK) {
    return PWM_ERR;
  }
  PWM_PE9_GPIO_Init();
  if (HAL_TIM_PWM_Start(htimer, TIM_CHANNEL_1) != HAL_OK) {
    return PWM_ERR;
  }

  return PWM_OK;
}
