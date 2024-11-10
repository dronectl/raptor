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
#ifndef __PWM_H__
#define __PWM_H__

#include <stdbool.h>
#include <stm32h7xx_hal.h>
#include <string.h>
/**
 * @brief Error codes
 *
 */
typedef int pwm_status_t;
#define PWM_OK (pwm_status_t)0
#define PWM_ERR (pwm_status_t)1

typedef enum pwm_oc_modes_t {
  PWM_OC_DEFAULT = 0,
  PWM_OC_COMBINDED = 1,
  PWM_OC_ASYMETRIC = 2,
} pwm_oc_modes_t;

/**
 * @brief PWM params
 *
 */
typedef struct pwm_t {
  uint32_t period;    // raw
  uint32_t prescaler; // raw
  double duty_cycle;  // %
  pwm_oc_modes_t tim_oc_mode;
  bool polarity; // High Polarity = True Low and Polarity = False
} pwm_t;

pwm_status_t pwm_tim_channel_1_init(TIM_HandleTypeDef *htimer, TIM_OC_InitTypeDef *tim_output_cmp_cfg, pwm_t *pwm_init_params);

#endif // __PWM_H__
