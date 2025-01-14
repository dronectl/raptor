/**
 * @file hsm.c
 * @brief Hierarchical state machine
 * @date 2025-01
 *
 * @copyright Copyright © 2025 dronectl
 */

#include "hsm.h"
#include "logger.h"
#include "dtc.h"
#include "led.h"
#include "esc_engine.h"
#include "uassert.h"
#include "power_manager.h"

#include <stdlib.h>
#include <stdint.h>
#include <FreeRTOS.h>
#include <task.h>

enum led_id {
  LED_IDLE = 0,
  LED_RUN,
  LED_ERROR,
  LED_COUNT
};

enum event_handle_result {
  EVENT_UNHANDLED = 0,
  EVENT_HANDLED = 1,
};

struct hsm_ctx {
  enum hsm_state current_state;
  enum hsm_state next_state;
  enum DTCID pending_dtc;
  uint32_t enter_timestamp;
  struct led_handle leds[LED_COUNT];
} _hsm_ctx;

struct state_table {
  enum hsm_state state;
  enum hsm_state parent;
  void (*enter)(void);
  void (*tick)(void);
  void (*exit)(void);
  enum event_handle_result (*handle_event)(const enum hsm_event event);
} _hsm_state_table;


/* raptor state handlers */
static enum event_handle_result handle_event_root(const enum hsm_event event);

static void run_init(void);

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

static struct hsm_ctx ctx = {
  .current_state = HSM_STATE_RESET,
  .next_state = HSM_STATE_RESET,
  .enter_timestamp = 0,
  .leds = {
    [LED_IDLE] = {
      .port = GPIOB,
      .pin = GPIO_PIN_0,
      .active_high = 1,
    },
    [LED_RUN] = {
      .port = GPIOB,
      .pin = GPIO_PIN_1,
      .active_high = 1,
    },
    [LED_ERROR] = {
      .port = GPIOB,
      .pin = GPIO_PIN_2,
      .active_high = 1,
    }
  }
};

static const struct state_table state_table[HSM_STATE_COUNT] = {
  {HSM_STATE_ROOT, .parent = HSM_STATE_ROOT, .enter = NULL, .tick = NULL, .exit = NULL, .handle_event = handle_event_root},
  {HSM_STATE_INIT, .parent = HSM_STATE_ROOT, .enter = NULL, .tick = run_init, .exit = NULL, .handle_event = NULL},
  {HSM_STATE_IDLE, .parent = HSM_STATE_ROOT, .enter = enter_idle, .tick = tick_idle, .exit = exit_idle, .handle_event = handle_event_idle},

  {HSM_STATE_RUN, .parent = HSM_STATE_ROOT, .enter = enter_run, .tick = tick_run, .exit = exit_run, .handle_event = handle_event_run},
  {HSM_STATE_RUN_STARTUP, .parent = HSM_STATE_RUN, .enter = enter_run_startup, .tick = tick_run_startup, .exit = exit_run_startup, .handle_event = handle_event_run_startup},
  {HSM_STATE_RUN_PROFILE, .parent = HSM_STATE_RUN, .enter = enter_run_profile, .tick = tick_run_profile, .exit = exit_run_profile, .handle_event = handle_event_run_profile},

  {HSM_STATE_STOP, .parent = HSM_STATE_ROOT, .enter = enter_stop, .tick = tick_stop, .exit = exit_stop, .handle_event = handle_event_stop},
  {HSM_STATE_ERROR, .parent = HSM_STATE_ROOT, .enter = enter_error, .tick = tick_error, .exit = exit_error, .handle_event = handle_event_error},

  {HSM_STATE_CALIBRATION, .parent = HSM_STATE_ROOT, .enter = enter_calibration, .tick = tick_calibration, .exit = exit_calibration, .handle_event = handle_event_calibration},
};

static TaskHandle_t hsm_task_handle;

