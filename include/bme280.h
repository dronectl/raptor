
#ifndef __BME280_H__
#define __BME280_H__

#include <stm32h7xx_hal.h>

#define BME280_DEFAULT_DEV_ADDR (uint8_t)(0x76 << 1)

#define BME280_HW_RESET_KEY 0xB6
#define BME280_CHIP_ID 0x60 // BME280 UUID

/**
 * Hardware limits
*/
#define BME280_TEMP_MIN -40.0f // °C
#define BME280_TEMP_MAX 85.0f // °C
#define BME280_TEMP_MIN -40.0f // Pa
#define BME280_PRESS_MAX 85.0f // Pa
#define BME280_HUM_MIN 0.0f // %
#define BME280_HUM_MAX 100.0f // %

/**
 * 0xF3 STATUS
*/
#define BME280_STAT_MEAS_MSK (uint8_t)(0x1 << 3)
#define BME280_STAT_UPDATE_MSK (uint8_t)(0x1)

/**
 * 0xF2 CTRL_HUM
*/
#define BME280_OSRS_H(x)      ((uint8_t)(x) << 0) // Humidity OSRS bits

/**
 * 0xF4 CTRL_MEAS
*/
#define BME280_OSRS_T(x)      ((uint8_t)(x) << 5) // Temperature OSRS bits
#define BME280_OSRS_P(x)      ((uint8_t)(x) << 2) // Pressure OSRS bits
#define BME280_PMODE(x)       ((uint8_t)(x) << 0) // Power mode bits


enum BME280_OSRS {
  BME280_OSRS_DISABLE = 0x0, // default on reset
  BME280_OSRS_1X = 0x1, // oversampling x 1
  BME280_OSRS_2X = 0x2, // oversampling x 2
  BME280_OSRS_4X = 0x3, // oversampling x 4
  BME280_OSRS_8X = 0x4, // oversampling x 8
  BME280_OSRS_16X = 0x5, // oversampling x 16
};



enum BME280_PModes {
  BME280_SLEEP = 0x0, // default on reset
  BME280_FORCED = 0x1, // triggered sampling
  BME280_NORMAL = 0x3, // continuous sampling
};


/**
 * 5.3 Memory Map Table 18
 * https://cdn-learn.adafruit.com/assets/assets/000/115/588/original/bst-bme280-ds002.pdf?1664822559
*/
enum BME280_MMap {
  BME280_ID = 0xD0,
  BME280_RESET = 0xE0,
  BME280_CTRL_HUM = 0xF2,
  BME280_STATUS = 0xF3,
  BME280_CTRL_MEAS = 0xF4,
  BME280_CONFIG = 0xF5,
  BME280_PRESS_MSB = 0xF7,
  BME280_PRESS_LSB = 0xF8,
  BME280_PRESS_XLSB = 0xF9,
  BME280_TEMP_MSB = 0xFA,
  BME280_TEMP_LSB = 0xFB,
  BME280_TEMP_XLSB = 0xFC,
  BME280_HUM_MSB = 0xFD,
  BME280_HUM_LSB = 0xFE
};

typedef struct bme280_meas_t {
  uint32_t temperature_raw;
  uint32_t pressure_raw;
  uint32_t humidity_raw;
  float temperature; // °C
  float pressure; // HPa
  float humidity; // %
} bme280_meas_t;

void bme280_init(I2C_HandleTypeDef *i2c_handle);
void bme280_hw_reset(I2C_HandleTypeDef *i2c_handle);
void bme280_sleep(I2C_HandleTypeDef *i2c_handle);
void bme280_read(I2C_HandleTypeDef *i2c_handle, bme280_meas_t *measurements);
void bme280_verify(I2C_HandleTypeDef *i2c_handle);
void bme280_enable_measurements(I2C_HandleTypeDef *i2c_handle);
void bme280_set_power_mode(I2C_HandleTypeDef *i2c_handle, const enum BME280_PModes mode);

#endif // __BME280_H__