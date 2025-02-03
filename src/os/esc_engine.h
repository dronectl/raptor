#ifndef __ESC_ENGINE_H__
#define __ESC_ENGINE_H__

struct esc_engine_init_ctx {
  void *null;
};

void esc_engine_init(const struct esc_engine_init_ctx *init_ctx);

#endif // __ESC_ENGINE_H__
