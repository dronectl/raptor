#include <gtest/gtest.h>

extern "C" {
#include "scpi/lexer.h"
}

TEST(SCPILexerTest, SCPILexerInit) {
  // clang-format off
  struct lexer_handle lhandle = {
    .cidx = 4,
    .tidx = 3,
    .status = 4,
    .prev_tt = LEXER_TT_SPACE,
    .tokens={
      {
        .type=LEXER_TT_SPACE,
        .token={
          .len = 2,
          .token=""
        }
      },
      {
        .type=LEXER_TT_TOKEN,
        .token={
          .len = 5,
          .token="dsala"
        }
      }
    }
  };
  // clang-format on
  lexer_init(&lhandle);
  for (int i = 0; i < SCPI_MAX_TOKENS; i++) {
    EXPECT_EQ(lhandle.tokens[i].token.len, 0) << "token length not cleared at index" << i;
    for (int j = 0; j < SCPI_MAX_TOKEN_LEN; j++) {
      EXPECT_EQ(lhandle.tokens[i].token.token[j], '\0') << "token not set to \0 at index" << j;
    }
    EXPECT_EQ(lhandle.tokens[i].type, LEXER_TT_NULL) << "token type not set to LEXER_TT_NULL for index" << i;
  }
  EXPECT_EQ(lhandle.status, 0) << "status not cleared";
  EXPECT_EQ(lhandle.cidx, 0) << "character index not cleared";
  EXPECT_EQ(lhandle.tidx, 0) << "token index not cleared";
  EXPECT_EQ(lhandle.err, 0) << "status not cleared";
  EXPECT_EQ(lhandle.prev_tt, LEXER_TT_NULL) << "previous token type not cleared";
}

TEST(SCPILexerTest, SCPILexerBasic) {
  struct lexer_handle lhandle = {.cidx = 4, .tidx = 3, .status = 4, .prev_tt = LEXER_TT_SPACE};
}
