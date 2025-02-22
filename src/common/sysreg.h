/**
 * @file sysreg.h
 * @author ztnel (christian911@sympatio.ca)
 * @brief System register API
 * @version 0.1
 * @date 2025-01
 *
 * @copyright Copyright © 2025 dronectl
 *
 */

#ifndef __SYSREG_H__
#define __SYSREG_H__

#include "registers.h"
#include <stdint.h>

enum sysreg_status {
  SYSREG_STATUS_OK,
  SYSREG_STATUS_OP_ERR,
  SYSREG_STATUS_DTYPE_ERR,
  SYSREG_STATUS_MEMORY_ERR,
  SYSREG_STATUS_ACCESS_ERR,
  SYSREG_STATUS_RANGE_ERR,

  SYSREG_STATUS_COUNT
};

enum sysreg_status sysreg_init(void);
enum sysreg_status sysreg_reset(void);
enum sysreg_status sysreg_save(void);
enum sysreg_status sysreg_load(void);
enum sysreg_status sysreg_lock(const enum register_id id);
enum sysreg_status sysreg_unlock(const enum register_id id);
enum sysreg_status sysreg_get_u8(const enum register_id id, uint8_t *data);
enum sysreg_status sysreg_set_u8(const enum register_id id, const uint8_t *data);
enum sysreg_status sysreg_get_u16(const enum register_id id, uint16_t *data);
enum sysreg_status sysreg_set_u16(const enum register_id id, const uint16_t *data);
enum sysreg_status sysreg_get_u32(const enum register_id id, uint32_t *data);
enum sysreg_status sysreg_set_u32(const enum register_id id, const uint32_t *data);
enum sysreg_status sysreg_get_f32(const enum register_id id, float *data);
enum sysreg_status sysreg_set_f32(const enum register_id id, const float *data);
enum sysreg_status sysreg_get_u64(const enum register_id id, uint64_t *data);
enum sysreg_status sysreg_set_u64(const enum register_id id, const uint64_t *data);

#endif // __SYSREG_H__
