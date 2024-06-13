/**
 * @file common.h
 * @brief SCPI common const and struct definitions
 * @version 0.1
 * @date 2024-06
 *
 * @copyright Copyright © 2024 dronectl
 *
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>
#include <stddef.h>

#define SCPI_PORT 5025
#define SCPI_MAX_INPUT_BUFFER_LEN 1024
#define SCPI_MAX_RESPONSE_LEN 128
#define SCPI_MAX_CMD_ARGS 10
#define SCPI_MAX_CMD_HDRS 5
#define SCPI_MAX_COMMANDS 5
#define SCPI_MAX_TOKENS 30
#define SCPI_MAX_RET_LEN 10
#define SCPI_MAX_TOKEN_LEN 15
#define SCPI_MAX_ABBR_TOKEN_LEN 5

typedef int scpi_err_t;

#define SCPI_ERR_OK (scpi_err_t)0
#define SCPI_ERR_GEN (scpi_err_t)1
#define SCPI_ERR_BAD_ARG (scpi_err_t)2
#define SCPI_ERR_OUT_OF_RANGE (scpi_err_t)3

struct scpi_token {
  uint8_t len;
  char token[SCPI_MAX_TOKEN_LEN];
};

struct scpi_handle {
  char buffer[SCPI_MAX_INPUT_BUFFER_LEN];
  size_t buflen;
  int clfd;
  int error;
};

struct scpi_header {
  char abbr[SCPI_MAX_ABBR_TOKEN_LEN];
  char full[SCPI_MAX_TOKEN_LEN];
};

struct scpi_endpoint {
  const struct scpi_header *headers[SCPI_MAX_CMD_HDRS];
  scpi_err_t (*query)(const uint8_t, const struct scpi_token argv[SCPI_MAX_CMD_ARGS], const int, char *);
  scpi_err_t (*write)(const uint8_t, const struct scpi_token argv[SCPI_MAX_CMD_ARGS]);
};

#endif // __COMMON_H__
