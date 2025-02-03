/**
 * @file hsm.h
 * @brief Hierarchical State Machine (HSM)
 * @date 2025-01
 *
 * @copyright Copyright © 2025 dronectl
 */

#ifndef __HSM_H__
#define __HSM_H__

#include "dtc.h"
#include "led.h"
#include "system.h"

#include <stdint.h>
#include <FreeRTOS.h>
#include <queue.h>

#define HSM_DEFAULT_TICK_RATE_MS 10
#define HSM_EVENT_QUEUE_SIZE 5 * sizeof(enum hsm_event)

enum hsm_led_id {
  HSM_LED_ID_ERROR,
  HSM_LED_ID_IDLE,
  HSM_LED_ID_RUN,
  
  HSM_LED_ID_COUNT
};

/**
 * @brief HSM External status codes
 * 
 */
enum hsm_status {
  HSM_STATUS_OK,
  HSM_STATUS_EVE_QUEUE_FULL,
  HSM_STATUS_COUNT,
};

/**
 * @brief HSM events
 */
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

/**
 * @brief HSM states
 */
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

struct hsm_init_context {
  const struct led_init_context led_init_ctx[HSM_LED_ID_COUNT];
  const size_t num_led_init_ctx;
};

struct hsm_context {
  enum hsm_state current_state;
  enum hsm_state next_state;
  enum DTCID pending_dtc;
  uint32_t enter_timestamp;
  uint32_t exit_timestamp;
  uint32_t hsm_tick_rate_ms;
  uint8_t event_queue_buffer[HSM_EVENT_QUEUE_SIZE];
  TaskHandle_t task_handle;
  StaticQueue_t event_queue_ctrl;
  QueueHandle_t event_queue;
  struct led_context led_ctx[HSM_LED_ID_COUNT];
};

/**
 * @brief Initialize and spawn the HSM process
 *
 * @param[in] task_ctx task initialization context
 */
void hsm_start(const struct system_task_context *task_ctx);

/**
 * @brief Get the current state of the HSM
 *
 * @return current state
 */
enum hsm_state hsm_get_current_state(void);

/**
 * @brief Post an event to the HSM event queue
 * 
 * @warning not ISR safe -> use `hsm_post_event_isr`
 * @param[in] event HSM event pointer
 * @param[in] wait_ms millis to block waiting for queue space.
 * @return hsm status code
 */
enum hsm_status hsm_post_event(const enum hsm_event *event, const uint16_t wait_ms);

/**
 * @brief ISR safe post to HSM event queue
 * 
 * @param[in] event HSM event pointer
 * @param[out] req_ctx_switch caller is required to perform a context switch
 * @return hsm status code
 */
enum hsm_status hsm_post_event_isr(const enum hsm_event *event, bool* req_ctx_switch);

#ifdef UNITTEST
struct hsm_context *test_hsm_get_context(void);
TaskFunction_t test_hsm_get_main(void);
void test_hsm_service_event_queue(void);
void test_hsm_enter_state(void);
void test_hsm_exit_state(void);
#endif // UNITTEST

#endif // __HSM_H__
