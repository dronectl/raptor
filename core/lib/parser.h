#include <stdint.h>
#include "lexer.h"
#include "scpi_constants.h"

#define PARSER_STAT_COMMON (uint8_t)(1 << 0)
#define PARSER_STAT_QUERY (uint8_t)(1 << 1)
#define PARSER_STAT_SET (uint8_t)(1 << 2)
#define PARSER_STAT_ARGS (uint8_t)(1 << 3)
#define PARSER_STAT_ERR (uint8_t)(1 << 7)

#define PARSER_ERR_INVALID_HDR (uint8_t)(1 << 0)
#define PARSER_ERR_INVALID_ARG (uint8_t)(1 << 1)
#define PARSER_ERR_UNEXPECTED (uint8_t)(1 << 2)
#define PARSER_ERR_UNSUPPORTED (uint8_t)(1 << 3)

struct parser_token {
  uint8_t len;
  char token[SCPI_MAX_TOKEN_LEN];
};

struct parser_handle {
  uint8_t cidx;   // command index
  uint8_t status; // status register
  struct {
    uint8_t err_status; // error register
    uint8_t tidx;       // token idx
    uint8_t cidx;       // char idx
  } error;
  struct {
    uint8_t aidx;                                 // argument index
    uint8_t hidx;                                 // header index
    struct parser_token args[SCPI_MAX_TOKENS];    // argument token registry
    struct parser_token headers[SCPI_MAX_TOKENS]; // header token registry
  } command[SCPI_MAX_COMMANDS];                   // command registry
};

void parser(struct parser_handle *phandle, const struct lexer_handle *lhandle);
