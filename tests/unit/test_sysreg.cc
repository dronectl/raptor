#include <gtest/gtest.h>

extern "C"{
#include "sysreg.h"
}

TEST(SysRegTest, SysRegU8) {
  const uint8_t data = 255;
  uint8_t buffer;
  EXPECT_EQ(sysreg_set_u8(SYSREG_GPU8, &data), SYSREG_OK) << "setter returned non-zero status code";
  EXPECT_EQ(sysreg_get_u8(SYSREG_GPU8, &buffer), SYSREG_OK) << "getter returned non-zero status code";
  EXPECT_EQ(buffer, 255) << "Register R/W failed";
}

TEST(SysRegTest, SysRegU16) {
  const uint16_t data = 65535;
  uint16_t buffer;
  EXPECT_EQ(sysreg_set_u16(SYSREG_GPU16, &data), SYSREG_OK) << "setter returned non-zero status code";
  EXPECT_EQ(sysreg_get_u16(SYSREG_GPU16, &buffer), SYSREG_OK) << "getter returned non-zero status code";
  EXPECT_EQ(buffer, 65535) << "Register R/W failed";
}

TEST(SysRegTest, SysRegU32) {
  const uint32_t data = 4294967295;
  uint32_t buffer;
  EXPECT_EQ(sysreg_set_u32(SYSREG_GPU32, &data), SYSREG_OK) << "setter returned non-zero status code";
  EXPECT_EQ(sysreg_get_u32(SYSREG_GPU32, &buffer), SYSREG_OK) << "getter returned non-zero status code";
  EXPECT_EQ(buffer, 4294967295) << "Register R/W failed";
}

TEST(SysRegTest, SysRegF32) {
  const float data = -1.0;
  float buffer;
  EXPECT_EQ(sysreg_set_f32(SYSREG_GPF32, &data), SYSREG_OK) << "setter returned non-zero status code";
  EXPECT_EQ(sysreg_get_f32(SYSREG_GPF32, &buffer), SYSREG_OK) << "getter returned non-zero status code";
  EXPECT_EQ(buffer, -1.0) << "Register R/W failed";
}
