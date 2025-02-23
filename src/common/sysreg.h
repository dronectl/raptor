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
  // requested operation was successful
  SYSREG_STATUS_OK,
  // requested operation failed due to internal error
  SYSREG_STATUS_OP_ERR,
  // requested operation does not match register data type
  SYSREG_STATUS_DTYPE_ERR,
  // memory access error
  SYSREG_STATUS_MEMORY_ERR,
  // out of memory error
  SYSREG_STATUS_OUT_OF_MEMORY_ERR,
  // requested access is not permitted
  SYSREG_STATUS_ACCESS_ERR,
  // requested write out of range
  SYSREG_STATUS_RANGE_ERR,
  // register callback is not implemented
  SYSREG_STATUS_NOT_IMPL_ERR,

  // reserved as final element
  SYSREG_STATUS_COUNT
};

enum sysreg_status sysreg_init(void);
enum sysreg_status sysreg_factory_reset(void);
enum sysreg_status sysreg_save_nvm(void);
enum sysreg_status sysreg_load_nvm(void);
enum sysreg_status sysreg_read_u8(const enum register_id id, uint8_t *data);
enum sysreg_status sysreg_write_u8(const enum register_id id, const uint8_t *data);
enum sysreg_status sysreg_read_u16(const enum register_id id, uint16_t *data);
enum sysreg_status sysreg_write_u16(const enum register_id id, const uint16_t *data);
enum sysreg_status sysreg_read_u32(const enum register_id id, uint32_t *data);
enum sysreg_status sysreg_write_u32(const enum register_id id, const uint32_t *data);
enum sysreg_status sysreg_read_f32(const enum register_id id, float *data);
enum sysreg_status sysreg_write_f32(const enum register_id id, const float *data);
enum sysreg_status sysreg_read_u64(const enum register_id id, uint64_t *data);
enum sysreg_status sysreg_write_u64(const enum register_id id, const uint64_t *data);

#endif // __SYSREG_H__
