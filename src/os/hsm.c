/**
 * @file hsm.c
 * @brief Hierarchical state machine
 * @date 2025-01
 *
 * @copyright Copyright © 2025 dronectl
 */

#include "hsm.h"
#include "logger.h"
#include "esc_engine.h"
#include "uassert.h"
#include "power_manager.h"

#include <string.h>
#include <stdlib.h>
#include <task.h>

#define IDLE_LED_TOGGLE_RATE_MS 1000
#define RUN_LED_TOGGLE_RATE_MS 500

enum event_handle_result {
  EVENT_UNHANDLED = 0,
  EVENT_HANDLED = 1,
};

struct state_table_entry {
  enum hsm_state parent;
  void (*enter)(void);
  void (*tick)(void);
  void (*exit)(void);
  enum event_handle_result (*handle_event)(const enum hsm_event event);
};

/* raptor state handlers */
static enum event_handle_result handle_event_root(const enum hsm_event event);

// reset state callbacks
static void enter_reset(void);
static void tick_reset(void);
static void exit_reset(void);

// init state callbacks
static void tick_init(void);

// idle state callbacks
static void enter_idle(void);
static void tick_idle(void);
static void exit_idle(void);
static enum event_handle_result handle_event_idle(const enum hsm_event event);

// error state callbacks
static void enter_error(void);
static void tick_error(void);
static void exit_error(void);
static enum event_handle_result handle_event_error(const enum hsm_event event);

// run state callbacks
static void enter_run(void);
static void tick_run(void);
static void exit_run(void);
static enum event_handle_result handle_event_run(const enum hsm_event event);

// run startup state callbacks
static void enter_run_startup(void);
static void tick_run_startup(void);
static void exit_run_startup(void);
static enum event_handle_result handle_event_run_startup(const enum hsm_event event);

// run startup state callbacks
static void enter_run_profile(void);
static void tick_run_profile(void);
static void exit_run_profile(void);
static enum event_handle_result handle_event_run_profile(const enum hsm_event event);

// stop state callbacks
static void enter_stop(void);
static void tick_stop(void);
static void exit_stop(void);
static enum event_handle_result handle_event_stop(const enum hsm_event event);

// calibration state callbacks
static void enter_calibration(void);
static void tick_calibration(void);
static void exit_calibration(void);
static enum event_handle_result handle_event_calibration(const enum hsm_event event);

// static hsm context
static struct hsm_context ctx = {0};

static const struct state_table_entry state_table[HSM_STATE_COUNT] = {
    [HSM_STATE_ROOT] = { .parent = HSM_STATE_ROOT, .enter = NULL, .tick = NULL, .exit = NULL, .handle_event = handle_event_root },
    [HSM_STATE_RESET] = { .parent = HSM_STATE_ROOT, .enter = enter_reset, .tick = tick_reset, .exit = exit_reset, .handle_event = NULL },
    [HSM_STATE_INIT] = { .parent = HSM_STATE_ROOT, .enter = NULL, .tick = tick_init, .exit = NULL, .handle_event = NULL },
    [HSM_STATE_IDLE] = { .parent = HSM_STATE_ROOT, .enter = enter_idle, .tick = tick_idle, .exit = exit_idle, .handle_event = handle_event_idle },

    [HSM_STATE_RUN] = { .parent = HSM_STATE_ROOT, .enter = enter_run, .tick = tick_run, .exit = exit_run, .handle_event = handle_event_run },
    [HSM_STATE_RUN_STARTUP] = { .parent = HSM_STATE_RUN, .enter = enter_run_startup, .tick = tick_run_startup, .exit = exit_run_startup, .handle_event = handle_event_run_startup },
    [HSM_STATE_RUN_PROFILE] = { .parent = HSM_STATE_RUN, .enter = enter_run_profile, .tick = tick_run_profile, .exit = exit_run_profile, .handle_event = handle_event_run_profile },

    [HSM_STATE_STOP] = { .parent = HSM_STATE_ROOT, .enter = enter_stop, .tick = tick_stop, .exit = exit_stop, .handle_event = handle_event_stop },
    [HSM_STATE_ERROR] = { .parent = HSM_STATE_ROOT, .enter = enter_error, .tick = tick_error, .exit = exit_error, .handle_event = handle_event_error },

    [HSM_STATE_CALIBRATION] = { .parent = HSM_STATE_ROOT, .enter = enter_calibration, .tick = tick_calibration, .exit = exit_calibration, .handle_event = handle_event_calibration },
};

// root state handlers

static enum event_handle_result handle_event_root(const enum hsm_event event) {
  enum event_handle_result result = EVENT_UNHANDLED;
  switch (event) {
    default:
      warning("Unhandled event %d in state %d", event, ctx.current_state);
      dtc_post_event(DTCID_HSM_UNHANDLED_EVENT);
      result = EVENT_HANDLED;
      break;
  }
  return result;
}

// reset state handlers

static void tick_init(void) {
  power_manager_init(NULL);
  esc_engine_init(NULL);
  ctx.next_state = HSM_STATE_IDLE;
}

// reset state handlers

