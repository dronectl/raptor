/**
 * @file system.h
 * @brief System core control module. This module is responsible for spawning the system tasks and distributing hardware resources.
 * @version 2.0
 * @date 2025-02
 *
 * @copyright Copyright © 2025 dronectl
 *
 */

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#define SYSTEM_MAX_TASKS 3
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
 * @brief Operating system startup genesis process
 *
 */
void system_boot(void);

/**
 * @brief Write the device UUID to the system context
 *
 * @param[in] uuid device UUID
 * @return status of the operation
 */
enum register_status system_write_uuid(const uint32_t uuid);

/**
 * @brief Read the device UUID from the system context
 *
 * @param[out] uuid device UUID
 * @return status of the operation
 */
enum register_status system_read_uuid(uint32_t* uuid);

/**
 * @brief Write the hardware version to the system context
 *
 * @param[in] hw_version hardware version
 * @return status of the operation
 */
enum register_status system_write_hw_version(const uint32_t hw_version);

/**
 * @brief Read the hardware version from the system context
 *
 * @param[out] hw_version hardware version
 * @return status of the operation
 */
enum register_status system_read_hw_version(uint32_t* hw_version);

/**
 * @brief Write the firmware version to the system context
 *
 * @param[in] fw_version firmware version
 * @return operation status
 */
enum register_status system_write_fw_version(const uint32_t hw_version);

/**
 * @brief Read the firmware version from the system context
 *
 * @param[out] fw_version firmware version
 * @return operation status
 */
enum register_status system_read_fw_version(uint32_t* fw_version);

/**
 * @brief Write the firmware commit SHA to the system context
 *
 * @param[in] fw_commit_sha firmware commit SHA
 * @return operation status
 */
enum register_status system_write_fw_commit_sha(const uint64_t fw_commit_sha);

/**
 * @brief Read the firmware commit SHA from the system context
 *
 * @param[out] fw_commit_sha firmware commit SHA
 * @return operation status
 */
enum register_status system_read_fw_commit_sha(uint64_t* fw_commit_sha);

#endif // __SYSTEM_H__
