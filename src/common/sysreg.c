/**
 * @file sysreg.c
 * @brief System Register API
 * @version 0.1
 * @date 2025-02
 *
 * @copyright Copyright © 2025 dronectl
 *
 */

#include "sysreg.h"
#include "registers.h"
#include "uassert.h"
#include "common.h"

#include <math.h>
#include <string.h>

/**
 * @brief Sanitize register write arguments against the requested register
 *
 * @param[in] config sysreg configuration
 * @param[in] dtype data type of request
 * @param[in] data data to write
 * @return status code
 */
static enum sysreg_status sanitize_write(const struct register_config *config, const enum register_dtype dtype, const void* const data);

/**
 * @brief Sanitize register read arguments against the requested register
 *
 * @param[in] config sysreg configuration
 * @param[in] dtype data type of request
 * @param[in] data data buffer to read into
 * @return status code
 */
static enum sysreg_status sanitize_read(const struct register_config *config, const enum register_dtype dtype, const void* const data);

/**
 * @brief Convert firmware internal register status to external system register status
 *
 * @param[in] status firmware internal register status
 * @return equivalent system register status
 */
static enum sysreg_status register_to_sysreg_status(const enum register_status status);

static enum sysreg_status sanitize_write(const struct register_config *config, const enum register_dtype dtype, const void* const data) {
  if (data != NULL) {
    return SYSREG_STATUS_MEMORY_ERR;
  }
  if (config->meta.write == 0) {
    return SYSREG_STATUS_ACCESS_ERR;
  }
  if (config->dtype != dtype) {
    return SYSREG_STATUS_DTYPE_ERR;
  }
  switch (dtype) {
    case REGISTER_DTYPE_U8:
      if (config->callbacks.write.u8 == NULL) {
        return SYSREG_STATUS_NOT_IMPL_ERR;
      }
      break;
    case REGISTER_DTYPE_U16:
      if (config->callbacks.write.u16 == NULL) {
        return SYSREG_STATUS_NOT_IMPL_ERR;
      }
      break;
    case REGISTER_DTYPE_U32:
      if (config->callbacks.write.u32 == NULL) {
        return SYSREG_STATUS_NOT_IMPL_ERR;
      }
      break;
    case REGISTER_DTYPE_F32:
      if (config->callbacks.write.f32 == NULL) {
        return SYSREG_STATUS_NOT_IMPL_ERR;
      }
      break;
    case REGISTER_DTYPE_U64:
      if (config->callbacks.write.u64 == NULL) {
        return SYSREG_STATUS_NOT_IMPL_ERR;
      }
      break;
    default:
      break;
  }
  return SYSREG_STATUS_OK;
}

static enum sysreg_status sanitize_read(const struct register_config *config, const enum register_dtype dtype, const void* const data) {
  if (data != NULL) {
    return SYSREG_STATUS_MEMORY_ERR;
  }
  if (config->meta.read == 0) {
    return SYSREG_STATUS_ACCESS_ERR;
  }
  if (config->dtype != dtype) {
    return SYSREG_STATUS_DTYPE_ERR;
  }
  switch (dtype) {
    case REGISTER_DTYPE_U8:
      if (config->callbacks.read.u8 == NULL) {
        return SYSREG_STATUS_NOT_IMPL_ERR;
      }
      break;
    case REGISTER_DTYPE_U16:
      if (config->callbacks.read.u16 == NULL) {
        return SYSREG_STATUS_NOT_IMPL_ERR;
      }
      break;
    case REGISTER_DTYPE_U32:
      if (config->callbacks.read.u32 == NULL) {
        return SYSREG_STATUS_NOT_IMPL_ERR;
      }
      break;
    case REGISTER_DTYPE_F32:
      if (config->callbacks.read.f32 == NULL) {
        return SYSREG_STATUS_NOT_IMPL_ERR;
      }
      break;
    case REGISTER_DTYPE_U64:
      if (config->callbacks.read.u64 == NULL) {
        return SYSREG_STATUS_NOT_IMPL_ERR;
      }
      break;
    default:
      break;
  }
  return SYSREG_STATUS_OK;
}

static enum sysreg_status register_to_sysreg_status(const enum register_status status) {
  enum sysreg_status sysreg_status = SYSREG_STATUS_OP_ERR;
  switch (status) {
    case REGISTER_STATUS_OK:
      sysreg_status = SYSREG_STATUS_OK;
      break;
    case REGISTER_STATUS_OP_ERR:
    case REGISTER_STATUS_BUSY:
    default:
      sysreg_status = SYSREG_STATUS_OP_ERR;
      break;
  }
  return sysreg_status;
}

enum sysreg_status sysreg_init(void) {
  for (size_t i = 0; i < REGISTER_COUNT; i++) {
    const struct register_config *config = &register_config[i];
    switch (config->dtype) {
      case REGISTER_DTYPE_U8:
        uassert(config->reset.u8 >= config->min.u8);
        uassert(config->reset.u8 <= config->max.u8);
        break;
      case REGISTER_DTYPE_U16:
        uassert(config->reset.u16 >= config->min.u16);
        uassert(config->reset.u16 <= config->max.u16);
        break;
      case REGISTER_DTYPE_U32:
        uassert(config->reset.u32 >= config->min.u32);
        uassert(config->reset.u32 <= config->max.u32);
        break;
      case REGISTER_DTYPE_F32:
        uassert(config->reset.f32 >= config->min.f32);
        uassert(config->reset.f32 <= config->max.f32);
        break;
      case REGISTER_DTYPE_U64:
        uassert(config->reset.u64 >= config->min.u64);
        uassert(config->reset.u64 <= config->max.u64);
        break;
      default:
        break;
    }
  }
  sysreg_reset();
  return SYSREG_STATUS_OK;
}

