#include "parser.h"
#include "string.h"
#include <stdbool.h>
#include "scpi_constants.h"

static bool is_numeric(const char token) {
  return (token >= 0x30 && token <= 0x39);
}

static bool is_alpha(const char token) {
  return ((token >= 0x41 && token <= 0x5a) || (token >= 0x61 && token <= 0x7a));
}

static bool is_uppercase(char alpha) { return (alpha < 0x5a && alpha > 0x41); }

static bool is_valid_arg(const char hdr[SCPI_MAX_TOKEN_LEN],
                         struct parser_handle *phandle) {
  // CS TODO: read up on 488.2 arg compliance
  return true;
}

static bool is_valid_hdr(struct parser_handle *phandle, const char hdr[SCPI_MAX_TOKEN_LEN],
                         const uint8_t len) {
  bool num_suffix = false;
  bool status = true;
  uint8_t ccnt = 0;
  for (uint8_t i = 0; i < len; i++) {
    char c = hdr[i];
    if ((!is_alpha(c) && !is_numeric(c)) || (is_alpha(c) && num_suffix)) {
      phandle->error.cidx = i;
      phandle->error.err_status |= PARSER_ERR_INVALID_HDR;
      status = false;
      break;
    }
    if (is_numeric(c)) {
      num_suffix = true;
    }
    ccnt++;
  }
  if (ccnt < 3) {
    phandle->error.cidx = ccnt;
    phandle->error.err_status |= PARSER_ERR_INVALID_HDR;
    status = false;
  }
  return status;
}

static void
parse_common_cmd(struct parser_handle *phandle,
                 const struct lexer_handle *lhandle) {
  phandle->status |= PARSER_STAT_COMMON;
  uint8_t tcnt = lhandle->tidx + 1;
  if (tcnt > 3) {
    phandle->error.err_status |= PARSER_ERR_UNEXPECTED;
    phandle->error.tidx = 3;
    phandle->status |= PARSER_STAT_ERR;
    return;
  }
  if (!is_valid_hdr(phandle, lhandle->tokens[1].lexeme, lhandle->tokens[1].len)) {
    phandle->error.err_status |= PARSER_ERR_INVALID_HDR;
    phandle->error.tidx = 1;
    phandle->status |= PARSER_STAT_ERR;
    return;
  }
  if (tcnt == 3 && lhandle->tokens[2].type != LEXER_TT_QUERY) {
    phandle->error.err_status |= PARSER_ERR_UNEXPECTED;
    phandle->error.tidx = 2;
    phandle->status |= PARSER_STAT_ERR;
    return;
  }
  if (tcnt == 3 && lhandle->tokens[2].type == LEXER_TT_QUERY) {
    phandle->status |= PARSER_STAT_QUERY;
  }
  memcpy(phandle->command[phandle->cidx].headers[0].token, lhandle->tokens[1].lexeme, lhandle->tokens[1].len);
  phandle->command[phandle->cidx].hidx++;
}

void parser(struct parser_handle *phandle,
            const struct lexer_handle *lhandle) {
  const uint8_t hdr_delim = (1 << 0);
  const uint8_t arg_delim = (1 << 1);
  const uint8_t soa = (1 << 2);
  // handle optional first ':' if not present
  uint8_t pflags = lhandle->tokens[0].type == LEXER_TT_HDR_SEP ? 0x0 : hdr_delim;
  // common command
  if (lhandle->tokens[0].type == SCPI_TOKEN_COMMON) {
    parse_common_cmd(phandle, tokens);
    return;
  }
  phandle->status |= PARSER_STAT_SET;
  for (int i = 0; i < lhandle->tidx + 1; i++) {
    struct lexer_token token = lhandle->tokens[i];
    // handle delimiters
    switch (token.type) {
      case LEXER_TT_QUERY:
        if (!(pflags & hdr_delim)) {
          phandle->status |= PARSER_STAT_QUERY;
          phandle->status &= ~PARSER_STAT_SET;
        } else {
          phandle->status |= PARSER_STAT_ERR;
          phandle->error.tidx = i;
          phandle->error.err_status |= PARSER_ERR_UNEXPECTED;
          return;
        }
      case LEXER_TT_CMD_SEP:
        // CS TODO: unsupported
        phandle->error.tidx = i;
        phandle->error.err_status |= PARSER_ERR_UNSUPPORTED;
        return;
      case LEXER_TT_HDR_SEP:
        if (pflags & hdr_delim || pflags & soa) {
          phandle->status |= PARSER_STAT_ERR;
          phandle->error.tidx = i;
          phandle->error.err_status |= PARSER_ERR_UNEXPECTED;
          return;
        }
        pflags |= hdr_delim;
    }
    if (c == SCPI_TOKEN_QUERY) {
    } else if (c == SCPI_TOKEN_CMD_SEPERATOR) {
    } else if (c == SCPI_TOKEN_HDR_SEPERATOR) {
    } else if (c == SCPI_TOKEN_ARG_SEPERATOR) {
      if (pflags & hdr_delim || !(pflags & soa)) {
        phandle->status |= PARSER_STAT_ERR;
        phandle->error.tidx = i;
        phandle->error.err_status |= PARSER_ERR_UNEXPECTED;
        return;
      }
      pflags |= arg_delim;
    } else if (c == SCPI_TOKEN_SOA) {
      if (pflags & hdr_delim) {
        phandle->status |= PARSER_STAT_ERR;
        phandle->error.tidx = i;
        phandle->error.err_status |= PARSER_ERR_UNEXPECTED;
        return;
      }
      pflags |= soa;
    } else {
      // handle all other tokens
      if (pflags & soa) {
        if (!is_valid_arg(tokens[i], phandle)) {
          phandle->status |= PARSER_STAT_ERR;
          phandle->error.tidx = i;
          return;
        }
        memcpy(phandle->args[phandle->arg_cnt], tokens[i], SCPI_MAX_TOKEN_LEN);
        phandle->arg_cnt++;
        pflags &= ~arg_delim;
      } else {
        if (!is_valid_hdr(tokens[i], phandle)) {
          phandle->status |= PARSER_STAT_ERR;
          phandle->error.tidx = i;
          return;
        }
        memcpy(phandle->headers[phandle->header_cnt], tokens[i], SCPI_MAX_TOKEN_LEN);
        phandle->header_cnt++;
        pflags &= ~hdr_delim;
      }
    }
  }
  return;
}
