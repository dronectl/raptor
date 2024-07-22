#include <gtest/gtest.h>

extern "C" {
#include "utf8.h"
}

TEST(UTF8Test, UTF8IsNumeric) {
  EXPECT_EQ(utf8_is_numeric('a'), false) << "utf8_is_numeric returns true for non-numeric";
  EXPECT_EQ(utf8_is_numeric('1'), true) << "utf8_is_numeric returns false for numeric";
}
