
#include "uassert.h"

struct assert_trace g_assert_info = {0};

void assert_handler(const uint32_t line, const uint32_t *pc, const uint32_t *lr) {
  // File and line deliberately left empty
  g_assert_info.line = line;
  g_assert_info.pc = (uint32_t)pc;
  g_assert_info.lr = (uint32_t)lr;
#ifdef RAPTOR_DEBUG
  // halt CPU core for easier debugging
  __asm("bkpt 5");
#elif UNITTEST
  #include <assert.h>
  assert(0);
#else
  // spinlock and wait for hardware watchdog to trigger reset
  while (1);
#endif
}
