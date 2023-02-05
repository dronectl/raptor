
#include "stm32f4xx.h"

static void __delay(uint32_t cnt) {
  for (uint32_t i = 0; i < cnt; i++) {
    __NOP();
  }; // Loop repeats 2,000,000 implementing a delay
}

int main(void) {

  // Configue LEDs
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN; // Enable the clock of port D of the GPIO

  GPIOD->MODER |= GPIO_MODER_MODER12_0; // Green LED, set pin 12 as output
  GPIOD->MODER |= GPIO_MODER_MODER13_0; // Orange LED, set pin 13 as output
  GPIOD->MODER |= GPIO_MODER_MODER14_0; // Red LED, set pin 14 as output
  GPIOD->MODER |= GPIO_MODER_MODER15_0; // Blue LED, set pin 15 as output

  while (1) {
    // Turn on LEDs
    GPIOD->BSRR = 1 << 12; // Set the BSRR bit 12 to 1 to turn respective LED on
    __delay(100000);
    GPIOD->BSRR = 1 << 13; // Set the BSRR bit 13 to 1 to turn respective LED on
    __delay(100000);
    GPIOD->BSRR = 1 << 14; // Set the BSRR bit 14 to 1 to turn respective LED on
    __delay(100000);
    GPIOD->BSRR = 1 << 15; // Set the BSRR bit 15 to 1 to turn respective LED on

    __delay(100000);
    // Turn off LEDs
    GPIOD->BSRR =
        1 << (12 +
              16); // Set the BSRR bit 12 + 16 to 1 to turn respective LED off
    __delay(100000);
    GPIOD->BSRR =
        1 << (13 +
              16); // Set the BSRR bit 13 + 16 to 1 to turn respective LED off
    __delay(100000);
    GPIOD->BSRR =
        1 << (14 +
              16); // Set the BSRR bit 14 + 16 to 1 to turn respective LED off
    __delay(100000);
    GPIOD->BSRR =
        1 << (15 +
              16); // Set the BSRR bit 15 + 16 to 1 to turn respective LED off
    __delay(300000);
  }
}