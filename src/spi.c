/**
 * @file spi.c
 * @brief
 * @version 0.1
 * @date 2023-01
 *
 * @copyright Copyright © 2023 dronectl
 *
 */
#include <avr/cpufunc.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#include "config.h"
#include "spi.h"

/**
 * @brief Register masks
 *
 */
#define SPSR_SPI2X_MASK 0x01
#define SPCR_SPI_MODE_MASK 0x0C
#define SPCR_SPI_CLK_MASK 0x03

/**
 * @brief Set SS low
 *
 */
void spi_enable_ss(void) { PORTB &= ~(1 << PB2); }
/**
 * @brief Reset SS high
 *
 */
void spi_disable_ss(void) { PORTB |= (1 << PB2); }

/**
 * @brief Setup SPI subsystem and gain access to SPI bus.
 *
 * @param spi_config spi bus configuration
 */
void spi_begin(spi_config_t spi_config) {
  // clang-format off
  /**
   * @brief find the fastest clock rate less than or equal too the passed clock speed
   * SPR1 SPR0 ~SPI2X
   *  0    0     0    fosc/2
   *  0    0     1    fosc/4
   *  0    1     0    fosc/8
   *  0    1     1    fosc/16
   *  1    0     0    fosc/32
   *  1    0     1    fosc/64 
   *  1    1     0    fosc/64 << note duplicate mode
   *  1    1     1    fosc/128
   */
  // clang-format on
  // the clock divider is computed by 2 ^^ (clock_div + 1)
  uint8_t clock_div = 7; // slowest speed 128 (default case) 6 + 1 compensates
                         // for duplicate fosc64
  if (spi_config.clock >= F_CPU / 2) {
    clock_div = 0;
  } else if (spi_config.clock >= F_CPU / 4) {
    clock_div = 1;
  } else if (spi_config.clock >= F_CPU / 8) {
    clock_div = 2;
  } else if (spi_config.clock >= F_CPU / 16) {
    clock_div = 3;
  } else if (spi_config.clock >= F_CPU / 32) {
    clock_div = 4;
  } else if (spi_config.clock >= F_CPU / 64) {
    clock_div = 5;
  }
  // invert SPI2x bit
  clock_div ^= 0x01;
  uint8_t sreg = SREG;
  cli();
  // write SPI2X to status register
  SPSR = (clock_div & SPSR_SPI2X_MASK);
  // pack and write spi config control register
  SPCR = (spi_config.endian == LITTLE_ENDIAN ? 1 : 0 << DORD) | (1 << SPE) |
         (1 << MSTR) | (spi_config.mode & SPCR_SPI_MODE_MASK) |
         ((clock_div >> 1) & SPCR_SPI_CLK_MASK);
  /**
   * @brief Setup SPI pins. Doing this AFTER enabling SPI, avoids clocking in a
   * single bit since the lines go directly from "input" to SPI control.
   * PB5 -> SCK
   * PB4 -> MISO (Configured as input automatically)
   * PB3 -> MOSI
   * PB2 -> SS (Slave select)
   */
  // set to high (source) internal pullup
  PORTB = (1 << PB2);
  // configure pins as outputs
  DDRB = (1 << PB5) | (1 << PB3) | (1 << PB2);
  SREG = sreg;
}

/**
 * @brief Write buffer to SPI bus and receive data and copy into buffer
 * TODO: enable capability to write array of instructions sequentially
 *
 * @param buffer pointer to single buffer element
 */
void spi_transact(uint8_t *buffer) {
  SPDR = *buffer;
  _NOP();
  while (!(SPSR & (1 << SPIF))) _NOP();
  *buffer = SPDR;
}

/**
 * @brief Write buffer byte to SPI bus and receive from bus and return
 *
 * @param buffer const byte
 * @return uint8_t receive byte from SPI bus
 */
uint8_t spi_transact_byte(const uint8_t buffer) {
  SPDR = buffer;
  _NOP();
  while (!(SPSR & (1 << SPIF))) _NOP();
  return SPDR;
}

/**
 * @brief Disable SPI subsystem.
 *
 */
void spi_end(void) {
  uint8_t sreg = SREG;
  cli();
  // disable spi
  SPCR &= ~(1 << SPE);
  SREG = sreg;
}