static void enter_reset(void) {
  info("HSM Reset\n");
  struct led_context *led_ctx = &ctx.led_ctx[0];
  for (; led_ctx < &ctx.led_ctx[0] + HSM_LED_ID_COUNT; led_ctx++) {
    led_disable(led_ctx);
  }
}

static void tick_reset(void) {
  struct led_context *led_ctx = &ctx.led_ctx[0];
  for (; led_ctx < &ctx.led_ctx[0] + HSM_LED_ID_COUNT; led_ctx++) {
    led_toggle(led_ctx);
  }
  vTaskDelay(500);
  for (; led_ctx < &ctx.led_ctx[0] + HSM_LED_ID_COUNT; led_ctx++) {
    led_toggle(led_ctx);
  }
  // todo: perform calibration
  ctx.next_state = HSM_STATE_INIT;
}

static void exit_reset(void) {
  struct led_context *led_ctx = &ctx.led_ctx[0];
  for (; led_ctx < &ctx.led_ctx[0] + HSM_LED_ID_COUNT; led_ctx++) {
    led_disable(led_ctx);
  }
}

// idle state handlers

static void enter_idle(void) {
  info("HSM entering idle\n");
  led_enable(&ctx.led_ctx[HSM_LED_ID_IDLE]);
}

static void tick_idle(void) {
  led_periodic_toggle(&ctx.led_ctx[HSM_LED_ID_IDLE], IDLE_LED_TOGGLE_RATE_MS);
}

static void exit_idle(void) {
  led_disable(&ctx.led_ctx[HSM_LED_ID_IDLE]);
}

static enum event_handle_result handle_event_idle(const enum hsm_event event) {
  enum event_handle_result result = EVENT_UNHANDLED;
  switch (event) {
    case HSM_EVENT_RUN:
      ctx.next_state = HSM_STATE_START;
      result = EVENT_HANDLED;
      break;
    case HSM_EVENT_CALIBRATION:
      ctx.next_state = HSM_STATE_CALIBRATION;
      result = EVENT_HANDLED;
      break;
    default:
      break;
  }
  return result;
}

// run state handlers

static void enter_run(void) {
  info("HSM entering run\n");
  led_enable(&ctx.led_ctx[HSM_LED_ID_RUN]);
}

static void tick_run(void) {
  led_toggle(&ctx.led_ctx[HSM_LED_ID_RUN]);
}

static void exit_run(void) {
  led_disable(&ctx.led_ctx[HSM_LED_ID_RUN]);
}

static enum event_handle_result handle_event_run(const enum hsm_event event) {
  enum event_handle_result result = EVENT_UNHANDLED;
  switch (event) {
    case HSM_EVENT_ABORT:
    case HSM_EVENT_STOP:
      ctx.next_state = HSM_STATE_STOP;
      result = EVENT_HANDLED;
      break;
    default:
      break;
  }
  return result;
}

// run startup state handlers

static void enter_run_startup(void) {}

static void tick_run_startup(void) {
  ctx.next_state = HSM_STATE_RUN_PROFILE;
}

static void exit_run_startup(void) {}

static enum event_handle_result handle_event_run_startup(const enum hsm_event event) {
  return EVENT_UNHANDLED;
}

// run profile state handlers

static void enter_run_profile(void) {}

static void tick_run_profile(void) {}

static void exit_run_profile(void) {}

static enum event_handle_result handle_event_run_profile(const enum hsm_event event) {
  return EVENT_UNHANDLED;
}

// stop state handlers

static void enter_stop(void) {}

static void tick_stop(void) {
  if (ctx.pending_dtc != DTCID_NONE) {
    ctx.next_state = HSM_STATE_ERROR;
  } else {
    ctx.next_state = HSM_STATE_IDLE;
  }
}

static void exit_stop(void) {}

static enum event_handle_result handle_event_stop(const enum hsm_event event) {
  return EVENT_UNHANDLED;
}

// error state handlers

static void enter_error(void) {
  if (ctx.pending_dtc != DTCID_NONE) {
    dtc_post_event(ctx.pending_dtc);
    ctx.pending_dtc = DTCID_NONE;
  }
  led_enable(&ctx.led_ctx[HSM_LED_ID_ERROR]);
}

static void tick_error(void) {
  led_toggle(&ctx.led_ctx[HSM_LED_ID_ERROR]);
}

static void exit_error(void) {
  led_disable(&ctx.led_ctx[HSM_LED_ID_ERROR]);
}

static enum event_handle_result handle_event_error(const enum hsm_event event) {
  enum event_handle_result result = EVENT_UNHANDLED;
  switch (event) {
    case HSM_EVENT_CLEAR_ERROR:
      ctx.next_state = HSM_STATE_IDLE;
      result = EVENT_HANDLED;
      break;
    default:
      break;
  }
  return result;
}

// calibration state handlers

static void enter_calibration(void) {}

static void tick_calibration(void) {}

static void exit_calibration(void) {}

static enum event_handle_result handle_event_calibration(const enum hsm_event event) {
  enum event_handle_result result = EVENT_UNHANDLED;
  switch (event) {
    case HSM_EVENT_ABORT:
    case HSM_EVENT_STOP:
      ctx.next_state = HSM_STATE_IDLE;
      result = EVENT_HANDLED;
      break;
    default:
      break;
  }
  return result;
}

