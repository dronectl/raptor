
#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#define SYSTEM_MAX_TASKS 10
#define SYSTEM_MAX_TASK_NAME_LEN 10

#include "registers.h"

#include <stdint.h>
#include <FreeRTOS.h>
#include <task.h>

struct system_context {
  uint32_t uuid;
  uint32_t hw_version;
  uint32_t fw_version;
  uint64_t fw_commit_sha;

  TaskHandle_t system_boostrap;
};

/**
 * @brief Task initialization parameters
 */
struct system_task_context {
  const char name[SYSTEM_MAX_TASK_NAME_LEN];
  const uint16_t priority;
  const uint16_t stack_size;
  const void *init_ctx;
};

typedef void (*system_task_start)(const struct system_task_context*);

struct system_task {
  struct system_task_context task_context;
  const system_task_start start;
};

/**
 * @brief Operating system startup entry
 *
 */
void system_boot(void);

enum register_status system_write_uuid(const uint32_t uuid);
enum register_status system_read_uuid(uint32_t* uuid);
enum register_status system_write_hw_version(const uint32_t hw_version);
enum register_status system_read_hw_version(uint32_t* hw_version);
enum register_status system_write_fw_version(const uint32_t hw_version);
enum register_status system_read_fw_version(uint32_t* fw_version);
enum register_status system_write_fw_commit_sha(const uint64_t hw_version);
enum register_status system_read_fw_commit_sha(uint64_t* fw_commit_sha);

#endif // __SYSTEM_H__
