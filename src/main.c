/**
 * @file main.c
 * @author Christian Sargusingh (christian911@sympatico.ca)
 * @brief 
 * @version 0.1
 * @date 2023-01
 * 
 * @copyright Copyright © 2023 dronectl
 * 
 */

#include <avr/io.h>
#include <avr/cpufunc.h>

static void __delay(uint64_t cycles) {
  for (uint64_t i = 0; i < cycles; i++) { _NOP(); };
}

static void spinlock(void) {
  while (1) {
    PORTB ^= (1 << PORTB5);
    __delay(10000);
    _NOP();
  }
}

int main(void) {
  spinlock();
  return 0;
}