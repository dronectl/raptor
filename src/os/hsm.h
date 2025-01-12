/**
 * @file hsm.h
 * @brief Hierarchical State Machine (HSM) for raptor
 * @date 2025-01
 *
 * @copyright Copyright © 2025 Christian Sargusingh
 */

#include <stdint.h>
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
  HSM_EVENT_ERROR,
  HSM_EVENT_CALIBRATION,
  HSM_EVENT_COUNT
};

enum HsmState {
  // root states
  HSM_STATE_RESET = 0,
  HSM_STATE_INIT = 1,
  HSM_STATE_IDLE = 2,
  HSM_STATE_RUN = 3,
  HSM_STATE_ERROR = 4,
  HSM_STATE_CALIBRATION = 5,

  // run substates
  HSM_STATE_RUN_ENABLE_PWR,
  HSM_STATE_RUN_ARM_ESC,
  HSM_STATE_RUN_PROFILE,

  // calibration substates
  HSM_STATE_CALIBRATION_TRIM,
  HSM_STATE_CALIBRATION_SCALE,

  HSM_STATE_COUNT
};

struct hsm_ctx {
  enum HsmState current_state;
  enum HsmState next_state;
  enum HsmEvent pending_event;
  uint32_t enter_timestamp;
  struct led_handle leds[HSM_LED_COUNT];
  struct esc_engine_ctx *esc_engine_ctx;
  struct power_manager_ctx *power_manager_ctx;
} _hsm_ctx;

struct hsm_state_table {
  enum HsmState event;
  void (*enter)(struct hsm_ctx *ctx);
  enum HsmState (*run)(struct hsm_ctx *ctx);
  void (*exit)(struct hsm_ctx *ctx);
} _hsm_state_table;

void hsm_post_event(struct hsm_ctx *ctx, enum HsmEvent event);
