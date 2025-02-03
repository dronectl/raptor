
#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#define SYSTEM_MAX_TASKS 10
#define SYSTEM_MAX_TASK_NAME_LEN 10

#include <stdint.h>

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

#endif // __SYSTEM_H__
