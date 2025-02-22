/**
 * @file registers.h
 * @brief Register public data containers 
 * @version 0.1
 * @date 2025-02
 *
 * @copyright Copyright © 2025 dronectl
 *
 */

#ifndef __REGISTERS_H__
#define __REGISTERS_H__

#include <stdint.h>
#include <stddef.h>

union register_value {
  uint8_t u8;
  uint16_t u16;
  uint32_t u32;
  float f32;
  uint64_t u64;
};

struct register_meta {
  uint8_t read:1;
  uint8_t write:1;
  uint8_t nvm:1;
  uint8_t lock:1;
};

enum register_dtype {
  REGISTER_DTYPE_U8,
  REGISTER_DTYPE_U16,
  REGISTER_DTYPE_U32,
  REGISTER_DTYPE_F32,
  REGISTER_DTYPE_U64,

  REGISTER_DTYPE_COUNT
};

enum register_id {
  // device UUID
  REGISTER_UUID,
  // device status register
  REGISTER_SYS_STAT,

  REGISTER_HW_VERSION,
  REGISTER_FW_VERSION,
  REGISTER_FW_COMMIT_SHA,

  REGISTER_IP_ADDR,
  REGISTER_NETMASK_ADDR,
  REGISTER_GATEWAY_ADDR,

  REGISTER_GPU8,
  REGISTER_GPU16,
  REGISTER_GPU32,
  REGISTER_GPF32,
  REGISTER_GPU64,

  REGISTER_COUNT
};

struct register_config {
  // register id
  const enum register_id id; 
  // register offset
  const size_t offset; 
  // properties
  struct register_meta meta;
  // data type
  const enum register_dtype dtype;
  // value on reset
  const union register_value reset;
  // minimum value
  const union register_value min;   
  // maximum value
  const union register_value max;
};

struct registers {
  uint32_t uuid;
  uint8_t sys_stat;

  uint32_t hw_version;
  uint32_t fw_version;
  uint64_t fw_commit_sha;

  uint32_t ip_addr;
  uint32_t netmask_addr;
  uint32_t gateway_addr;

  uint8_t gpu8;
  uint16_t gpu16;
  uint32_t gpu32;
  float gpf32;
  float gpu64;
};

extern const struct register_config register_config[];

#endif // __REGISTERS_H__
