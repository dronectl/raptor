/**
 * @file parser.c
 * @brief SCPI command parser
 * @version 0.1
 * @date 2024-06
 *
 * @copyright Copyright © 2024 dronectl
 *
 */

#include "scpi/utf8.h"
#include "scpi/common.h"
#include "scpi/parser.h"

#include <string.h>
#include <stdbool.h>

#define PARSER_STAT_HDR_DELIM (uint8_t)(1 << 0)
#define PARSER_STAT_ARG_DELIM (uint8_t)(1 << 1)
#define PARSER_STAT_EOH (uint8_t)(1 << 2) // end of header

static bool is_valid_arg(struct parser_handle *phandle, const char arg[SCPI_MAX_TOKEN_LEN], const uint8_t len) {
  // CS TODO: read up on 488.2 arg compliance
  return true;
}

static bool is_valid_hdr(struct parser_handle *phandle, const char hdr[SCPI_MAX_TOKEN_LEN], const uint8_t len) {
  bool num_suffix = false;
  bool status = true;
  uint8_t ccnt = 0;
  for (uint8_t i = 0; i < len; i++) {
    char c = hdr[i];
    if ((!utf8_is_alpha(c) && !utf8_is_numeric(c)) || (utf8_is_alpha(c) && num_suffix)) {
      phandle->error.cidx = i;
      phandle->error.code |= PARSER_ERR_INVALID_HDR;
      status = false;
      break;
    }
    if (utf8_is_numeric(c)) {
      num_suffix = true;
    }
    ccnt++;
  }
  if (ccnt < 3) {
    phandle->error.cidx = ccnt;
    phandle->error.code |= PARSER_ERR_INVALID_HDR;
    status = false;
  }
  return status;
}

void parser(struct parser_handle *phandle, const struct lexer_handle *lhandle) {
  uint8_t pflags = PARSER_STAT_HDR_DELIM;
  for (int i = 0; i < lhandle->tidx; i++) {
    // syntax simplification
    struct parser_cmd *cmd = &phandle->commands[phandle->cmdidx];
    struct lexer_token lexer_token = lhandle->tokens[i];
    switch (lexer_token.type) {
      case LEXER_TT_COMMON:
        cmd->spec |= PARSER_CMD_SPEC_COMMON;
        break;
      case LEXER_TT_QUERY:
        if (!(pflags & PARSER_STAT_HDR_DELIM)) {
          cmd->spec |= PARSER_CMD_SPEC_QUERY;
        } else {
          phandle->error.code |= PARSER_ERR_UNEXPECTED;
        }
        break;
      case LEXER_TT_EOS:
      case LEXER_TT_CMD_SEP:
        phandle->cmdidx++;
        pflags = PARSER_STAT_HDR_DELIM;
        if (!(cmd->spec & PARSER_CMD_SPEC_QUERY)) {
          cmd->spec |= PARSER_CMD_SPEC_SET;
        }
        break;
      case LEXER_TT_HDR_SEP:
        // after 'h:_' ':' token is not allowed
        if (pflags & PARSER_STAT_EOH) {
          phandle->error.code |= PARSER_ERR_UNEXPECTED;
          return;
        }
        pflags |= PARSER_STAT_HDR_DELIM;
        break;
      case LEXER_TT_ARG_SEP:
        // ',' token not allowed before end of headers
        if (!(pflags & PARSER_STAT_EOH) || cmd->spec & PARSER_CMD_SPEC_COMMON) {
          phandle->error.code |= PARSER_ERR_UNEXPECTED;
          return;
        }
        pflags |= PARSER_STAT_ARG_DELIM;
        break;
      case LEXER_TT_SPACE:
        if (!(pflags & PARSER_STAT_HDR_DELIM)) {
          pflags |= PARSER_STAT_EOH;
        }
        break;
      case LEXER_TT_TOKEN:
        if (cmd->spec & PARSER_CMD_SPEC_COMMON && cmd->hidx >= 1) {
          phandle->error.code |= PARSER_ERR_UNEXPECTED;
          return;
        }
        if (pflags & PARSER_STAT_EOH) {
          if (!is_valid_arg(phandle, lexer_token.token.token, lexer_token.token.len)) {
            phandle->error.code |= PARSER_ERR_INVALID_ARG;
            return;
          }
          memcpy(cmd->args[cmd->aidx].token, lexer_token.token.token, lexer_token.token.len);
          cmd->args[cmd->aidx].len = lexer_token.token.len;
          cmd->aidx++;
          pflags &= ~PARSER_STAT_ARG_DELIM;
        } else {
          if (!is_valid_hdr(phandle, lexer_token.token.token, lexer_token.token.len)) {
            phandle->error.code |= PARSER_ERR_INVALID_HDR;
            return;
          }
          memcpy(cmd->headers[cmd->hidx].token, lexer_token.token.token, lexer_token.token.len);
          cmd->headers[cmd->hidx].len = lexer_token.token.len;
          cmd->hidx++;
          pflags &= ~PARSER_STAT_HDR_DELIM;
        }
        break;
      default:
      case LEXER_TT_NULL:
        phandle->error.code |= PARSER_ERR_UNEXPECTED;
        break;
    }
    if (phandle->error.code > 0) {
      phandle->error.tidx = i;
      break;
    }
  }
}
