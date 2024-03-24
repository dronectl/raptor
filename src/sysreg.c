/**
 * @file sysreg.c
 * @author ztnel (christian911@sympatio.ca)
 * @brief System Register API
 * @version 0.1
 * @date 2024-03
 *
 * @copyright Copyright © 2024 dronectl
 *
 */

#include "sysreg.h"
#include <math.h>
#include <assert.h>
#include <string.h>

/**
 * @brief System register data type support
 */
typedef union {
  uint8_t u8;
  uint16_t u16;
  uint32_t u32;
  float f32;
} value_t;

typedef enum {
  DTYPE_U8,
  DTYPE_U16,
  DTYPE_U32,
  DTYPE_F32,
} dtype_t;

static sysreg_t registers = {0};

typedef struct reg_conf_t {
  size_t offset; // register offset
  uint8_t access; // bitfield
  dtype_t dtype; // data type
  value_t reset; // value on reset
  value_t min; // minimum value
  value_t max; // maximum value
} reg_conf_t;

// clang-format off
static reg_conf_t register_config[] = {
  {
    .offset = SYSREG_SYS_STAT,
    .dtype=DTYPE_U8,
    .access = SYSREG_ACCESS_R | SYSREG_ACCESS_W,
    .reset = {.u8 = 0},
    .min={.u8 = 0},
    .max={.u8=100}
  },
  {
    .offset = SYSREG_UUID,
    .dtype=DTYPE_U16,
    .access = SYSREG_ACCESS_W,
    .reset = {.u16 = 0},
    .min={.u16 = 0},
    .max={.u16=1000}
  },
  {
    .offset = SYSREG_HW_VERSION,
    .dtype=DTYPE_U16,
    .access = SYSREG_ACCESS_W,
    .reset = {.u16 = 0},
    .min={.u16 = 0},
    .max={.u16=1000}
  },
  {
    .offset = SYSREG_FW_VERSION,
    .dtype=DTYPE_U16,
    .access = SYSREG_ACCESS_W,
    .reset = {.u16 = 0},
    .min={.u16 = 0},
    .max={.u16=1000}
  },
  {
    .offset = SYSREG_SETPOINT,
    .dtype=DTYPE_F32,
    .access = SYSREG_ACCESS_L | SYSREG_ACCESS_R,
    .reset = {.f32 = 0.0f},
    .min={.f32=-4.0f},
    .max={.f32=500.0f}
  },
};
// clang-format on

#define SYSREG_NUM_REGISTERS sizeof(register_config) / sizeof(reg_conf_t)

static sysreg_status_t _get_reg_config(size_t offset, reg_conf_t *config);
static sysreg_status_t _verify_write(reg_conf_t *config, dtype_t dtype);
static sysreg_status_t _verify_read(reg_conf_t *config, dtype_t dtype);

static sysreg_status_t _get_reg_config(size_t offset, reg_conf_t *config) {
  config = NULL;
  sysreg_status_t status = SYSREG_OK;
  for (size_t i = 0; i < SYSREG_NUM_REGISTERS; i++) {
    if (offset == register_config[i].offset) {
      config = &register_config[i];
      break;
    }
  }
  if (config == NULL) {
    status = SYSREG_NOT_FOUND_ERR;
  }
  return status;
}

static sysreg_status_t _verify_write(reg_conf_t *config, dtype_t dtype) {
  if (!(config->access & SYSREG_ACCESS_W)) {
    return SYSREG_ACCESS_ERR;
  }
  if (config->dtype != dtype){
    return SYSREG_DTYPE_ERR;
  }
  return SYSREG_OK;
}

static sysreg_status_t _verify_read(reg_conf_t *config, dtype_t dtype) {
  if (!(config->access & SYSREG_ACCESS_R)) {
    return SYSREG_ACCESS_ERR;
  }
  if (config->dtype != dtype){
    return SYSREG_DTYPE_ERR;
  }
  return SYSREG_OK;
}

sysreg_status_t sysreg_init(void) {
  for (size_t i = 0; i < SYSREG_NUM_REGISTERS; i++) {
    reg_conf_t config = register_config[i];
    switch (config.dtype) {
      case DTYPE_U8:
        assert(config.reset.u8 >= config.min.u8);
        assert(config.reset.u8 <= config.max.u8);
        break;
      case DTYPE_U16:
        assert(config.reset.u16 >= config.min.u16);
        assert(config.reset.u16 <= config.max.u16);
        break;
      case DTYPE_U32:
        assert(config.reset.u32 >= config.min.u32);
        assert(config.reset.u32 <= config.max.u32);
        break;
      case DTYPE_F32:
        assert(config.reset.f32 >= config.min.f32);
        assert(config.reset.f32 <= config.max.f32);
        break;
      default:
        break;
    }
  }
  sysreg_reset();
  // CS TODO: write reset values to registers
  return SYSREG_OK;
}

sysreg_status_t sysreg_reset(void) {
  for (size_t i = 0; i < SYSREG_NUM_REGISTERS; i++) {
    size_t size = 0;
    void *value = NULL;
    reg_conf_t config = register_config[i];
    switch (config.dtype) {
      case DTYPE_U8:
        value = &config.reset.u8;
        size = sizeof(uint8_t);
        break;
      case DTYPE_U16:
        value = &config.reset.u16;
        size = sizeof(uint16_t);
        break;
      case DTYPE_U32:
        value = &config.reset.u32;
        size = sizeof(uint32_t);
        break;
      case DTYPE_F32:
        value = &config.reset.f32;
        size = sizeof(float_t);
        break;
      default:
        break;
    }
    if (size > 0) {
      memcpy(&registers + config.offset, value, size);
    }
  }
  return SYSREG_OK;
}

