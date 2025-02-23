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
#include <stm32h7xx_hal.h>

// flash memory as defined in linker script
extern uint32_t _prod_data_start;
extern uint32_t _prod_data_end;
extern uint32_t _user_data_start;
extern uint32_t _user_data_end;

#define PROD_DATA_START   ((uint32_t)&_prod_data_start)
#define PROD_DATA_END     ((uint32_t)&_prod_data_end)
#define USER_DATA_START   ((uint32_t)&_user_data_start)
#define USER_DATA_END     ((uint32_t)&_user_data_end)

#define PROD_DATA_SIZE    (size_t)(PROD_DATA_END - PROD_DATA_START)
#define USER_DATA_SIZE    (size_t)(USER_DATA_END - USER_DATA_START)

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

/**
 * @brief Convert HAL status to system register status
 *
 * @param[in] status HAL status code
 * @return equivalent system register status
 */
static enum sysreg_status hal_to_sysreg_status(const HAL_StatusTypeDef status);

static void read_nvm_register(const struct register_config *config, uint8_t *data, size_t *size);
static enum sysreg_status kv_encode_registers(const size_t user_data_sector_size, uint8_t *encoded_data, size_t *size);

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

static enum sysreg_status hal_to_sysreg_status(const HAL_StatusTypeDef status) {
  enum sysreg_status sysreg_status = SYSREG_STATUS_OP_ERR;
  switch (status) {
    case HAL_OK:
      sysreg_status = SYSREG_STATUS_OK;
      break;
    default:
    case HAL_BUSY:
    case HAL_TIMEOUT:
    case HAL_ERROR:
      sysreg_status = SYSREG_STATUS_OP_ERR;
      break;
  }
  return sysreg_status;
}

static void read_nvm_register(const struct register_config *config, uint8_t *data, size_t *size) {
  switch (config->dtype) {
    case REGISTER_DTYPE_U8:
      {
        uint8_t value = 0;
        config->callbacks.read.u8(&value);
        *size = sizeof(value);
        memcpy(data, &value, sizeof(value));
      }
      break;
    case REGISTER_DTYPE_U16:
      {
        uint16_t value = 0;
        config->callbacks.read.u16(&value);
        *size = sizeof(value);
        memcpy(data, &value, sizeof(value));
      }
      break;
    case REGISTER_DTYPE_U32:
      {
        uint32_t value = 0;
        config->callbacks.read.u32(&value);
        *size = sizeof(value);
        memcpy(data, &value, sizeof(value));
      }
      break;
    case REGISTER_DTYPE_F32:
      {
        float value = 0;
        config->callbacks.read.f32(&value);
        *size = sizeof(value);
        memcpy(data, &value, sizeof(value));
      }
      break;
    case REGISTER_DTYPE_U64:
      {
        uint64_t value = 0;
        config->callbacks.read.u64(&value);
        *size = sizeof(value);
        memcpy(data, &value, sizeof(value));
      }
      break;
    default:
      uassert(0);
      break;
  }
}

static enum sysreg_status kv_encode_registers(const size_t user_data_sector_size, uint8_t *encoded_data, size_t *size) {
  size_t cumulative_size = 0;
  for (enum register_id id = 0; id < REGISTER_COUNT; id++) {
    const struct register_config *config = &register_config[id];
    // skip non-NVM registers
    if (config->meta.nvm == 0) {
      continue;
    }
    const uint16_t key = (uint16_t)config->id;
    uint8_t data[sizeof(uint64_t)];
    size_t register_size = 0;
    read_nvm_register(config, data, &register_size);
    // ensure register will not exceed user data sector size
    if ((cumulative_size + sizeof(key) + register_size) > user_data_sector_size) {
      return SYSREG_STATUS_OUT_OF_MEMORY_ERR;
    }
    // encode key
    encoded_data[id] = key >> 8;
    encoded_data[id+1] = key & 0xFF;
    // encode value based on register key metadata
    for (size_t i = 0; i < register_size; i++) {
      encoded_data[id + sizeof(key) + i] = data[i];
    }
    cumulative_size += sizeof(key) + register_size;
  }

  *size = cumulative_size;
  return SYSREG_STATUS_OK;
}

enum sysreg_status sysreg_init(void) {
  for (size_t i = 0; i < REGISTER_COUNT; i++) {
    const struct register_config *config = &register_config[i];
    switch (config->dtype) {
      case REGISTER_DTYPE_U8:
        uassert(config->meta.nvm ? config->callbacks.read.u8 != NULL : 1);
        uassert(config->reset.u8 >= config->min.u8);
        uassert(config->reset.u8 <= config->max.u8);
        break;
      case REGISTER_DTYPE_U16:
        uassert(config->meta.nvm ? config->callbacks.read.u16 != NULL : 1);
        uassert(config->reset.u16 >= config->min.u16);
        uassert(config->reset.u16 <= config->max.u16);
        break;
      case REGISTER_DTYPE_U32:
        uassert(config->meta.nvm ? config->callbacks.read.u32 != NULL : 1);
        uassert(config->reset.u32 >= config->min.u32);
        uassert(config->reset.u32 <= config->max.u32);
        break;
      case REGISTER_DTYPE_F32:
        uassert(config->meta.nvm ? config->callbacks.read.f32 != NULL : 1);
        uassert(config->reset.f32 >= config->min.f32);
        uassert(config->reset.f32 <= config->max.f32);
        break;
      case REGISTER_DTYPE_U64:
        uassert(config->meta.nvm ? config->callbacks.read.u64 != NULL : 1);
        uassert(config->reset.u64 >= config->min.u64);
        uassert(config->reset.u64 <= config->max.u64);
        break;
      default:
        break;
    }
  }
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

enum sysreg_status sysreg_factory_reset(void) {
  return SYSREG_STATUS_OK;
}

enum sysreg_status sysreg_save_nvm(void) {
  uint32_t encoded_data[USER_DATA_SIZE];
  HAL_StatusTypeDef status = HAL_ERROR;
  size_t size = 0;

  // read and encode all nvm registers into stream
  enum sysreg_status sstat = kv_encode_registers(USER_DATA_SIZE, (uint8_t *)encoded_data, &size);
  if (sstat != SYSREG_STATUS_OK) {
    return sstat;
  }
  status = HAL_FLASH_Unlock();
  if (status != HAL_OK) {
    return hal_to_sysreg_status(status);
  }
  for (int i = 0; i < array_size(encoded_data); i++) {
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, USER_DATA_START, encoded_data[i]);
    if (status != HAL_OK) {
      HAL_FLASH_Lock();
      return hal_to_sysreg_status(status);
    }
  }
  status = HAL_FLASH_Lock();
  if (status != HAL_OK) {
    return hal_to_sysreg_status(status);
  }
  return SYSREG_STATUS_OK;
}

enum sysreg_status sysreg_load_nvm(void) {
  return SYSREG_STATUS_OK;
}
