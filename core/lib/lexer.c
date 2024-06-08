/**
 * @file lexer.c
 * @brief SCPI command lexer
 * @version 0.1
 * @date 2024-06
 *
 * @copyright Copyright © 2024 dronectl
 *
 */

#include "lexer.h"
#include <stdbool.h>
#include <unistd.h>

/**
 * @brief Append a character to the correct token structure. Handle incrementing of token and chracter indices
 *
 * @param[in,out] lhandle lexer context
 * @param[in] c next input buffer char
 * @param[in] is_token true if the character is a token
 * @param[in] type token tag to write to token position
 */
static void append_char(struct lexer_handle *lhandle, const char c,
                        bool is_token, const enum TokenType type) {
  if (is_token && lhandle->cidx != 0) {
    lhandle->cidx = 0;
    lhandle->tidx++;
  }
  // overflow protection
  if (lhandle->cidx >= SCPI_MAX_TOKEN_LEN) {
    lhandle->status |= LEXER_STAT_ERR;
    lhandle->err |= LEXER_ERR_LOF;
    return;
  }
  lhandle->tokens[lhandle->tidx].type = type;
  lhandle->tokens[lhandle->tidx].len += 1;
  lhandle->tokens[lhandle->tidx].lexeme[lhandle->cidx] = c;
  if (!is_token) {
    lhandle->cidx++;
  } else {
    lhandle->tidx++;
  }
}

/**
 * @brief Handle the single char token classification.
 *
 * @param[in,out] lhandle lexer context
 * @param[in] c next input buffer character
 */
static void handle_sc_token(struct lexer_handle *lhandle, const char c) {
  enum TokenType type = EOS;
  switch (c) {
    case LEXER_CHAR_HDR_SEP:
      type = HDR_SEP;
      break;
    case LEXER_CHAR_ARG_SEP:
      type = ARG_SEP;
      break;
    case LEXER_CHAR_CMD_SEP:
      type = CMD_SEP;
      break;
    case LEXER_CHAR_QUERY:
      type = QUERY;
      break;
    case LEXER_CHAR_COMMON:
      type = COMMON;
      break;
    case LEXER_CHAR_SPACE:
      type = SPACE;
      break;
    case LEXER_CHAR_EOS:
      lhandle->status |= LEXER_STAT_EOS;
      type = EOS;
      break;
    default:
      // unsupported single char token; return
      lhandle->status |= LEXER_STAT_ERR;
      lhandle->err |= LEXER_ERR_USCT;
      return;
  }
  append_char(lhandle, c, true, type);
}

void lexer(struct lexer_handle *lhandle, const char *buffer, const size_t len) {
  for (size_t i = 0; i < len; i++) {
    char c = buffer[i];
    switch (c) {
      case LEXER_CHAR_HDR_SEP:
      case LEXER_CHAR_CMD_SEP:
      case LEXER_CHAR_QUERY:
      case LEXER_CHAR_COMMON:
      case LEXER_CHAR_ARG_SEP:
      case LEXER_CHAR_SPACE:
      case LEXER_CHAR_EOS:
        handle_sc_token(lhandle, c);
        break;
      default:
        append_char(lhandle, c, false, TOKEN);
        break;
    }
    // OPT: return on error or end of sequence
    if ((lhandle->status & LEXER_STAT_EOS) ||
        (lhandle->status & LEXER_STAT_ERR))
      break;
  }
}
