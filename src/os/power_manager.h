
#ifndef __POWER_MANAGER_H__
#define __POWER_MANAGER_H__

struct power_manager_init_ctx {
  void *null;
};

void power_manager_init(const struct power_manager_init_ctx *init_ctx);

#endif // __POWER_MANAGER_H__
