/**
 * @file lexer.c
 * @brief SCPI command lexer
 * @version 0.1
 * @date 2024-06
 *
 * @copyright Copyright © 2024 dronectl
 *
 */

#include "scpi/lexer.h"

#include <stdbool.h>
#include <string.h>
#include <unistd.h>

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
 * @brief Classify token to the an enum type
 *
 * @param[in] c char to classify
 */
static enum LexerTokenType classify_char(const char c) {
  enum LexerTokenType type;
  switch (c) {
  case LEXER_CHAR_HDR_SEP:
    type = LEXER_TT_HDR_SEP;
    break;
  case LEXER_CHAR_ARG_SEP:
    type = LEXER_TT_ARG_SEP;
    break;
  case LEXER_CHAR_CMD_SEP:
    type = LEXER_TT_CMD_SEP;
    break;
  case LEXER_CHAR_QUERY:
    type = LEXER_TT_QUERY;
    break;
  case LEXER_CHAR_COMMON:
    type = LEXER_TT_COMMON;
    break;
  case LEXER_CHAR_SPACE:
    type = LEXER_TT_SPACE;
    break;
  case LEXER_CHAR_EOS:
    type = LEXER_TT_EOS;
    break;
  default:
    // assume character is part of a token
    type = LEXER_TT_TOKEN;
    break;
  }
  return type;
}

/**
 * @brief Append a character to the correct token structure. Handle incrementing of token and chracter indices
 *
 * @param[in,out] lexer lexer context
 * @param[in] c next input buffer char
 */
static void process_char(struct lexer_handle *lexer, const char c) {
  enum LexerTokenType type = classify_char(c);
  if (type == LEXER_TT_EOS) {
    lexer->status_flags |= LEXER_STAT_EOS;
  }
  if (type == LEXER_TT_TOKEN) {
    if (lexer->prev_token_type == LEXER_TT_TOKEN) {
      lexer->char_index++;
    } else {
      if (lexer->prev_token_type != LEXER_TT_NULL) {
        lexer->token_index++;
      }
      lexer->char_index = 0;
    }
  } else {
    if (lexer->prev_token_type != LEXER_TT_NULL) {
      lexer->token_index++;
    }
    lexer->char_index = 0;
  }
  // overflow protection
  if (lexer->char_index >= SCPI_MAX_TOKEN_LEN) {
    lexer->status_flags |= LEXER_STAT_ERR;
    lexer->error_flags |= LEXER_ERR_LOF;
    return;
  }
  lexer->tokens[lexer->token_index].type = type;
  lexer->tokens[lexer->token_index].token.len += 1;
  lexer->tokens[lexer->token_index].token.value[lexer->char_index] = c;
  lexer->prev_token_type = type;
}

lex_status_t lexer_process_buffer(struct lexer_handle *lexer, const char *buffer, const size_t len) {
  if (lexer == NULL || buffer == NULL || len == 0) {
    return LEX_STAT_BAD_ARG;
  }
  lex_status_t status = LEX_STAT_GEN_ERR;
  for (size_t i = 0; i < len; i++) {
    char c = buffer[i];
    process_char(lexer, c);
    if (lexer->status_flags & LEXER_STAT_ERR) {
      status = LEX_STAT_GEN_ERR;
      break;
    }
    if ((lexer->status_flags & LEXER_STAT_EOS)) {
      status = LEX_STAT_OK;
      break;
    }
  }
  return status;
}

void lexer_init(struct lexer_handle *lexer) {
  lexer->token_index = 0;
  lexer->char_index = 0;
  lexer->error_flags = 0;
  lexer->status_flags = 0;
  lexer->prev_token_type = LEXER_TT_NULL;
  memset(&lexer->tokens, 0, sizeof(lexer->tokens));
}
