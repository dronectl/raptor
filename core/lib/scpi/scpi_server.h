
#ifndef __SCPI_H__
#define __SCPI_H__

#include <stddef.h>
#include "scpi_constants.h"

typedef int scpi_err_t;

#define SCPI_ERR_OK (scpi_err_t)0
#define SCPI_ERR_GEN (scpi_err_t)1
#define SCPI_ERR_BAD_ARG (scpi_err_t)2
#define SCPI_ERR_OUT_OF_RANGE (scpi_err_t)3

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
  scpi_err_t (*query)(int, char (*)[SCPI_MAX_TOKEN_LEN], int *, char (*)[SCPI_MAX_TOKEN_LEN]);
  scpi_err_t (*write)(int, char (*)[SCPI_MAX_TOKEN_LEN]);
};

#endif // __SCPI_H__
