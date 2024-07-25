/**
 * @file lexer.h
 * @brief SCPI command lexer bifurcates string stream into distinct scpi tokens to be fed into the SCPI parser
 * @version 0.1
 * @date 2024-06
 *
 * @copyright Copyright © 2024 dronectl
 *
 */

#ifndef __LEXER_H__
#define __LEXER_H__

#include "common.h"
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Lexer status bits
 */
#define LEXER_STAT_EOS (uint8_t)(1 << 0) // end of sequence reached
#define LEXER_STAT_EOH (uint8_t)(1 << 1) // end of header reached
#define LEXER_STAT_ERR (uint8_t)(1 << 2) // error

/**
 * @brief Lexer error bits
 */
#define LEXER_ERR_LOF (uint8_t)(1 << 0)  // Lexeme overflow
#define LEXER_ERR_USCT (uint8_t)(1 << 1) // unsupported single char token

enum LexerTokenType {
  LEXER_TT_NULL, // null type
  // single char tokens
  LEXER_TT_EOS,     // end of sequence
  LEXER_TT_SPACE,   // end of header
  LEXER_TT_ARG_SEP, // argument seperator
  LEXER_TT_HDR_SEP, // header seperator
  LEXER_TT_CMD_SEP, // command seperator
  LEXER_TT_COMMON,  // common
  LEXER_TT_QUERY,   // query
  // multichar tokens
  LEXER_TT_TOKEN,
  NUM_LEXER_TT
};

struct lexer_token {
  enum LexerTokenType type;
  struct scpi_token token;
};

struct lexer_handle {
  uint8_t cidx;                               // char index
  uint8_t tidx;                               // token index
  uint8_t status;                             // status register
  uint8_t err;                                // error register
  enum LexerTokenType prev_tt;                // previous token type
  struct lexer_token tokens[SCPI_MAX_TOKENS]; // token structures
};

/**
 * @brief Lex and tag tokens in buffer. A lexing error is reported by the status error bit and the error code in the error member
 * @param[in,out] lhandle lexer context
 * @param[in] buffer SCPI command input buffer
 * @param[in] len length of input buffer
 */
void lexer_run(struct lexer_handle *lhandle, const char *buffer, const size_t len);
void lexer_init(struct lexer_handle *lhandle);

#endif // __LEXER_H__
