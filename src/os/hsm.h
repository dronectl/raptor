/**
 * @file hsm.h
 * @brief Hierarchical State Machine (HSM) for raptor
 * @date 2025-01
 *
 * @copyright Copyright © 2025 Christian Sargusingh
 */

#include <stdint.h>
#include "dtc.h"
#include "led.h"
#include "esc_engine.h"
#include "power_manager.h"

typedef enum HsmLedId {
  HSM_LED_IDLE = 0,
  HSM_LED_RUN,
  HSM_LED_ERROR,
  HSM_LED_COUNT
} HsmLedId;

enum HsmEvent {
  HSM_EVENT_NONE = 0,
  HSM_EVENT_RUN,
  HSM_EVENT_STOP,
  HSM_EVENT_ABORT,
  HSM_EVENT_CLEAR_ERROR,
  HSM_EVENT_CALIBRATION,
  HSM_EVENT_COUNT
};

enum HsmState {
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

struct hsm_ctx {
  enum HsmState current_state;
  enum HsmState next_state;
  enum HsmEvent pending_event;
  enum DTCID pending_dtc;
  uint32_t enter_timestamp;
  struct led_handle leds[HSM_LED_COUNT];
  struct esc_engine_ctx *esc_engine_ctx;
  struct power_manager_ctx *power_manager_ctx;
} _hsm_ctx;

struct hsm_state_table {
  enum HsmState event;
  enum HsmState parent;
  void (*enter)(struct hsm_ctx *ctx);
  void (*run)(struct hsm_ctx *ctx);
  void (*exit)(struct hsm_ctx *ctx);
  void (*handle_event)(struct hsm_ctx *ctx, const enum HsmEvent event);
} _hsm_state_table;

void hsm_post_event(struct hsm_ctx *ctx, enum HsmEvent event);
