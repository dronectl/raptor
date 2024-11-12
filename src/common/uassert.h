/**
 * @file uassert.c
 * @author ztnel (christian911@sympatico.ca)
 * @brief Micro-assert: Assertion tracing for minimal memory footprint
 * @version 0.1
 * @date 2024-11
 *
 * @copyright Copyright © 2024 Christian Sargusingh
 *
 */

#ifndef __UASSERT_H__
#define __UASSERT_H__

#include <stdint.h>

struct assert_trace {
  uint32_t pc;   // program counter register
  uint32_t lr;   // link register
  uint32_t line; // line number (for avoiding compiler optimzations where asserts could be merged)
};

extern struct assert_trace g_assert_info;

extern void assert_handler(const uint32_t line, const uint32_t *pc, const uint32_t *lr);

#define GET_LR() __builtin_return_address(0)
#define GET_PC(_a) __asm volatile("mov %0, pc" : "=r"(_a))

#define __uassert_record(__LINE__)    \
  do {                                \
    void *pc;                         \
    GET_PC(pc);                       \
    const void *lr = GET_LR();        \
    assert_handler(__LINE__, pc, lr); \
  } while (0)

#define uassert(exp)              \
  do {                            \
    if (!(exp)) {                 \
      __uassert_record(__LINE__); \
    }                             \
  } while (0)

#endif // __UASSERT_H__
