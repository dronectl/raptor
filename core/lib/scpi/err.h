
#ifndef __ERR_H__
#define __ERR_H__

#include <stddef.h>

#define MAX_REASON_LEN 25

struct scpi_err {
  int code;
  char reason[MAX_REASON_LEN];
};

typedef enum ScpiErrorType {
  SCPI_ERR_NULL = 0,
  SCPI_ERR_SYNTAX,
  SCPI_ERR_BAD_ARG,
  SCPI_ERR_UNDEFINED_HEADER,
  SCPI_ERR_OUT_OF_RANGE,
  SCPI_ERR_EQUEUE_OF,
  NUM_SCPI_ERR
} scpi_err_t;

void scpi_error_init(void);
void scpi_error_push(const scpi_err_t error);
int scpi_error_strfmt(const scpi_err_t error, char *buffer);
scpi_err_t scpi_error_pop(void);

#endif // __ERR_H__
