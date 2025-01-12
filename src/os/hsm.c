
#include "hsm.h"
#include <stdlib.h>


/* raptor state handlers */
static enum HsmState run_idle(struct hsm_ctx *ctx);
static enum HsmState run_init(struct hsm_ctx *ctx);

static const struct hsm_state_table hsm_state_table[HSM_STATE_COUNT] = {
  {HSM_STATE_INIT, .enter = NULL, .run = run_init, .exit = NULL},
  {HSM_STATE_IDLE, .enter = NULL, .run = run_idle, .exit = NULL},
  {HSM_STATE_ERROR, NULL, NULL, NULL}
};

static enum HsmState run_init(struct hsm_ctx *ctx) {
  power_manager_init(ctx->power_manager_ctx);
  esc_engine_init(ctx->esc_engine_ctx);
  return HSM_STATE_IDLE;
}

static enum HsmState run_idle(struct hsm_ctx *ctx) {
  led_toggle(&ctx->leds[HSM_LED_IDLE]);
  return HSM_STATE_IDLE;
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
