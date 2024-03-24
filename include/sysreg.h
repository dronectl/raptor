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

typedef struct sysreg_t {
  uint16_t uuid;
  uint8_t sys_stat;
  uint32_t hw_version;
  uint32_t fw_version;
  float setpoint;
} sysreg_t;

/**
 * @brief System register access offsets
 */
#define SYSREG_UUID offsetof(sysreg_t, uuid)
#define SYSREG_SYS_STAT offsetof(sysreg_t, sys_stat)
#define SYSREG_HW_VERSION offsetof(sysreg_t, hw_version)
#define SYSREG_FW_VERSION offsetof(sysreg_t, fw_version)
#define SYSREG_SETPOINT offsetof(sysreg_t, setpoint)

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
