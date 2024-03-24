#include <gtest/gtest.h>
extern "C"{
#include "sysreg.h"
}
// Demonstrate some basic assertions.
TEST(HelloTest, BasicAssertions) {
  // Expect two strings not to be equal.
  EXPECT_STRNE("hello", "world");
  // Expect equality.
  EXPECT_EQ(7 * 6, 42);
}

TEST(SysReg, BasicAssertions) {
  uint8_t data = 67;
  EXPECT_EQ(sysreg_get_u8(SYSREG_SYS_STAT, &data), SYSREG_OK);
  EXPECT_EQ(data, 0);
}
