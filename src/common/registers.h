  
/**
 * @file registers.h
 * @brief Register public data containers 
 * @version 0.1
 * @date 2025-02
 *
 * @copyright Copyright © 2025 dronectl
 *
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Register identifiers
 * @warning Do not remove / modify the order of the elements.
 */
enum register_id {
  // reserved as first element
  REGISTER_NULL = 0,

  REGISTER_UUID,
  REGISTER_HW_VERSION,
  REGISTER_FW_VERSION,
  REGISTER_FW_COMMIT_SHA,

  // reserved as final element
  REGISTER_COUNT
};

enum register_dtype {
  REGISTER_DTYPE_U8,
  REGISTER_DTYPE_U16,
  REGISTER_DTYPE_U32,
  REGISTER_DTYPE_F32,
  REGISTER_DTYPE_U64,

  // reserved as final element
  REGISTER_DTYPE_COUNT
};

union register_value {
  uint8_t u8;
  uint16_t u16;
  uint32_t u32;
  float f32;
  uint64_t u64;
};

struct register_meta {
  // read access flag 
  uint8_t read:1;
  // write access flag
  uint8_t write:1;
  // non-volatile memory flag
  uint8_t nvm:1;
};

enum register_status {
  // Operation was successful
  REGISTER_STATUS_OK,
  // Failed to complete the operation
  REGISTER_STATUS_OP_ERR,
  // Disallowed the operation due to state
  REGISTER_STATUS_BUSY,

  // reserved as final element
  REGISTER_STATUS_COUNT
};


enum register_dtype {
  REGISTER_DTYPE_U8,
  REGISTER_DTYPE_I8,
  REGISTER_DTYPE_U16,
  REGISTER_DTYPE_I16,
  REGISTER_DTYPE_U32,
  REGISTER_DTYPE_I32,
  REGISTER_DTYPE_F32,
  REGISTER_DTYPE_F64,
  REGISTER_DTYPE_U64,
  REGISTER_DTYPE_I64,

  // reserved as final element
  REGISTER_DTYPE_COUNT
};

union register_value {
  uint8_t u8;
  int8_t i8;
  uint16_t u16;
  int16_t i16;
  uint32_t u32;
  int32_t i32;
  float f32;
  double f64;
  uint64_t u64;
  int64_t i64;
};

struct register_callbacks {
  union {
    enum register_status (*u8)(uint8_t *);
    enum register_status (*i8)(int8_t *);
    enum register_status (*u16)(uint16_t *);
    enum register_status (*i16)(int16_t *);
    enum register_status (*u32)(uint32_t *);
    enum register_status (*i32)(int32_t *);
    enum register_status (*f32)(float *);
    enum register_status (*f64)(double *);
    enum register_status (*u64)(uint64_t *);
    enum register_status (*i64)(int64_t *);
  } read;
  union {
    enum register_status (*u8)(const uint8_t);
    enum register_status (*i8)(const int8_t);
    enum register_status (*u16)(const uint16_t);
    enum register_status (*i16)(const int16_t);
    enum register_status (*u32)(const uint32_t);
    enum register_status (*i32)(const int32_t);
    enum register_status (*f32)(const float);
    enum register_status (*f64)(const double);
    enum register_status (*u64)(const uint64_t);
    enum register_status (*i64)(const int64_t);
  } write;
};

struct register_config {
  const enum register_id id; 
  const struct register_meta meta;
  const enum register_dtype dtype;
  const union register_value reset;
  const union register_value min;   
  const union register_value max;
  const struct register_callbacks callbacks;
};

extern const struct register_config register_config[];
