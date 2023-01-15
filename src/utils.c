
#include "utils.h"
#include <avr/cpufunc.h>

/**
 * @brief
 *
 * @param cycles
 */
void delay_cycles(uint64_t cycles) {
  for (uint64_t i = 0; i < cycles; i++) {
    _NOP();
  };
}