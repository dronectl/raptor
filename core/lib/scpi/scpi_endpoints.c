
#include "logger.h"
#include "sysreg.h"
#include "utf8.h"
#include "scpi_endpoints.h"
#include "scpi/scpi.h"
#include <stdbool.h>
#include <string.h>

const struct scpi_header IDN = {.abbr = "idn", .full = "idn"};
const struct scpi_header RST = {.abbr = "rst", .full = "rst"};
const struct scpi_header CONTrol = {.abbr = "cont", .full = "control"};
const struct scpi_header SETpoint = {.abbr = "set", .full = "setpoint"};
const struct scpi_header STATus = {.abbr = "stat", .full = "status"};

/************************************ SCPI ENDPOINTS **********************************************/

static scpi_err_t error = SCPI_ERR_OK;

static scpi_err_t _system_reset(__attribute__((unused)) const uint8_t argc, __attribute__((unused)) const struct scpi_token argv[SCPI_MAX_CMD_ARGS]);

static struct scpi_endpoint endpoints[] = {
    {.headers = {&IDN}, .query = NULL, .write = NULL},
    {.headers = {&RST}, .query = NULL, .write = &_system_reset},
    {.headers = {&CONTrol, &SETpoint}, .query = NULL, .write = NULL},
    {.headers = {&CONTrol, &STATus}, .query = NULL, .write = NULL},
};

/************************************ SCPI ENDPOINTS **********************************************/

static scpi_err_t _system_reset(__attribute__((unused)) const uint8_t argc, __attribute__((unused)) const struct scpi_token argv[SCPI_MAX_CMD_ARGS]) {
  info("Resetting system registers");
  sysreg_reset();
  return SCPI_ERR_OK;
}

const size_t num_endpoints = sizeof(endpoints) / sizeof(endpoints[0]);

static bool header_comparator(const struct scpi_header *sh, const struct scpi_token *st) {
  bool abbr = false;
  if (st->len == strlen(sh->abbr)) {
    abbr = true;
  } else if (st->len == strlen(sh->full)) {
    abbr = false;
  } else {
    return false;
  }
  char token[SCPI_MAX_TOKEN_LEN] = {0};
  memcpy(token, st->token, st->len);
  for (size_t i = 0; i < st->len; i++) {
    token[i] = utf8_uppercase_to_lowercase(token[i]);
  }
  if (abbr) {
    return strncmp(sh->abbr, token, st->len) == 0;
  }
  return strncmp(sh->full, token, st->len) == 0;
}

void scpi_endpoint_process_write(const int index, const uint8_t argc, const struct scpi_token argv[]) {
  if (index < 0 || index >= (int)num_endpoints) {
    error("Endpoint index out of range\n");
    return;
  }
  if (endpoints[index].write == NULL) {
    error("Write not found\n");
    return;
  }
  scpi_err_t status = endpoints[index].write(argc, argv);
  // CS TODO: set error registers
  error = status;
}

void scpi_endpoint_process_query(const int index, const uint8_t argc, const struct scpi_token argv[], char *buffer, const size_t size) {
  if (index < 0 || index >= (int)num_endpoints) {
    error("Endpoint index out of range\n");
    return;
  }
  if (endpoints[index].query == NULL) {
    error("Query not found\n");
    return;
  }
  scpi_err_t status = endpoints[index].query(argc, argv, size, buffer);
  // CS TODO: set error registers
  error = status;
}

int scpi_endpoint_search(const struct scpi_token sts[], const uint8_t len) {
  for (int i = 0; i < (int)num_endpoints; i++) {
    for (int j = 0; j < len + 1; j++) {
      if (!header_comparator(endpoints[i].headers[j], &sts[j])) {
        break;
      }
      if (j >= len) {
        return i;
      }
    }
  }
  return -1;
}
