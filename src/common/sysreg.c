/**
 * @file sysreg.c
 * @brief System Register API
 * @version 0.1
 * @date 2024-03
 *
 * @copyright Copyright © 2025 dronectl
 *
 */

#include "sysreg.h"
#include "uassert.h"
#include "common.h"

#include <math.h>
#include <string.h>

// protected register struct
static struct registers registers = {0};

/**
 * @brief Sanitize register write at offset is permitted by type and access checks
 *
 * @param[in] config sysreg configuration
 * @param[in] dtype data type of request
 * @return status code
 */
static enum sysreg_status sanitize_write(const struct register_config *config, const enum register_dtype dtype);

/**
 * @brief Sanitize register read at offset is permitted by type and access checks
 *
 * @param[in] config sysreg configuration
 * @param[in] dtype data type of request
 * @return status code
 */
static enum sysreg_status sanitize_read(const struct register_config *config, const enum register_dtype dtype);

static enum sysreg_status sanitize_write(const struct register_config *config, const enum register_dtype dtype) {
  if (config->meta.write == 0) {
    return SYSREG_STATUS_ACCESS_ERR;
  }
  if (config->dtype != dtype) {
    return SYSREG_STATUS_DTYPE_ERR;
  }
  return SYSREG_STATUS_OK;
}

static enum sysreg_status sanitize_read(const struct register_config *config, const enum register_dtype dtype) {
  if (config->meta.read == 0) {
    return SYSREG_STATUS_ACCESS_ERR;
  }
  if (config->dtype != dtype) {
    return SYSREG_STATUS_DTYPE_ERR;
  }
  return SYSREG_STATUS_OK;
}

enum sysreg_status sysreg_init(void) {
  for (size_t i = 0; i < REGISTER_COUNT; i++) {
    struct register_config *config = &register_config[i];
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

enum sysreg_status sysreg_reset(void) {
  for (size_t i = 0; i < REGISTER_COUNT; i++) {
    size_t size = 0;
    const void *value = NULL;
    struct register_config *config = &register_config[i];
    switch (config->dtype) {
      case REGISTER_DTYPE_U8:
        value = &config->reset.u8;
        size = sizeof(uint8_t);
        break;
      case REGISTER_DTYPE_U16:
        value = &config->reset.u16;
        size = sizeof(uint16_t);
        break;
      case REGISTER_DTYPE_U32:
        value = &config->reset.u32;
        size = sizeof(uint32_t);
        break;
      case REGISTER_DTYPE_F32:
        value = &config->reset.f32;
        size = sizeof(float);
        break;
      case REGISTER_DTYPE_U64:
        value = &config->reset.u64;
        size = sizeof(uint64_t);
        break;
      default:
        break;
    }
    if (size > 0) {
      memcpy((uint8_t *)&registers + config->offset, value, size);
    }
  }
  return SYSREG_STATUS_OK;
}

enum sysreg_status sysreg_lock(const enum register_id id) {
  struct register_config *config = &register_config[id];
  config->meta.lock = 1;
  return SYSREG_STATUS_OK;
}

enum sysreg_status sysreg_unlock(const enum register_id id) {
  struct register_config *config = &register_config[id];
  config->meta.lock = 0;
  return SYSREG_STATUS_OK;
}

enum sysreg_status sysreg_get_u8(const enum register_id id, uint8_t *data) {
  struct register_config *config = &register_config[id];
  enum sysreg_status status = sanitize_read(config, REGISTER_DTYPE_U8);
  if (status != SYSREG_STATUS_OK) {
    return status;
  }
  memcpy(data, (uint8_t *)&registers + config->offset, sizeof(uint8_t));
  return SYSREG_STATUS_OK;
}

enum sysreg_status sysreg_set_u8(const enum register_id id, const uint8_t *data) {
  const struct register_config *config = &register_config[id];
  enum sysreg_status status = sanitize_write(config, REGISTER_DTYPE_U8);
  if (status != SYSREG_STATUS_OK) {
    return status;
  }
  uint8_t value = *data;
  value = min(value, config->max.u8);
  value = max(value, config->min.u8);
  memcpy((uint8_t *)&registers + config->offset, &value, sizeof(uint8_t));
  return SYSREG_STATUS_OK;
}

enum sysreg_status sysreg_get_u16(const enum register_id id, uint16_t *data) {
  const struct register_config *config = &register_config[id];
  enum sysreg_status status = sanitize_read(config, REGISTER_DTYPE_U16);
  if (status != SYSREG_STATUS_OK) {
    return status;
  }
  memcpy(data, (uint8_t *)&registers + config->offset, sizeof(uint16_t));
  return SYSREG_STATUS_OK;
}

enum sysreg_status sysreg_set_u16(const enum register_id id, const uint16_t *data) {
  const struct register_config *config = &register_config[id];
  enum sysreg_status status = sanitize_write(config, REGISTER_DTYPE_U16);
  if (status != SYSREG_STATUS_OK) {
    return status;
  }
  uint16_t value = *data;
  value = min(value, config->max.u16);
  value = max(value, config->min.u16);
  memcpy((uint8_t *)&registers + config->offset, &value, sizeof(uint16_t));
  return SYSREG_STATUS_OK;
}

enum sysreg_status sysreg_get_u32(const enum register_id id, uint32_t *data) {
  struct register_config *config = &register_config[id];
  enum sysreg_status status = sanitize_read(config, REGISTER_DTYPE_U32);
  if (status != SYSREG_STATUS_OK) {
    return status;
  }
  memcpy(data, (uint8_t *)&registers + config->offset, sizeof(uint32_t));
  return SYSREG_STATUS_OK;
}

enum sysreg_status sysreg_set_u32(const enum register_id id, const uint32_t *data) {
  struct register_config *config = &register_config[id];
  enum sysreg_status status = sanitize_write(config, REGISTER_DTYPE_U32);
  if (status != SYSREG_STATUS_OK) {
    return status;
  }
  uint32_t value = *data;
  value = min(value, config->max.u32);
  value = max(value, config->min.u32);
  memcpy((uint8_t *)&registers + config->offset, &value, sizeof(uint32_t));
  return SYSREG_STATUS_OK;
}

enum sysreg_status sysreg_get_f32(const enum register_id id, float *data) {
  struct register_config *config = &register_config[id];
  enum sysreg_status status = sanitize_read(config, REGISTER_DTYPE_F32);
  if (status != SYSREG_STATUS_OK) {
    return status;
  }
  memcpy(data, (uint8_t *)&registers + config->offset, sizeof(float));
  return SYSREG_STATUS_OK;
}

enum sysreg_status sysreg_set_f32(const enum register_id id, const float *data) {
  struct register_config *config = &register_config[id];
  enum sysreg_status status = sanitize_write(config, REGISTER_DTYPE_F32);
  if (status != SYSREG_STATUS_OK) {
    return status;
  }
  float value = *data;
  value = fminf(value, config->max.f32);
  value = fmaxf(value, config->min.f32);
  memcpy((uint8_t *)&registers + config->offset, &value, sizeof(float));
  return SYSREG_STATUS_OK;
}
