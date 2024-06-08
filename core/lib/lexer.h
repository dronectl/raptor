
#include "stddef.h"
#include "stdint.h"
#include "scpi_constants.h"

/**
 * @brief Single character tokens
 */
#define LEXER_CHAR_HDR_SEP ':' // header seperator
#define LEXER_CHAR_CMD_SEP ';' // command seperator
#define LEXER_CHAR_ARG_SEP ',' // argument seperator
#define LEXER_CHAR_SPACE ' '   // space
#define LEXER_CHAR_EOS '\n'    // end of sequence
#define LEXER_CHAR_QUERY '?'   // query command
#define LEXER_CHAR_COMMON '*'  // common command

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

enum TokenType {
  TNULL, // null type
  // single char tokens
  EOS,     // end of sequence
  SPACE,   // end of header
  ARG_SEP, // argument seperator
  HDR_SEP, // header seperator
  CMD_SEP, // command seperator
  COMMON,  // common
  QUERY,   // query

  // multichar tokens
  TOKEN,
};

struct token {
  enum TokenType type;
  uint8_t len;
  char lexeme[SCPI_MAX_TOKEN_LEN];
};

struct lexer_handle {
  uint8_t cidx;                         // char index
  uint8_t tidx;                         // token index
  uint8_t status;                       // status register
  uint8_t err;                          // error register
  struct token tokens[SCPI_MAX_TOKENS]; // token structures
};

/**
 * @brief Lex and tag tokens in buffer. A lexing error is reported by the status error bit and the error code in the error member
 *
 * @param[in,out] lhandle lexer context
 * @param[in] buffer SCPI command input buffer
 * @param[in] len length of input buffer
 */
void lexer(struct lexer_handle *lhandle, const char *buffer, const size_t len);