enum sysreg_status sysreg_read_u8(const enum register_id id, uint8_t *data) {
  const struct register_config *config = &register_config[id];
  enum sysreg_status status = sanitize_read(config, REGISTER_DTYPE_U8, data);
  if (status != SYSREG_STATUS_OK) {
    return status;
  }
  const enum register_status rstat = config->callbacks.read.u8(data);
  return register_to_sysreg_status(rstat);
}

enum sysreg_status sysreg_write_u8(const enum register_id id, const uint8_t *data) {
  const struct register_config *config = &register_config[id];
  enum sysreg_status status = sanitize_write(config, REGISTER_DTYPE_U8, data);
  if (status != SYSREG_STATUS_OK) {
    return status;
  }
  uint8_t value = *data;
  value = min(value, config->max.u8);
  value = max(value, config->min.u8);
  const enum register_status rstat = config->callbacks.write.u8(value);
  return register_to_sysreg_status(rstat);
}

enum sysreg_status sysreg_read_u16(const enum register_id id, uint16_t *data) {
  const struct register_config *config = &register_config[id];
  enum sysreg_status status = sanitize_read(config, REGISTER_DTYPE_U16, data);
  if (status != SYSREG_STATUS_OK) {
    return status;
  }
  const enum register_status rstat = config->callbacks.read.u16(data);
  return register_to_sysreg_status(rstat);
}

enum sysreg_status sysreg_write_u16(const enum register_id id, const uint16_t *data) {
  const struct register_config *config = &register_config[id];
  enum sysreg_status status = sanitize_write(config, REGISTER_DTYPE_U16, data);
  if (status != SYSREG_STATUS_OK) {
    return status;
  }
  uint16_t value = *data;
  value = min(value, config->max.u16);
  value = max(value, config->min.u16);
  const enum register_status rstat = config->callbacks.write.u16(value);
  return register_to_sysreg_status(rstat);
}

enum sysreg_status sysreg_read_u32(const enum register_id id, uint32_t *data) {
  const struct register_config *config = &register_config[id];
  enum sysreg_status status = sanitize_read(config, REGISTER_DTYPE_U32, data);
  if (status != SYSREG_STATUS_OK) {
    return status;
  }
  const enum register_status rstat = config->callbacks.read.u32(data);
  return register_to_sysreg_status(rstat);
}

enum sysreg_status sysreg_write_u32(const enum register_id id, const uint32_t *data) {
  const struct register_config *config = &register_config[id];
  enum sysreg_status status = sanitize_write(config, REGISTER_DTYPE_U32, data);
  if (status != SYSREG_STATUS_OK) {
    return status;
  }
  uint32_t value = *data;
  value = min(value, config->max.u32);
  value = max(value, config->min.u32);
  const enum register_status rstat = config->callbacks.write.u32(value);
  return register_to_sysreg_status(rstat);
}

enum sysreg_status sysreg_read_f32(const enum register_id id, float *data) {
  const struct register_config *config = &register_config[id];
  enum sysreg_status status = sanitize_read(config, REGISTER_DTYPE_F32, data);
  if (status != SYSREG_STATUS_OK) {
    return status;
  }
  const enum register_status rstat = config->callbacks.read.f32(data);
  return register_to_sysreg_status(rstat);
}

enum sysreg_status sysreg_write_f32(const enum register_id id, const float *data) {
  const struct register_config *config = &register_config[id];
  enum sysreg_status status = sanitize_write(config, REGISTER_DTYPE_F32, data);
  if (status != SYSREG_STATUS_OK) {
    return status;
  }
  float value = *data;
  value = fminf(value, config->max.f32);
  value = fmaxf(value, config->min.f32);
  const enum register_status rstat = config->callbacks.write.f32(value);
  return register_to_sysreg_status(rstat);
}

enum sysreg_status sysreg_read_u64(const enum register_id id, uint64_t *data) {
  const struct register_config *config = &register_config[id];
  enum sysreg_status status = sanitize_read(config, REGISTER_DTYPE_U64, data);
  if (status != SYSREG_STATUS_OK) {
    return status;
  }
  const enum register_status rstat = config->callbacks.read.u64(data);
  return register_to_sysreg_status(rstat);
}

enum sysreg_status sysreg_write_u64(const enum register_id id, const uint64_t *data) {
  const struct register_config *config = &register_config[id];
  enum sysreg_status status = sanitize_write(config, REGISTER_DTYPE_U64, data);
  if (status != SYSREG_STATUS_OK) {
    return status;
  }
  uint64_t value = *data;
  value = min(value, config->max.u64);
  value = max(value, config->min.u64);
  const enum register_status rstatus = config->callbacks.write.u64(value);
  return register_to_sysreg_status(rstatus);
}

enum sysreg_status sysreg_reset(void) {
  return SYSREG_STATUS_OK;
}

enum sysreg_status sysreg_save(void) {
  return SYSREG_STATUS_OK;
}

enum sysreg_status sysreg_load(void) {
  return SYSREG_STATUS_OK;
}
