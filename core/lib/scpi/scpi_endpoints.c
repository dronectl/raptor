
#include "logger.h"
#include "scpi/parser.h"
#include "sysreg.h"
#include "utf8.h"
#include "scpi_endpoints.h"
#include <stdbool.h>
#include <string.h>

const struct scpi_header IDN = {.abbr = "idn", .full = "idn"};
const struct scpi_header RST = {.abbr = "rst", .full = "rst"};
const struct scpi_header CONTrol = {.abbr = "cont", .full = "control"};
const struct scpi_header SETpoint = {.abbr = "set", .full = "setpoint"};
const struct scpi_header STATus = {.abbr = "stat", .full = "status"};

/************************************ SCPI ENDPOINTS **********************************************/

static scpi_err_t error = SCPI_ERR_OK;

static scpi_err_t scpi_ep_system_reset(__attribute__((unused)) int argc, __attribute__((unused)) char (*argv)[SCPI_MAX_TOKEN_LEN]) {
  info("Resetting system registers");
  sysreg_reset();
  return SCPI_ERR_OK;
}

/************************************ SCPI ENDPOINTS **********************************************/

static struct scpi_endpoint endpoints[] = {
    {.headers = {&IDN}, .query = NULL, .write = NULL},
    {.headers = {&RST}, .query = NULL, .write = &scpi_ep_system_reset},
    {.headers = {&CONTrol, &SETpoint}, .query = NULL, .write = NULL},
    {.headers = {&CONTrol, &STATus}, .query = NULL, .write = NULL},
};

const size_t num_endpoints = sizeof(endpoints) / sizeof(endpoints[0]);

static bool header_comparator(const struct scpi_header *sh, const struct parser_token *ptoken) {
  bool abbr = false;
  if (ptoken->len == strlen(sh->abbr)) {
    abbr = true;
  } else if (ptoken->len == strlen(sh->full)) {
    abbr = false;
  } else {
    return false;
  }
  char token[SCPI_MAX_TOKEN_LEN] = {0};
  memcpy(token, ptoken->token, ptoken->len);
  for (size_t i = 0; i < ptoken->len; i++) {
    token[i] = utf8_uppercase_to_lowercase(token[i]);
  }
  // compare lowercase headers
  if (abbr) {
    return strncmp(sh->abbr, token, ptoken->len) == 0;
  }
  return strncmp(sh->full, token, ptoken->len) == 0;
}

void scpi_endpoint_process_write(const int index, const struct parser_cmd *cmd) {
  if (index < 0 || index >= (int)num_endpoints) {
    error("Endpoint index out of range\n");
    return;
  }
  if (endpoints[index].write == NULL) {
    error("Write not found\n");
    return;
  }
  scpi_err_t status = endpoints[index].write(cmd->aidx + 1, cmd->args);
  // CS TODO: set error registers
  error = status;
}

void scpi_endpoint_process_query(const int index, struct parser_cmd *cmd, char *buffer, int *len) {
  char response[SCPI_MAX_RESPONSE_LEN][SCPI_MAX_TOKEN_LEN] = {0};
  if (index < 0 || index >= (int)num_endpoints) {
    error("Endpoint index out of range\n");
    return;
  }
  if (endpoints[index].query == NULL) {
    error("Query not found\n");
    return;
  }
  scpi_err_t status = endpoints[index].query(cmd->aidx + 1, cmd->args, len, response);
  if (*len >= buf_size) {
    return;
  }
  memcpy(buffer, response, len);
  // CS TODO: set error registers
  error = status;
}

int scpi_endpoint_search(const struct parser_cmd *pcmd) {
  for (int i = 0; i < (int)num_endpoints; i++) {
    for (int j = 0; j < pcmd->hidx + 1; j++) {
      if (!header_comparator(endpoints[i].headers[j], &pcmd->headers[j])) {
        break;
      }
      if (j >= pcmd->hidx) {
        return i;
      }
    }
    return -1;
  }
}