static enum event_handle_result handle_event_root(const enum hsm_event event){
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

static void run_init(void) {
  power_manager_init();
  esc_engine_init();
}

// idle state handlers

static void enter_idle(void) {
  led_enable(&ctx.leds[LED_IDLE]);
}

static void tick_idle(void) {
  led_toggle(&ctx.leds[LED_IDLE]);
}

static void exit_idle(void) {
  led_disable(&ctx.leds[LED_IDLE]);
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
  led_enable(&ctx.leds[LED_RUN]);
}

static void tick_run(void) {
  led_toggle(&ctx.leds[LED_RUN]);
}

static void exit_run(void) {
  led_disable(&ctx.leds[LED_RUN]);
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

static void enter_run_startup(void) { }

static void tick_run_startup(void) {
  ctx.next_state = HSM_STATE_RUN_PROFILE;
}

static void exit_run_startup(void) { }

static enum event_handle_result handle_event_run_startup(const enum hsm_event event) {
  return EVENT_UNHANDLED;
}

// run profile state handlers

static void enter_run_profile(void) { }

static void tick_run_profile(void) { }

static void exit_run_profile(void) { }

static enum event_handle_result handle_event_run_profile(const enum hsm_event event) {
  return EVENT_UNHANDLED;
}

// stop state handlers

static void enter_stop(void) { }

static void tick_stop(void) {
  if (ctx.pending_dtc != DTCID_NONE) {
    ctx.next_state = HSM_STATE_ERROR;
  } else {
    ctx.next_state = HSM_STATE_IDLE;
  }
}

static void exit_stop(void) { }

static enum event_handle_result handle_event_stop(const enum hsm_event event) {
  return EVENT_UNHANDLED;
}

// error state handlers

static void enter_error(void) {
  if (ctx.pending_dtc != DTCID_NONE) {
    dtc_post_event(ctx.pending_dtc);
    ctx.pending_dtc = DTCID_NONE;
  }
  led_enable(&ctx.leds[LED_ERROR]);
}

static void tick_error(void) {
  led_toggle(&ctx.leds[LED_ERROR]);
}

static void exit_error(void) {
  led_disable(&ctx.leds[LED_ERROR]);
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

static void enter_calibration(void) { }

static void tick_calibration(void) { }

static void exit_calibration(void) { }

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

static void exit_state(void) {
  enum hsm_state current_state = ctx.current_state;
  while (current_state != HSM_STATE_ROOT) {
    if (state_table[current_state].exit != NULL) {
      state_table[current_state].exit();
    }
    current_state = state_table[current_state].parent;
  }
}

static void enter_state(void) {
  ctx.current_state = ctx.next_state;
  enum hsm_state current_state = ctx.current_state;
  while (current_state != HSM_STATE_ROOT) {
    if (state_table[current_state].enter != NULL) {
      ctx.enter_timestamp = HAL_GetTick();
      state_table[current_state].enter();
    }
    current_state = state_table[current_state].parent;
  }
}

void hsm_init(void) {
  ctx.current_state = HSM_STATE_RESET;
  ctx.next_state = HSM_STATE_RESET;
  ctx.enter_timestamp = 0;
}

static void hsm_main(void __attribute__((unused)) *argument) {
  while (1) {
    if (ctx.current_state == HSM_STATE_RESET) {
      ctx.current_state = HSM_STATE_INIT;
      ctx.next_state = HSM_STATE_INIT;
      ctx.enter_timestamp = 0;
    }
    if (ctx.current_state != ctx.next_state) {
      exit_state();
      enter_state();
    }
    enum hsm_state current_state = ctx.current_state;
    while (current_state != HSM_STATE_ROOT) {
      if (state_table[current_state].tick != NULL) {
        state_table[current_state].tick();
      }
      current_state = state_table[current_state].parent;
    }
    vTaskDelay(10);
  }
}

void hsm_post_event(const enum hsm_event event) {
  enum hsm_state current_state = ctx.current_state;
  while (current_state != HSM_STATE_ROOT) {
    if (state_table[current_state].handle_event != NULL) {
      enum event_handle_result result = state_table[ctx.current_state].handle_event(event);
      if (result == EVENT_HANDLED) {
        break;
      }
    }
    current_state = state_table[current_state].parent;
  }
}

enum hsm_state hsm_get_current_state(void) {
  return ctx.current_state;
}

void hsm_start(void) {
  BaseType_t ret = xTaskCreate(hsm_main, "hsm", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 20, &hsm_task_handle);
  uassert(ret == pdPASS);
}

