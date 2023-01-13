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

#include "spi.h"
#include "utils.h"
#include "w5100.h"
#include <avr/cpufunc.h>
#include <avr/io.h>

static void spinlock(void) {
  // status LED tied to SCK requires release of spi subsystem before use
  spi_end();
  // Setup PB5 as Output high (source)
  PORTB = (1 << PB5);
  DDRB = (1 << DDB5);
  // sync nop
  _NOP();
  while (1) {
    // blink SCK
    PORTB ^= (1 << PORTB5);
    delay_cycles(100000);
  }
}

int main(void) {
  w5100_init();
  spinlock();
  return 0;
}