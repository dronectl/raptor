#include <gtest/gtest.h>

extern "C" {
#include "scpi/lexer.h"
}

TEST(SCPILexerTest, SCPILexerInit) {
  // clang-format off
  struct lexer_handle lexer = {
    .char_index = 4,
    .token_index = 3,
    .status_flags = 4,
    .error_flags = 4,
    .prev_token_type = LEXER_TT_SPACE,
    .tokens={
      {
        .type=LEXER_TT_SPACE,
        .token={
          .len = 2,
          .value=""
        }
      },
      {
        .type=LEXER_TT_TOKEN,
        .token={
          .len = 5,
          .value="dsala"
        }
      }
    }
  };
  // clang-format on
  lexer_init(&lexer);
  for (int i = 0; i < SCPI_MAX_TOKENS; i++) {
    EXPECT_EQ(lexer.tokens[i].token.len, 0) << "token length not cleared at index" << i;
    for (int j = 0; j < SCPI_MAX_TOKEN_LEN; j++) {
      EXPECT_EQ(lexer.tokens[i].token.value[j], '\0') << "token not set to \0 at index" << j;
    }
    EXPECT_EQ(lexer.tokens[i].type, LEXER_TT_NULL) << "token type not set to LEXER_TT_NULL for index" << i;
  }
  EXPECT_EQ(lexer.status_flags, 0) << "status not cleared";
  EXPECT_EQ(lexer.char_index, 0) << "character index not cleared";
  EXPECT_EQ(lexer.token_index, 0) << "token index not cleared";
  EXPECT_EQ(lexer.error_flags, 0) << "status not cleared";
  EXPECT_EQ(lexer.prev_token_type, LEXER_TT_NULL) << "previous token type not cleared";
}

TEST(SCPILexerTest, SCPILexerProcessBasic) {
}
TEST(SCPILexerTest, SCPILexerInit) {
