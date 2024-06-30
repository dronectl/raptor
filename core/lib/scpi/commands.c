
#include "logger.h"
#include "sysreg.h"
#include "scpi/utf8.h"
#include "scpi/common.h"
#include "scpi/commands.h"
#include "scpi/err.h"

#include <stdbool.h>
#include <string.h>

const struct scpi_header IDN = {.abbr = "idn", .full = "idn"};
const struct scpi_header RST = {.abbr = "rst", .full = "rst"};
const struct scpi_header CONTrol = {.abbr = "cont", .full = "control"};
const struct scpi_header SETpoint = {.abbr = "set", .full = "setpoint"};
const struct scpi_header STATus = {.abbr = "stat", .full = "status"};
const struct scpi_header SYSTem = {.abbr = "syst", .full = "system"};
const struct scpi_header ERRor = {.abbr = "err", .full = "error"};

/************************************ SCPI ENDPOINTS **********************************************/

const char idn[100] = "dronectl, raptor, v0.1.0\n";

static scpi_err_t _system_reset(__attribute__((unused)) const uint8_t argc, __attribute__((unused)) const struct scpi_token argv[SCPI_MAX_CMD_ARGS]);
static scpi_err_t _get_idn(__attribute__((unused)) const uint8_t argc, __attribute__((unused)) const struct scpi_token argv[SCPI_MAX_CMD_ARGS], char *buffer, const size_t size);
static scpi_err_t _system_error_pop(__attribute__((unused)) const uint8_t argc, __attribute__((unused)) const struct scpi_token argv[SCPI_MAX_CMD_ARGS], char *buffer, const size_t size);

static const struct scpi_endpoint endpoints[] = {
    {.headers = {&IDN}, .query = _get_idn, .write = NULL},
    {.headers = {&SYSTem, &ERRor}, .query = _system_error_pop, .write = NULL},
    {.headers = {&RST}, .query = NULL, .write = _system_reset},
    {.headers = {&CONTrol, &SETpoint}, .query = NULL, .write = NULL},
    {.headers = {&CONTrol, &STATus}, .query = NULL, .write = NULL},
};

#define NUM_ENDPOINTS (sizeof(endpoints) / sizeof(struct scpi_endpoint))

/************************************ SCPI ENDPOINTS **********************************************/
static scpi_err_t _get_idn(__attribute__((unused)) const uint8_t argc, __attribute__((unused)) const struct scpi_token argv[SCPI_MAX_CMD_ARGS], char *buffer, const size_t size) {
  memcpy(buffer, idn, sizeof(idn));
  return SCPI_ERR_NULL;
}

static scpi_err_t _system_error_pop(__attribute__((unused)) const uint8_t argc, __attribute__((unused)) const struct scpi_token argv[SCPI_MAX_CMD_ARGS], char *buffer, const size_t size) {
  scpi_err_t syserror = scpi_error_pop();
  scpi_error_strfmt(syserror, buffer, size);
  return SCPI_ERR_NULL;
}

static scpi_err_t _system_reset(__attribute__((unused)) const uint8_t argc, __attribute__((unused)) const struct scpi_token argv[SCPI_MAX_CMD_ARGS]) {
  info("Resetting system registers\n");
  sysreg_reset();
  return SCPI_ERR_NULL;
}

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

void commands_process_write(const int index, const uint8_t argc, const struct scpi_token argv[]) {
  if (index < 0 || index >= (int)NUM_ENDPOINTS - 1) {
    error("Endpoint index out of range\n");
    return;
  }
  if (endpoints[index].write == NULL) {
    error("Write not found\n");
    return;
  }
  scpi_err_t status = endpoints[index].write(argc, argv);
  if (status != SCPI_ERR_NULL) {
    scpi_error_push(status);
  }
}

void commands_process_query(const int index, const uint8_t argc, const struct scpi_token argv[], char *buffer, const size_t size) {
  if (index < 0 || index >= 4) {
    error("Endpoint index out of range\n");
    return;
  }
  if (endpoints[index].query == NULL) {
    error("Query not found\n");
    return;
  }
  scpi_err_t status = endpoints[index].query(argc, argv, buffer, size);
  if (status != SCPI_ERR_NULL) {
    scpi_error_push(status);
  }
}

int commands_search_index(const struct scpi_token sts[], const uint8_t len) {
  for (size_t i = 0; i < NUM_ENDPOINTS; i++) {
    for (int j = 0; j < len; j++) {
      if (!header_comparator(endpoints[i].headers[j], &sts[j])) {
        break;
      }
      if (j + 1 == len) {
        return i;
      }
    }
  }
  scpi_error_push(SCPI_ERR_UNDEFINED_HEADER);
  return -1;
}
