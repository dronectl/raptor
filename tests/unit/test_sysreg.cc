#include <gtest/gtest.h>

extern "C"{
#include "sysreg.h"
}

TEST(SysReg, BasicAssertions) {
  uint8_t data = 67;
  EXPECT_EQ(sysreg_get_u8(SYSREG_SYS_STAT, &data), SYSREG_OK) << "getter returned non-zero status code";
  EXPECT_EQ(data, 0);
}

TEST(SysReg, BasicAssertions) {
  float data = 0.0;
  EXPECT_EQ(sysreg_get_f32(SYSREG_SYS_STAT, &data), SYSREG_OK) << "";
  EXPECT_EQ(data, 0);
}
