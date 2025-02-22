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

#include <assert.h>
#include <float.h>
#include <stdint.h>
#include <stddef.h>

// clang-format off
const struct register_config register_config[] = {
  [REGISTER_GPU8] = {
    .id = REGISTER_GPU8,
    .offset = offsetof(struct registers, gpu8),
    .dtype = REGISTER_DTYPE_U8,
    .meta = { .read = 1, .write = 1, .nvm = 0, .lock = 0 },
    .reset = {.u8 = 0},
    .min = {.u8 = 0},
    .max = {.u8 = UINT8_MAX}
  },

  [REGISTER_GPU16] = {
    .id = REGISTER_GPU16,
    .offset = offsetof(struct registers, gpu16),
    .dtype = REGISTER_DTYPE_U16,
    .meta = { .read = 1, .write = 1, .nvm = 0, .lock = 0 },
    .reset = {.u16 = 0},
    .min = {.u16 = 0},
    .max = {.u16 = UINT16_MAX}
  },
 
  [REGISTER_GPU32] = {
    .id = REGISTER_GPU32,
    .offset = offsetof(struct registers, gpu32),
    .dtype = REGISTER_DTYPE_U32,
    .meta = { .read = 1, .write = 1, .nvm = 0, .lock = 0 },
    .reset = {.u32 = 0},
    .min = {.u32 = 0},
    .max = {.u32 = UINT32_MAX}
  },

  [REGISTER_GPF32] = {
    .id = REGISTER_GPF32,
    .offset = offsetof(struct registers, gpf32),
    .dtype = REGISTER_DTYPE_F32,
    .meta = { .read = 1, .write = 1, .nvm = 0, .lock = 0 },
    .reset = {.f32 = 0},
    .min= {.f32 = -FLT_MAX},
    .max= {.f32 = FLT_MAX}
  },

  [REGISTER_GPU64] = {
    .id = REGISTER_GPU64,
    .offset = offsetof(struct registers, gpf32),
    .dtype = REGISTER_DTYPE_F32,
    .meta = { .read = 1, .write = 1, .nvm = 0, .lock = 0 },
    .reset = {.f32 = 0},
    .min= {.f32 = -FLT_MAX},
    .max= {.f32 = FLT_MAX}
  },

  [REGISTER_UUID] = {
    .id = REGISTER_UUID,
    .offset = offsetof(struct registers, uuid),
    .dtype = REGISTER_DTYPE_U32,
    .meta = { .read = 1, .write = 0, .nvm = 1, .lock = 1 },
    .reset = {.u32 = 0xDECAFBAD},
    .min = {.u32 = 0},
    .max = {.u32 = UINT32_MAX}
  },

  [REGISTER_SYS_STAT] = {
    .id = REGISTER_SYS_STAT,
    .offset = offsetof(struct registers, sys_stat),
    .dtype = REGISTER_DTYPE_U8,
    .meta = { .read = 1, .write = 0, .nvm = 0, .lock = 0 },
    .reset = {.u8 = 0},
    .min = {.u8 = 0},
    .max = {.u8 = UINT8_MAX}
  },

  [REGISTER_HW_VERSION] = {
    .id = REGISTER_HW_VERSION,
    .offset = offsetof(struct registers, hw_version),
    .dtype = REGISTER_DTYPE_U32,
    .meta = { .read = 1, .write = 0, .nvm = 1, .lock = 1 },
    .reset = {.u32 = 0},
    .min = {.u32 = 0},
    .max = {.u32 = UINT32_MAX}
  },

  [REGISTER_FW_VERSION] = {
    .id = REGISTER_FW_VERSION,
    .offset = offsetof(struct registers, fw_version),
    .dtype = REGISTER_DTYPE_U32,
    .meta = { .read = 1, .write = 0, .nvm = 1, .lock = 1 },
    .reset = {.u32 = 0},
    .min= {.u32 = 0},
    .max= {.u32 = UINT32_MAX}
  },

  [REGISTER_FW_COMMIT_SHA] = {
    .id = REGISTER_FW_COMMIT_SHA,
    .offset = offsetof(struct registers, fw_commit_sha),
    .dtype = REGISTER_DTYPE_U64,
    .meta = { .read = 1, .write = 0, .nvm = 1, .lock = 1 },
    .reset = {.u64 = 0},
    .min= {.u64 = 0},
    .max= {.u64 = UINT64_MAX}
  },

  [REGISTER_IP_ADDR] = {
    .id = REGISTER_IP_ADDR,
    .offset = offsetof(struct registers, ip_addr),
    .dtype = REGISTER_DTYPE_U32,
    .meta = { .read = 1, .write = 1, .nvm = 1, .lock = 0 },
    .reset = {.u32 = 0},
    .min= {.u32 = 0},
    .max= {.u32 = UINT32_MAX}
  },

  [REGISTER_NETMASK_ADDR] = {
    .id = REGISTER_NETMASK_ADDR,
    .offset = offsetof(struct registers, netmask_addr),
    .dtype = REGISTER_DTYPE_U32,
    .meta = { .read = 1, .write = 1, .nvm = 1, .lock = 0 },
    .reset = {.u32 = 0},
    .min= {.u32 = 0},
    .max= {.u32 = UINT32_MAX}
  },

  [REGISTER_GATEWAY_ADDR] = {
    .id = REGISTER_GATEWAY_ADDR,
    .offset = offsetof(struct registers, gateway_addr),
    .dtype = REGISTER_DTYPE_U32,
    .meta = { .read = 1, .write = 1, .nvm = 1, .lock = 0 },
    .reset = {.u32 = 0},
    .min= {.u32 = 0},
    .max= {.u32 = UINT32_MAX}
  },

};
// clang-format on

static_assert(REGISTER_COUNT == (sizeof(register_config) / sizeof(struct register_config)), "register_config size mismatch");
