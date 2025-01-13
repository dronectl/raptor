
#include "hsm.h"
#include "logger.h"
#include <stdlib.h>
#include <stdint.h>
#include "dtc.h"
#include "led.h"
#include "esc_engine.h"
#include "power_manager.h"

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
  enum hsm_event event;
  enum hsm_state parent;
  void (*enter)(void);
  void (*tick)(void);
  void (*exit)(void);
  enum event_handle_result (*handle_event)(const enum hsm_event event);
} _hsm_state_table;


/* raptor state handlers */
static enum event_handle_result handle_event_root(const enum hsm_event event);

static void run_init(void);

static void run_idle(void);
static enum event_handle_result handle_event_idle(struct hsm_ctx *ctx, const enum hsm_event event);

static  enum event_handle_result handle_event_error(struct hsm_ctx *ctx, const enum hsm_event event);

// run handlers
static void enter_run(void);
static void tick_run(void);
static void exit_run(void);
static enum event_handle_result handle_event_run(const enum hsm_event event);

// run startup handlers
static void enter_run_startup(struct hsm_ctx *ctx);
static void tick_run_startup(struct hsm_ctx *ctx);
static void exit_run_startup(struct hsm_ctx *ctx);
static enum event_handle_result handle_event_run_startup(const enum hsm_event event);

// run profile handlers
static void enter_run_profile(struct hsm_ctx *ctx);
static void tick_run_profile(struct hsm_ctx *ctx);
static void exit_run_profile(struct hsm_ctx *ctx);
static enum event_handle_result handle_event_run_profile(const enum hsm_event event);

// run shutdown handlers
static void enter_run_shutdown(void);
static void tick_run_shutdown(void);
static void exit_run_shutdown(void);
static enum event_handle_result handle_event_run_shutdown(const enum hsm_event event);

static void run_error(struct hsm_ctx *ctx);
static void enter_error(struct hsm_ctx *ctx);

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
  {HSM_STATE_IDLE, .parent = HSM_STATE_ROOT, .enter = NULL, .tick = run_idle, .exit = NULL, .handle_event = handle_event_idle},
  {HSM_STATE_RUN, .parent = HSM_STATE_ROOT, .enter = enter_run, .tick = tick_run, .exit = NULL, .handle_event = handle_event_run},
  {HSM_STATE_RUN_STARTUP, .parent = HSM_STATE_RUN, .enter = enter_run_startup, .tick = tick_run_startup, .exit = exit_run_startup, .handle_event = handle_event_run_startup},
  {HSM_STATE_RUN_PROFILE, .parent = HSM_STATE_RUN, .enter = enter_run_profile, .tick = tick_run_profile, .exit = exit_run_profile, .handle_event = handle_event_run_profile},
  {HSM_STATE_RUN_SHUTDOWN, .parent = HSM_STATE_RUN, .enter = enter_run_shutdown, .tick = tick_run_shutdown, .exit = exit_run_shutdown, .handle_event = handle_event_run_shutdown},
  {HSM_STATE_ERROR, .parent = HSM_STATE_ROOT, .enter = enter_error, .tick = run_error, .exit = NULL, .handle_event = handle_event_error},
};


static int handle_event_root(const enum hsm_event event){
  switch (event) {
    default:
      warning("Unhandled event %d in state %d", event, ctx.current_state);
      dtc_post_event(DTCID_HSM_UNHANDLED_EVENT);
      break;
  }
  return 1;
}

static void run_init(void) {
  power_manager_init();
  esc_engine_init();
}

// Idle state handlers

static enum event_handle_result handle_event_idle(const enum hsm_event event) {
  enum event_handle_result result = EVENT_UNHANDLED;
  switch (event) {
    case HSM_EVENT_RUN:
      ctx.next_state = HSM_STATE_RUN;
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

static void run_idle(void) {
  led_toggle(&ctx.leds[LED_IDLE]);
}

// Run state handlers
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
      ctx.next_state = HSM_STATE_RUN_SHUTDOWN;
      result = EVENT_HANDLED;
      break;
    default:
      break;
  }
  return result;
}

static void enter_error(struct hsm_ctx *ctx) {
  // post error encountered to DTC
  dtc_post_event(ctx->pending_dtc);
}

static void run_error(struct hsm_ctx *ctx) {
  led_toggle(&ctx->leds[HSM_LED_ERROR]);
}

static void handle_event_error(struct hsm_ctx *ctx, enum hsm_event event) {
  switch (event) {
    case HSM_EVENT_CLEAR_ERROR:
      ctx->next_state = HSM_STATE_IDLE;
    default:
      break;
  }
}


void hsm_init(void) {
  ctx.current_state = HSM_STATE_RESET;
  ctx.next_state = HSM_STATE_RESET;
  ctx.enter_timestamp = 0;
}

void hsm_do_tick(void) {
  if (ctx.current_state == HSM_STATE_RESET) {
    ctx.current_state = HSM_STATE_INIT;
    ctx.next_state = HSM_STATE_INIT;
    ctx.enter_timestamp = 0;
  }

  if (ctx.current_state != ctx.next_state) {
    if (hsm_state_table[ctx.current_state].exit != NULL) {
      hsm_state_table[ctx.current_state].exit();
    }
    ctx.current_state = ctx.next_state;
    if (hsm_state_table[ctx.current_state].enter != NULL) {
      hsm_state_table[ctx.current_state].enter();
    }
  }

  if (hsm_state_table[ctx.current_state].run != NULL) {
    hsm_state_table[ctx.current_state].run();
  }
}

void hsm_post_event(const enum hsm_event event) {
  while (1) {
    if (hsm_state_table[ctx.current_state].handle_event != NULL) {
      // if the event is handled, break out of the loop
      if (hsm_state_table[ctx.current_state].handle_event(event) == 1) {
        break;
      }
    }
    if (hsm_state_table[ctx.current_state].parent == HSM_STATE_ROOT) {
      break;
    }
    // move up the state hierarchy to find a handler for the event
    ctx.current_state = hsm_state_table[ctx.current_state].parent;
  }
}
