/**
 * @file bme280.h
 * @author ztnel (christian911@sympatico.ca)
 * @brief I2C Driver for BME280 Humidity, Temperature and Pressure Sensor.
 * https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme280-ds002.pdf
 * @version 0.1
 * @date 2024-01
 *
 * @copyright Copyright © 2024 dronectl
 *
 */

#ifndef __BME280_H__
#define __BME280_H__

#include <stm32h7xx_hal.h>

/**
 * @brief BME280 metadata
 * 5.4 Register description (5.4.1 & 5.4.2)
 */
#define BME280_DEFAULT_DEV_ADDR (uint8_t)(0x76 << 1)
#define BME280_HW_RESET_KEY 0xB6
#define BME280_CHIP_ID 0x60

/**
 * @brief BME280 Hardware limits
 *
 */
#define BME280_TEMP_MIN -40.0f    // °C
#define BME280_TEMP_MAX 85.0f     // °C
#define BME280_PRES_MIN 30000.0f  // Pa
#define BME280_PRES_MAX 110000.0f // Pa
#define BME280_HUM_MIN 0.0f       // %
#define BME280_HUM_MAX 100.0f     // %

#define BME280_CALIB00 0x88
#define BME280_CALIB25 0xA1
#define BME280_CALIB_BLK0_SIZE (uint16_t)(BME280_CALIB25 - BME280_CALIB00)
#define BME280_CALIB26 0xE1
#define BME280_CALIB41 0xF0
#define BME280_CALIB_BLK1_SIZE (uint16_t)(BME280_CALIB41 - BME280_CALIB26)

/**
 * @brief 0xF3 STATUS
 */
#define BME280_STAT_MEAS_MSK (uint8_t)(0x1 << 3)
#define BME280_STAT_UPDATE_MSK (uint8_t)(0x1)

/**
 * @brief 0xF2 CTRL_HUM
 */
#define BME280_OSRS_H(x) ((uint8_t)(x) << 0) // Humidity OSRS bits

/**
 * @brief 0xF4 CTRL_MEAS
 */
#define BME280_OSRS_T(x) ((uint8_t)(x) << 5) // Temperature OSRS bits
#define BME280_OSRS_P(x) ((uint8_t)(x) << 2) // Pressure OSRS bits
#define BME280_PMODE(x) ((uint8_t)(x) << 0)  // Power mode bits

/**
 * @brief Error codes
 *
 */
typedef int bme280_status_t;
#define BME280_OK (bme280_status_t)0
#define BME280_ERR (bme280_status_t)1
#define BME280_VERIFICATION (bme280_status_t)2
#define BME280_TIMEOUT (bme280_status_t)3
#define BME280_NVM_ERR (bme280_status_t)4
#define BME280_FORCE_ERR (bme280_status_t)5
#define BME280_MEAS_TIMEOUT (bme280_status_t)6
#define BME280_NO_DEVICE (bme280_status_t)7

/**
 * @brief Oversampling register settings
 * 5.4 Register description Table 20, 23, 24
 */
enum BME280_OSRS {
  BME280_OSRS_DISABLE = 0x0, // default on reset
  BME280_OSRS_1X = 0x1,      // oversampling x 1
  BME280_OSRS_2X = 0x2,      // oversampling x 2
  BME280_OSRS_4X = 0x3,      // oversampling x 4
  BME280_OSRS_8X = 0x4,      // oversampling x 8
  BME280_OSRS_16X = 0x5,     // oversampling x 16
};

/**
 * @brief Power modes register settings
 * 5.4 Register description Table 25
 */
enum BME280_PModes {
  BME280_SLEEP = 0x0,  // default on reset
  BME280_FORCED = 0x1, // triggered sampling
  BME280_NORMAL = 0x3, // continuous sampling
};

/**
 * @brief BME280 Register Memory Map
 * 5.3 Memory Map Table 18
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

/**
 * @brief Compensation parameters
 * 4.2.2 Trimming Parameter Readout Table 16: Compensation parameter storage, naming and dtype
 */
struct bme280_calibration {
  uint16_t dig_t1;
  int16_t dig_t2;
  int16_t dig_t3;
  uint16_t dig_p1;
  int16_t dig_p2;
  int16_t dig_p3;
  int16_t dig_p4;
  int16_t dig_p5;
  int16_t dig_p6;
  int16_t dig_p7;
  int16_t dig_p8;
  int16_t dig_p9;
  uint16_t dig_h1; // uint8_t (space optimized due to struct padding)
  int16_t dig_h2;
  uint16_t dig_h3; // uint8_t (space optimized due to struct padding)
  int16_t dig_h4;
  int16_t dig_h5;
  uint16_t dig_h6; // uint8_t (space optimized due to struct padding)
  int16_t t_fine;
};

/**
 * @brief BME280 device struct required for using the bme280 driver.
 *
 */
struct bme280_dev {
  uint8_t chip_id;
  I2C_HandleTypeDef *i2c;
  struct bme280_calibration calib_data;
};

/**
 * @brief Verify sensor, reset, load calibration trimming parameters, enable measurement subsystems.
 *
 * @param dev bme280 device struct
 * @return bme280_status_t status code
 */
bme280_status_t bme280_init(struct bme280_dev *dev);

/**
 * @brief Sensor soft reset
 *
 * @param dev bme280 device struct
 * @return bme280_status_t status code
 */
bme280_status_t bme280_reset(struct bme280_dev *dev);

/**
 * @brief Power off BME280
 *
 * @param dev bme280 device struct
 * @return bme280_status_t status code
 */
bme280_status_t bme280_sleep(struct bme280_dev *dev);

/**
 * @brief Perform a triggered bulk read of temperature, humidity, and pressure using forced mode.
 *
 * @param[in,out] dev BME280 device struct
 * @param[out] Temperature measurement (˚C)
 * @param[out] Pressure measurement (Pa)
 * @param[out] Humidity measurement (%)
 * @return status code
 */
bme280_status_t bme280_trigger_read(struct bme280_dev *dev, float *temperature, float *pressure, float *humidity);

#endif // __BME280_H__