sysreg_status_t sysreg_set_access(size_t offset, uint8_t access) {
  sysreg_status_t status;
  reg_conf_t *config = NULL;
  status = _get_reg_config(offset, config);
  if (status != SYSREG_OK) {
    return status;
  }
  // check if register access config is locked
  if (config->access & SYSREG_ACCESS_L) {
    return SYSREG_ACCESS_ERR;
  }
  // do not allow write of L bit
  if (access & SYSREG_ACCESS_L) {
    return SYSREG_OP_ERR;
  }
  config->access = access;
  return SYSREG_OK;
}

sysreg_status_t sysreg_get_u8(size_t offset, uint8_t *data) {
  sysreg_status_t status;
  reg_conf_t *config = NULL;
  status = _get_reg_config(offset, config);
  if (status != SYSREG_OK) {
    return status;
  }
  status = _verify_read(config, DTYPE_U8);
  if (status != SYSREG_OK) {
    return status;
  }
  memcpy(data, &registers + offset, sizeof(uint8_t));
  return SYSREG_OK;
}

sysreg_status_t sysreg_set_u8(size_t offset, const uint8_t *data) {
  sysreg_status_t status;
  reg_conf_t *config = NULL;
  status = _get_reg_config(offset, config);
  if (status != SYSREG_OK) {
    return status;
  }
  status = _verify_write(config, DTYPE_U8);
  if (status != SYSREG_OK) {
    return status;
  }
  uint8_t value = *data;
  if (value < config->min.u8) {
    value = config->min.u8;
  } else if (value > config->max.u8) {
    value = config->max.u8;
  }
  memcpy(&registers + offset, &value, sizeof(float));
  return SYSREG_OK;
}

sysreg_status_t sysreg_get_u16(size_t offset, uint16_t *data) {
  sysreg_status_t status;
  reg_conf_t *config = NULL;
  status = _get_reg_config(offset, config);
  if (status != SYSREG_OK) {
    return status;
  }
  status = _verify_read(config, DTYPE_U16);
  if (status != SYSREG_OK) {
    return status;
  }
  memcpy(data, &registers + offset, sizeof(uint16_t));
  return SYSREG_OK;
}

sysreg_status_t sysreg_set_u16(size_t offset, const uint16_t *data) {
  sysreg_status_t status;
  reg_conf_t *config = NULL;
  status = _get_reg_config(offset, config);
  if (status != SYSREG_OK) {
    return status;
  }
  status = _verify_write(config, DTYPE_U16);
  if (status != SYSREG_OK) {
    return status;
  }
  uint16_t value = *data;
  if (value < config->min.u16) {
    value = config->min.u16;
  } else if (value > config->max.u16) {
    value = config->max.u16;
  }
  memcpy(&registers + offset, &value, sizeof(uint16_t));
  return SYSREG_OK;
}

sysreg_status_t sysreg_get_u32(size_t offset, uint32_t *data) {
  sysreg_status_t status;
  reg_conf_t *config = NULL;
  status = _get_reg_config(offset, config);
  if (status != SYSREG_OK) {
    return status;
  }
  status = _verify_read(config, DTYPE_U32);
  if (status != SYSREG_OK) {
    return status;
  }
  memcpy(data, &registers + offset, sizeof(uint32_t));
  return SYSREG_OK;
}

sysreg_status_t sysreg_set_u32(size_t offset, const uint32_t *data) {
  sysreg_status_t status;
  reg_conf_t *config = NULL;
  status = _get_reg_config(offset, config);
  if (status != SYSREG_OK) {
    return status;
  }
  status = _verify_write(config, DTYPE_U16);
  if (status != SYSREG_OK) {
    return status;
  }
  uint16_t value = *data;
  if (value < config->min.u32) {
    value = config->min.u32;
  } else if (value > config->max.u32) {
    value = config->max.u32;
  }
  memcpy(&registers + offset, &value, sizeof(uint32_t));
  return SYSREG_OK;
}

sysreg_status_t sysreg_get_f32(size_t offset, float *data) {
  sysreg_status_t status;
  reg_conf_t *config = NULL;
  status = _get_reg_config(offset, config);
  if (status != SYSREG_OK) {
    return status;
  }
  status = _verify_read(config, DTYPE_F32);
  if (status != SYSREG_OK) {
    return status;
  }
  memcpy(data, &registers + offset, sizeof(float));
  return SYSREG_OK;
}

sysreg_status_t sysreg_set_f32(size_t offset, const float *data) {
  sysreg_status_t status;
  reg_conf_t *config = NULL;
  status = _get_reg_config(offset, config);
  if (status != SYSREG_OK) {
    return status;
  }
  status = _verify_write(config, DTYPE_F32);
  if (status != SYSREG_OK) {
    return status;
  }
  float value = *data;
  value = fminf(value, config->max.f32);
  value = fmaxf(value, config->min.f32);
  memcpy(&registers + offset, &value, sizeof(float));
  return SYSREG_OK;
}