// static functions

static void service_event_queue(void) {
  enum hsm_state current_state = ctx.current_state;
  enum hsm_event event = HSM_EVENT_NONE;
  BaseType_t status = xQueueReceive(ctx.event_queue, &event, 0);
  if (status == pdFALSE) {
    return;
  }
  while (current_state != HSM_STATE_ROOT) {
    const struct state_table_entry *state = &state_table[current_state];
    if (state->handle_event != NULL) {
      enum event_handle_result result = state->handle_event(event);
      if (result == EVENT_HANDLED) {
        break;
      }
    }
    current_state = state->parent;
  }
}

static void exit_state(void) {
  enum hsm_state state_iterator = ctx.current_state;
  while (state_iterator != HSM_STATE_ROOT) {
    const struct state_table_entry *state = &state_table[state_iterator];
    if (state->exit != NULL) {
      state->exit();
      ctx.exit_timestamp = HAL_GetTick();
    }
    state_iterator = state->parent;
  }
}

static void enter_state(void) {
  ctx.current_state = ctx.next_state;
  enum hsm_state state_iterator = ctx.current_state;
  while (state_iterator != HSM_STATE_ROOT) {
    const struct state_table_entry *state = &state_table[state_iterator];
    if (state->enter != NULL) {
      ctx.enter_timestamp = HAL_GetTick();
      state->enter();
    }
    state_iterator = state->parent;
  }
}

static void hsm_main(void* __attribute__((unused)) argument) {
  info("Starting HSM\n");
  while (1) {
    service_event_queue();
    if (ctx.current_state != ctx.next_state) {
      exit_state();
      enter_state();
    }
    enum hsm_state current_state = ctx.current_state;
    while (current_state != HSM_STATE_ROOT) {
      const struct state_table_entry *state = &state_table[current_state];
      if (state->tick != NULL) {
        state->tick();
      }
      current_state = state->parent;
    }
    vTaskDelay(ctx.hsm_tick_rate_ms);
  }
}

enum hsm_status hsm_post_event_isr(const enum hsm_event *event, bool* req_ctx_switch) {
  enum hsm_status status = HSM_STATUS_EVE_QUEUE_FULL;
  BaseType_t ctx_switch = pdFALSE;
  uassert(event != NULL);
  uassert(req_ctx_switch != NULL);
  *req_ctx_switch = false;
  BaseType_t code = xQueueSendFromISR(ctx.event_queue, event, &ctx_switch);
  if (ctx_switch == pdTRUE) {
    *req_ctx_switch = true;
  }
  if (code == pdTRUE) {
    status = HSM_STATUS_OK;
  }
  return status;
}

enum hsm_status hsm_post_event(const enum hsm_event *event, const uint16_t wait_ms) {
  enum hsm_status status = HSM_STATUS_EVE_QUEUE_FULL;
  uassert(event != NULL);
  BaseType_t resp = xQueueSend(ctx.event_queue, event, pdMS_TO_TICKS(wait_ms));
  if (resp == pdTRUE) {
    info("posted <%i> to HSM event queue\n", event);
    status = HSM_STATUS_OK;
  } else {
    warning("failed to post <%i> to HSM event queue waiting: %u ms\n", event, wait_ms);
  }
  return status;
}

enum hsm_state hsm_get_current_state(void) {
  return ctx.current_state;
}

void hsm_start(const struct system_task_context *task_ctx) {
  // header guards
  uassert(task_ctx != NULL);
  uassert(task_ctx->init_ctx != NULL);

  // populate static context
  const struct hsm_init_context* init = (const struct hsm_init_context *)task_ctx->init_ctx;
  uassert(init->led_init_ctx != NULL);
  for (uint8_t i = 0; i < init->num_led_init_ctx; i++) {
    led_init(&ctx.led_ctx[i], &init->led_init_ctx[i]);
  }
  ctx.current_state = HSM_STATE_RESET;
  ctx.hsm_tick_rate_ms = HSM_DEFAULT_TICK_RATE_MS;
  ctx.next_state = HSM_STATE_RESET;
  ctx.enter_timestamp = 0;
  ctx.event_queue = xQueueCreateStatic(sizeof(ctx.event_queue_buffer), sizeof(enum hsm_event), ctx.event_queue_buffer, &ctx.event_queue_ctrl);
  uassert(ctx.event_queue != NULL);

  // start task
  BaseType_t ret = xTaskCreate(hsm_main, task_ctx->name, task_ctx->stack_size, NULL, task_ctx->priority, &ctx.task_handle);
  uassert(ret == pdPASS);
}

#ifdef UNITTEST

struct hsm_context *test_hsm_get_context(void) {
  return &ctx;
}

TaskFunction_t test_hsm_get_main(void) {
  return (TaskFunction_t)hsm_main;
}

void test_hsm_service_event_queue(void) {
  service_event_queue();
}

void test_hsm_enter_state(void) {
  enter_state();
}

void test_hsm_exit_state(void) {
  exit_state();
}

#endif // UNITTEST
