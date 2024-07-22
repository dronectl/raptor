#include <gtest/gtest.h>

extern "C" {
#include "sysreg.h"
}

TEST(SysRegTest, SysRegInit) {
  EXPECT_EQ(sysreg_init(), SYSREG_OK) << "init returned non-zero status code";
  uint8_t sys_stat;
  sysreg_get_u8(SYSREG_SYS_STAT, &sys_stat);
  EXPECT_EQ(sys_stat, SYSREG_SYS_STAT_RESET) << "failed to reset SYS_STAT";
  uint32_t uuid;
  sysreg_get_u32(SYSREG_UUID, &uuid);
  EXPECT_EQ(uuid, SYSREG_UUID_RESET) << "failed to reset UUID";
  uint32_t hw_version;
  sysreg_get_u32(SYSREG_HW_VERSION, &hw_version);
  EXPECT_EQ(hw_version, SYSREG_HW_VERSION_RESET) << "failed to reset HW_VERSION";
  uint32_t fw_version;
  sysreg_get_u32(SYSREG_FW_VERSION, &fw_version);
  EXPECT_EQ(fw_version, SYSREG_FW_VERSION_RESET) << "failed to reset FW_VERSION";
}

TEST(SysRegTest, SysRegSetAccess) {
  EXPECT_EQ(sysreg_set_access(SYSREG_GPU8, SYSREG_ACCESS_L), SYSREG_OP_ERR) << "lock bit set did not return SYSREG_OP_ERR";
  EXPECT_EQ(sysreg_set_access(999999, SYSREG_ACCESS_R), SYSREG_NOT_FOUND_ERR) << "invalid offset did not return SYSREG_NOT_FOUND_ERR";
  EXPECT_EQ(sysreg_set_access(SYSREG_GPU8, SYSREG_ACCESS_R), SYSREG_ACCESS_ERR) << "locked register access modification did not return SYSREG_ACCESS_ERR";
  EXPECT_EQ(sysreg_set_access(SYSREG_GPU8_UL, SYSREG_ACCESS_R), SYSREG_OK) << "unlocked register access modification did not return SYSREG_OK";
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
