/**
 * @file spi.h
 * @brief SPI definitions, constants and function declarations. Implementation
 * is adapted from ArduinoCore-avr library:
 * https://github.com/arduino/ArduinoCore-avr. This driver is written with
 * minimal capabilities and does not include cross platform support preprocessor
 * conditions.
 * @version 0.1
 * @date 2023-01
 *
 * @copyright Copyright © 2023 dronectl
 *
 */

#ifndef __SPI_H__
#define __SPI_H__

#include <stdint.h>

/**
 * @brief SCK Phase x Polarity modes
 * 18.4 Data Modes
 */
enum SPIMode {
  SPI_MODE0 = 0x00, // Sample (rising), Setup (falling)
  SPI_MODE1 = 0x04, // Setup (rising), Sample (falling)
  SPI_MODE2 = 0x08, // Sample (falling), Setup (rising)
  SPI_MODE3 = 0x0C, // Setup (falling), Sample (rising)
};

/**
 * @brief Bit endianness
 *
 */
enum Endian {
  LITTLE_ENDIAN = 0x00, // LSB first
  BIG_ENDIAN = 0x01,    // MSB first
};

/**
 * @brief SPI init configuration
 *
 */
typedef struct spi_config_t {
  uint64_t clock;     // clock speed (Hz)
  enum Endian endian; // bit order
  enum SPIMode mode;  // data mode
} spi_config_t;

void spi_begin(spi_config_t spi_config);
void spi_transact(uint8_t *buffer);
uint8_t spi_transact_byte(const uint8_t buffer);
void spi_enable_ss(void);
void spi_disable_ss(void);
void spi_end(void);

#endif // __SPI_H__