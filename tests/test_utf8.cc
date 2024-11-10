#include <gtest/gtest.h>

extern "C" {
#include "scpi/utf8.h"
}

TEST(UTF8Test, UTF8IsNumeric) {
  EXPECT_EQ(utf8_is_numeric('\0'), false) << "utf8_is_numeric returns true for non-numeric";
  EXPECT_EQ(utf8_is_numeric('a'), false) << "utf8_is_numeric returns true for non-numeric";
  for (char c = '0'; c <= '9'; c++) {
    EXPECT_EQ(utf8_is_numeric(c), true) << "utf8_is_numeric returns false for numeric";
  }
}

TEST(UTF8Test, UTF8IsAlpha) {
  EXPECT_EQ(utf8_is_alpha('\0'), false) << "utf8_is_alpha returns true for numeric";
  EXPECT_EQ(utf8_is_alpha('1'), false) << "utf8_is_alpha returns true for numeric";
  for (char c = 'A'; c <= 'Z'; c++) {
    EXPECT_EQ(utf8_is_alpha(c), true) << "utf8_is_alpha returns false for alpha";
  }
  for (char c = 'a'; c <= 'z'; c++) {
    EXPECT_EQ(utf8_is_alpha(c), true) << "utf8_is_alpha returns false for alpha";
  }
}

TEST(UTF8Test, UTF8IsUppercase) {
  EXPECT_EQ(utf8_is_uppercase('\0'), false) << "utf8_is_uppercase returns true for non-alpha";
  EXPECT_EQ(utf8_is_uppercase('a'), false) << "utf8_is_uppercase returns true for lowercase";
  for (char c = 'A'; c <= 'Z'; c++) {
    EXPECT_EQ(utf8_is_uppercase(c), true) << "utf8_is_uppercase returns false for uppercase";
  }
}

TEST(UTF8Test, UTF8IsLowercase) {
  EXPECT_EQ(utf8_is_lowercase('\0'), false) << "utf8_is_lowercase returns true for non-alpha";
  EXPECT_EQ(utf8_is_lowercase('A'), false) << "utf8_is_lowercase returns true for uppercase";
  for (char c = 'a'; c <= 'z'; c++) {
    EXPECT_EQ(utf8_is_lowercase(c), true) << "utf8_is_lowercase returns false for lowercase";
  }
}

TEST(UTF8Test, UTF8UppercaseToLowercase) {
  for (char c = 'A'; c <= 'Z'; c++) {
    EXPECT_EQ(utf8_uppercase_to_lowercase(c), c + 0x20) << "utf8_uppercase_to_lowercase fails to convert uppercase to lowercase equivalent";
  }
  for (char c = 'a'; c <= 'z'; c++) {
    EXPECT_EQ(utf8_uppercase_to_lowercase(c), c) << "utf8_uppercase_to_lowercase fails to return lowercase";
  }
}

TEST(UTF8Test, UTF8LowercaseToUppercase) {
  for (char c = 'a'; c <= 'z'; c++) {
    EXPECT_EQ(utf8_lowercase_to_uppercase(c), c - 0x20) << "utf8_is_lowercase returns true for uppercase";
  }
  for (char c = 'A'; c <= 'Z'; c++) {
    EXPECT_EQ(utf8_lowercase_to_uppercase(c), c) << "utf8_is_lowercase returns true for uppercase";
  }
}
