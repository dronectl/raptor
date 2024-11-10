/**
 * @file sysreg.h
 * @author ztnel (christian911@sympatio.ca)
 * @brief System register API
 * @version 0.1
 * @date 2024-03
 *
 * @copyright Copyright © 2024 dronectl
 *
 */

#ifndef __SYSREG_H__
#define __SYSREG_H__

#include <stdint.h> // IWYU pragma: export
#include <stddef.h>

typedef int sysreg_status_t;

#define SYSREG_OK (sysreg_status_t)0
#define SYSREG_OP_ERR (sysreg_status_t)1
#define SYSREG_NOT_FOUND_ERR (sysreg_status_t)2
#define SYSREG_DTYPE_ERR (sysreg_status_t)3
#define SYSREG_MEMORY_ERR (sysreg_status_t)4
#define SYSREG_ACCESS_ERR (sysreg_status_t)5
#define SYSREG_RANGE_ERR (sysreg_status_t)6

/**
 * @brief Semantic Versioning encoding 4 bytes
 */
#define SYSREG_SEMVER_MAJOR_OFFSET 24
#define SYSREG_SEMVER_MINOR_OFFSET 16
#define SYSREG_SEMVER_PATCH_OFFSET 8
#define SYSREG_SEMVER_REVISION_OFFSET 0
#define SYSREG_SEMVER_MAJOR_MSK (0xFF << SYSREG_SEMVER_MAJOR_OFFSET)
#define SYSREG_SEMVER_MINOR_MSK (0xFF << SYSREG_SEMVER_MINOR_OFFSET)
#define SYSREG_SEMVER_PATCH_MSK (0xFF << SYSREG_SEMVER_PATCH_OFFSET)
#define SYSREG_SEMVER_REVISION_MSK (0xFF << SYSREG_SEMVER_REVISION_OFFSET)

/**
 * @brief System register access API
 *
 *   7   6   5   4   3   2   1   0
 *   ^                       ^   ^
 *   L                       W   R
 */
#define SYSREG_ACCESS_R_OFFSET 0 // read bit
#define SYSREG_ACCESS_W_OFFSET 1 // write bit
#define SYSREG_ACCESS_L_OFFSET 7 // lock bit (access cannot be modified)
#define SYSREG_ACCESS_R (1 << SYSREG_ACCESS_R_OFFSET)
#define SYSREG_ACCESS_W (1 << SYSREG_ACCESS_W_OFFSET)
#define SYSREG_ACCESS_L (1 << SYSREG_ACCESS_L_OFFSET)
// one or more errors exist in error FIFO queue
#define SYSREG_STB_ERR_QUEUE (1 << 2)

typedef struct sysreg_t {
  uint8_t gpu8;      // general purpose u8 register
  uint8_t gpu8_ul;   // general purpose u8 register (unlocked)
  uint16_t gpu16;    // general purpose u16 register
  uint16_t gpu16_ul; // general purpose u16 register (unlocked)
  uint32_t gpu32;    // general purpose u32 register
  uint32_t gpu32_ul; // general purpose u32 register (unlocked)
  float gpf32;       // general purpose f32 register
  float gpf32_ul;    // general purpose f32 register (unlocked)
  uint32_t uuid;
  uint8_t sys_stat;
  uint8_t stb; // status byte register (IEEE 488.2)
  uint32_t hw_version;
  uint32_t fw_version;
  float setpoint;
} sysreg_t;

/**
 * @brief System register access offsets
 */
#define SYSREG_GPU8 offsetof(sysreg_t, gpu8)
#define SYSREG_GPU8_UL offsetof(sysreg_t, gpu8_ul)
#define SYSREG_GPU16 offsetof(sysreg_t, gpu16)
#define SYSREG_GPU16_UL offsetof(sysreg_t, gpu16_ul)
#define SYSREG_GPU32 offsetof(sysreg_t, gpu32)
#define SYSREG_GPU32_UL offsetof(sysreg_t, gpu32_ul)
#define SYSREG_GPF32 offsetof(sysreg_t, gpf32)
#define SYSREG_GPF32_UL offsetof(sysreg_t, gpf32_ul)
#define SYSREG_UUID offsetof(sysreg_t, uuid)
#define SYSREG_SYS_STAT offsetof(sysreg_t, sys_stat)
#define SYSREG_STB offsetof(sysreg_t, stb)
#define SYSREG_HW_VERSION offsetof(sysreg_t, hw_version)
#define SYSREG_FW_VERSION offsetof(sysreg_t, fw_version)
#define SYSREG_SETPOINT offsetof(sysreg_t, setpoint)

/**
 * @brief System register reset
 */
#define SYSREG_UUID_RESET (uint32_t)0xDECAFBAD
#define SYSREG_STB_RESET (uint8_t)0x0
#define SYSREG_SYS_STAT_RESET (uint8_t)0x0
#define SYSREG_HW_VERSION_RESET (uint32_t)0x10000 // v0.1.0
#define SYSREG_FW_VERSION_RESET (uint32_t)0x10000 // v0.1.0

/**
 * @brief Sanitize register reset values are in min/max and set registers to default.
 * Registers without a default are set to 0 by zero initialization
 *
 * @return status code
 */
sysreg_status_t sysreg_init(void);
sysreg_status_t sysreg_reset(void);
sysreg_status_t sysreg_set_access(size_t offset, uint8_t access);
sysreg_status_t sysreg_get_u8(size_t offset, uint8_t *data);
sysreg_status_t sysreg_set_u8(size_t offset, const uint8_t *data);
sysreg_status_t sysreg_get_u16(size_t offset, uint16_t *data);
sysreg_status_t sysreg_set_u16(size_t offset, const uint16_t *data);
sysreg_status_t sysreg_get_u32(size_t offset, uint32_t *data);
sysreg_status_t sysreg_set_u32(size_t offset, const uint32_t *data);
sysreg_status_t sysreg_get_f32(size_t offset, float *data);
sysreg_status_t sysreg_set_f32(size_t offset, const float *data);

#endif // __SYSREG_H__
