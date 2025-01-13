/**
 * @file hsm.h
 * @brief Hierarchical State Machine (HSM) for raptor
 * @date 2025-01
 *
 * @copyright Copyright © 2025 Christian Sargusingh
 */

#ifndef __HSM_H__
#define __HSM_H__



enum hsm_event {
  HSM_EVENT_NONE = 0,
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
  HSM_STATE_ERROR,
  HSM_STATE_CALIBRATION,

  // run substates
  HSM_STATE_RUN_STARTUP,
  HSM_STATE_RUN_PROFILE,
  HSM_STATE_RUN_SHUTDOWN,

  // calibration substates
  HSM_STATE_CALIBRATION_TRIM,
  HSM_STATE_CALIBRATION_SCALE,

  HSM_STATE_COUNT
};

enum hsm_state hsm_get_current_state(void);
void hsm_post_event(const enum hsm_event event);

#endif // __HSM_H__
