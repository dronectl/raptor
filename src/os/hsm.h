/**
 * @file hsm.h
 * @brief Hierarchical State Machine (HSM)
 * @date 2025-01
 *
 * @copyright Copyright © 2025 dronectl
 */

#ifndef __HSM_H__
#define __HSM_H__

enum hsm_led_id {
  HSM_LED_ID_ERROR = 0,
  HSM_LED_ID_IDLE,
  HSM_LED_ID_RUN,
  HSM_LED_ID_COUNT
};

enum hsm_event {
  HSM_EVENT_NONE = 0,
  HSM_EVENT_SOFT_RESET,
  HSM_EVENT_HARD_RESET,
  HSM_EVENT_RUN,
  HSM_EVENT_STOP,
  HSM_EVENT_ABORT,
  HSM_EVENT_CLEAR_ERROR,
  HSM_EVENT_CALIBRATION,
  HSM_EVENT_COUNT
};

enum hsm_state {
  // root states
  HSM_STATE_ROOT,
  HSM_STATE_RESET,
  HSM_STATE_INIT,
  HSM_STATE_IDLE,
  HSM_STATE_RUN,
  HSM_STATE_START,
  HSM_STATE_STOP,
  HSM_STATE_ERROR,
  HSM_STATE_CALIBRATION,

  // run substates
  HSM_STATE_RUN_STARTUP,
  HSM_STATE_RUN_PROFILE,

  // calibration substates
  HSM_STATE_CALIBRATION_TRIM,
  HSM_STATE_CALIBRATION_SCALE,

  HSM_STATE_COUNT
};

struct hsm_init_params {
  struct led_ctx *led_ctxs[HSM_LED_ID_COUNT];
};

void hsm_init(const struct hsm_init_params *init_params);
enum hsm_state hsm_get_current_state(void);
void hsm_post_event(const enum hsm_event event);

#endif // __HSM_H__
