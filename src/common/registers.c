/**
 * @file registers.c
 * @brief Protected register configuration
 * @version 0.1
 * @date 2025-02
 *
 * @copyright Copyright © 2025 dronectl
 *
 */

#include "registers.h"
#include "system.h"

#include <assert.h>
#include <float.h>
#include <stdint.h>
#include <stddef.h>

// clang-format off
const struct register_config register_config[] = {

  [REGISTER_UUID] = {
    .id = REGISTER_UUID,
    .dtype = REGISTER_DTYPE_U32,
    .meta = { .read = 1, .write = 0, .nvm = 1 },
    .reset.u32 = 0xDECAFBAD,
    .min.u32 = 0,
    .max.u32 = UINT32_MAX,
    .callbacks = {
      .write.u32 = system_write_uuid,
      .read.u32 = system_read_uuid
    }
  },

  [REGISTER_HW_VERSION] = {
    .id = REGISTER_HW_VERSION,
    .dtype = REGISTER_DTYPE_U32,
    .meta = { .read = 1, .write = 0, .nvm = 1 },
    .reset.u32 = 0,
    .min.u32 = 0,
    .max.u32 = UINT32_MAX,
    .callbacks = {
      .write.u32 = system_write_hw_version,
      .read.u32 = system_read_hw_version
    }
  },

  [REGISTER_FW_VERSION] = {
    .id = REGISTER_FW_VERSION,
    .dtype = REGISTER_DTYPE_U32,
    .meta = { .read = 1, .write = 0, .nvm = 1 },
    .reset.u32 = 0,
    .min.u32 = 0,
    .max.u32 = UINT32_MAX,
    .callbacks = {
      .write.u32 = system_write_fw_version,
      .read.u32 = system_read_fw_version
    }
  },

  [REGISTER_FW_COMMIT_SHA] = {
    .id = REGISTER_FW_COMMIT_SHA,
    .dtype = REGISTER_DTYPE_U64,
    .meta = { .read = 1, .write = 0, .nvm = 1 },
    .reset.u64 = 0,
    .min.u64 = 0,
    .max.u64 = UINT64_MAX,
    .callbacks = {
      .write.u64 = system_write_fw_commit_sha,
      .read.u64 = system_read_fw_commit_sha
    }
  },

};
// clang-format on

static_assert(REGISTER_COUNT == (sizeof(register_config) / sizeof(struct register_config)), "register_config size mismatch");
