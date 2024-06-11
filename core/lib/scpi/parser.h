/**
 * @file parser.h
 * @brief SCPI command parser ingests lexed tokens, evaluates syntax compliance and interprets token stream into a SCPI command
 * @version 0.1
 * @date 2024-06
 *
 * @copyright Copyright © 2024 dronectl
 *
 */

#ifndef __PARSER_H__
#define __PARSER_H__

#include <stdint.h>
#include "lexer.h"
#include "scpi_constants.h"

#define PARSER_CMD_SPEC_COMMON (uint8_t)(1 << 0)
#define PARSER_CMD_SPEC_QUERY (uint8_t)(1 << 1)
#define PARSER_CMD_SPEC_SET (uint8_t)(1 << 2)
#define PARSER_CMD_SPEC_ARGS (uint8_t)(1 << 3)

#define PARSER_ERR_INVALID_HDR (uint8_t)(1 << 0)
#define PARSER_ERR_INVALID_ARG (uint8_t)(1 << 1)
#define PARSER_ERR_UNEXPECTED (uint8_t)(1 << 2)
#define PARSER_ERR_UNSUPPORTED (uint8_t)(1 << 3)

struct parser_token {
  uint8_t len;
  char token[SCPI_MAX_TOKEN_LEN];
};

struct parser_cmd {
  uint8_t spec;                                   // command specification register
  uint8_t aidx;                                   // argument index
  uint8_t hidx;                                   // header index
  struct parser_token args[SCPI_MAX_CMD_ARGS];    // argument token registry
  struct parser_token headers[SCPI_MAX_CMD_HDRS]; // header token registry
};

struct parser_err {
  uint8_t code; // error register
  uint8_t tidx; // token idx
  uint8_t cidx; // char idx
};

struct parser_handle {
  uint8_t cmdidx;                                // command index
  struct parser_err error;                       // parser error
  struct parser_cmd commands[SCPI_MAX_COMMANDS]; // command registry
};

/**
 * @brief Interpret SCPI command by parsing lexer token buffer.
 *
 * @param[in,out] phandle parser context handle
 * @param[in] lhandle lexer context handle
 */
void parser(struct parser_handle *phandle, const struct lexer_handle *lhandle);

#endif // __PARSER_H__
