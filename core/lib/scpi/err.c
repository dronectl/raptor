
#include "logger.h"
#include "sysreg.h"
#include "cbuffer.h"
#include "scpi/ieee.h"
#include "scpi/err.h"
#include <stdio.h>

static scpi_err_t errors[IEEE_SCPI_MAX_ERR_QUEUE_LEN];
static struct cbuffer_handle equeue;

static const struct scpi_err emap[NUM_SCPI_ERR] = {
    [SCPI_ERR_NULL] = {.code = 0, .reason = "No error"},
    [SCPI_ERR_UNDEFINED_HEADER] = {.code = -113, .reason = "Undefined header"},
    [SCPI_ERR_SYNTAX] = {.code = -222, .reason = "Bad syntax"},
    [SCPI_ERR_EQUEUE_OF] = {.code = -350, .reason = "Error queue overflow"},
};

void scpi_error_init(void) {
  cbuffer_init(&equeue, errors, sizeof(scpi_err_t), sizeof(errors));
}

void scpi_error_push(const scpi_err_t err) {
  uint8_t stb;
  cbuffer_status_t status = cbuffer_push(&equeue, &err);
  trace("pushing error (%d) to fifo queue\n", err);
  if (status == CBUFFER_OVERFLOW) {
    errors[equeue.tail] = SCPI_ERR_EQUEUE_OF;
  }
  sysreg_get_u8(SYSREG_STB, &stb);
  stb |= SYSREG_STB_ERR_QUEUE;
  sysreg_set_u8(SYSREG_STB, &stb);
}

scpi_err_t scpi_error_pop(void) {
  uint8_t stb;
  scpi_err_t err;
  cbuffer_status_t status = cbuffer_pop(&equeue, &err);
  trace("popping error (%d) from fifo queue\n", err);
  if (status == CBUFFER_UNDERFLOW) {
    trace("FIFO error queue underflow\n");
    err = SCPI_ERR_NULL;
  } else if (status == CBUFFER_EMPTY) {
    trace("FIFO error queue empty\n");
    sysreg_get_u8(SYSREG_STB, &stb);
    stb &= ~SYSREG_STB_ERR_QUEUE;
    sysreg_set_u8(SYSREG_STB, &stb);
  }
  return err;
}

int scpi_error_strfmt(const scpi_err_t error, char *buffer) {
  return snprintf(buffer, MAX_REASON_LEN, IEEE_ERR_STRFMT, emap[error].code, emap[error].reason);
}
