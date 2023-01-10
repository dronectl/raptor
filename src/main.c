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

#include <avr/cpufunc.h>
#include <avr/io.h>

static void __delay(uint64_t cycles) {
  for (uint64_t i = 0; i < cycles; i++) {
    _NOP();
  };
}

static void spinlock(void) {
  // Setup PB5 as Output high (source)
  PORTB = (1 << PB5);
  DDRB = (1 << DDB5);
  // sync nop
  _NOP();
  while (1) {
    // blink SCK
    PORTB ^= (1 << PORTB5);
    __delay(100000);
  }
}

int main(void) {
  spinlock();
  return 0;
}