
#include "hsm.h"
#include "logger.h"
#include <stdlib.h>


/* raptor state handlers */
static void handle_event_root(struct hsm_ctx *ctx, const enum HsmEvent event);

static void run_init(struct hsm_ctx *ctx);

static void run_idle(struct hsm_ctx *ctx);
static void handle_event_idle(struct hsm_ctx *ctx, const enum HsmEvent event);
static void handle_event_error(struct hsm_ctx *ctx, const enum HsmEvent event);

static void enter_run_startup(struct hsm_ctx *ctx);
static void run_run_startup(struct hsm_ctx *ctx);
static void exit_run_startup(struct hsm_ctx *ctx);
static void handle_event_run_startup(struct hsm_ctx *ctx, const enum HsmEvent event);

static void run_error(struct hsm_ctx *ctx);
static void run_run(struct hsm_ctx *ctx);
static void enter_error(struct hsm_ctx *ctx);


static const struct hsm_state_table hsm_state_table[HSM_STATE_COUNT] = {
  {HSM_STATE_ROOT, .parent = HSM_STATE_ROOT, .enter = NULL, .run = NULL, .exit = NULL, .handle_event = handle_event_root},
  {HSM_STATE_INIT, .parent = HSM_STATE_ROOT, .enter = NULL, .run = run_init, .exit = NULL, .handle_event = NULL},
  {HSM_STATE_IDLE, .parent = HSM_STATE_ROOT, .enter = NULL, .run = run_idle, .exit = NULL, .handle_event = handle_event_idle},
  {HSM_STATE_RUN, .parent = HSM_STATE_ROOT, .enter = NULL, .run = run_run, .exit = NULL, .handle_event = handle_event_idle},
  {HSM_STATE_RUN_STARTUP, .parent = HSM_STATE_RUN, .enter = enter_run_startup, .run = run_run_startup, .exit = exit_run_startup, .handle_event = handle_event_run_startup},
  {HSM_STATE_ERROR, .parent = HSM_STATE_ROOT, .enter = enter_error, .run = run_error, .exit = NULL, .handle_event = handle_event_error},
};


static void handle_event_root(struct hsm_ctx *ctx, enum HsmEvent event){
  switch (event) {
    default:
      error("Unhandled event %d in state %d", event, ctx->current_state);
      dtc_post_event(DTCID_HSM_UNHANDLED_EVENT);
      break;
  }
}

static void run_init(struct hsm_ctx *ctx) {
  power_manager_init(ctx->power_manager_ctx);
  esc_engine_init(ctx->esc_engine_ctx);
}

static void handle_event_idle(struct hsm_ctx *ctx, enum HsmEvent event) {
  switch (event) {
    case HSM_EVENT_RUN:
      ctx->next_state = HSM_STATE_RUN;
      ctx->pending_event = HSM_EVENT_NONE;
      break;
    case HSM_EVENT_CALIBRATION:
      ctx->next_state = HSM_STATE_RUN;
      ctx->pending_event = HSM_EVENT_NONE;
      break;
    default:
      break;
  }
}

static void run_idle(struct hsm_ctx *ctx) {
  led_toggle(&ctx->leds[HSM_LED_IDLE]);
}

static void run_run(struct hsm_ctx *ctx) {
  led_toggle(&ctx->leds[HSM_LED_RUN]);
}

static void enter_error(struct hsm_ctx *ctx) {
  // post error encountered to DTC
  dtc_post_event(ctx->pending_dtc);
}

static void run_error(struct hsm_ctx *ctx) {
  led_toggle(&ctx->leds[HSM_LED_ERROR]);
}

static void handle_event_error(struct hsm_ctx *ctx, enum HsmEvent event) {
  switch (event) {
    case HSM_EVENT_CLEAR_ERROR:
      ctx->next_state = HSM_STATE_IDLE;
      ctx->pending_event = HSM_EVENT_NONE;
      break;
    default:
      break;
  }
}


void hsm_init(struct hsm_ctx *ctx) {
  ctx->current_state = HSM_STATE_RESET;
  ctx->next_state = HSM_STATE_RESET;
  ctx->enter_timestamp = 0;
}

void hsm_run(struct hsm_ctx *ctx) {
  if (ctx->current_state == HSM_STATE_RESET) {
    ctx->current_state = HSM_STATE_INIT;
    ctx->next_state = HSM_STATE_INIT;
    ctx->enter_timestamp = 0;
  }

  if (ctx->current_state != ctx->next_state) {
    if (hsm_state_table[ctx->current_state].exit != NULL) {
      hsm_state_table[ctx->current_state].exit(ctx);
    }
    ctx->current_state = ctx->next_state;
    if (hsm_state_table[ctx->current_state].enter != NULL) {
      hsm_state_table[ctx->current_state].enter(ctx);
    }
  }

  if (hsm_state_table[ctx->current_state].run != NULL) {
    hsm_state_table[ctx->current_state].run(ctx);
  }
}

void hsm_post_event(struct hsm_ctx *ctx, enum HsmEvent event) {
  ctx->pending_event = event;
}
