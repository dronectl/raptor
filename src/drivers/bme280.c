/**
 * @file bme280.c
 * @author ztnel (christian911@sympatico.ca)
 * @brief I2C Driver for BME280 Humidity, Temperature and Pressure Sensor.
 * https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme280-ds002.pdf
 * @version 0.1
 * @date 2024-01
 *
 * @copyright Copyright © 2024 dronectl
 *
 */

#include "bme280.h"

/**
 * @brief Convert temperature ADC measurement to ˚C.
 * 4.2.3 Compensation Formulas
 *
 * @param measurements measurement struct container with raw and converted measurments
 * @param calib_data bme280 calibration constants
 */
static void convert_temperature(bme280_meas_t *measurements, bme280_calib_t *calib_data);

/**
 * @brief Convert humidity ADC measurement to %.
 * 4.2.3 Compensation Formulas
 *
 * @param measurements measurement struct container with raw and converted measurments
 * @param calib_data bme280 calibration constants
 */
static void convert_humidity(bme280_meas_t *measurements, bme280_calib_t *calib_data);

/**
 * @brief Convert pressure ADC measurement to Pa.
 * 4.2.3 Compensation Formulas
 *
 * @param measurements measurement struct container with raw and converted measurments
 * @param calib_data bme280 calibration constants
 */
static void convert_pressure(bme280_meas_t *measurements, bme280_calib_t *calib_data);

/**
 * @brief Read I2C memory address from BME280
 *
 * @param hi2c I2C handle
 * @param mem_address device memory address
 * @param rx_buffer receive buffer
 * @param size number of bytes to read
 * @return bme280_status_t status code (converted from `HAL_StatusTypeDef`)
 */
static bme280_status_t _read(I2C_HandleTypeDef *hi2c, uint8_t mem_address, uint8_t *rx_buffer, uint16_t size);

/**
 * @brief Write I2C memory address from BME280
 *
 * @param hi2c I2C handle
 * @param mem_address device memory address
 * @param tx_buffer transact buffer
 * @param size number of bytes to write
 * @return bme280_status_t status code (converted from `HAL_StatusTypeDef`)
 */
static bme280_status_t _write(I2C_HandleTypeDef *hi2c, uint16_t mem_address, uint8_t *tx_buffer, uint16_t size);

/**
 * @brief Trigger a measurement and wait for conversion to complete.
 *
 * @param dev bme280 device struct
 * @return bme280_status_t status code
 */
static bme280_status_t acq_trigger_and_wait(bme280_dev_t *dev);

/**
 * @brief Configure measurement subsystems for temperature, pressure and humidity.
 * `BME280_OSRS_1X` to enable `BME280_OSRS_DISABLE` to disable, all other OSRS configure oversampling.
 *
 * @param dev bme280 device struct
 * @param temp_osrs temperature oversampling rate
 * @param press_osrs temperature oversampling rate
 * @param hum_osrs temperature oversampling rate
 * @return bme280_status_t
 */
static bme280_status_t configure_measurements(bme280_dev_t *dev, const enum BME280_OSRS temp_osrs, const enum BME280_OSRS press_osrs, const enum BME280_OSRS hum_osrs);

/**
 * @brief Set the sensor power mode
 *
 * @param dev bme280 device struct
 * @param mode power mode
 * @return bme280_status_t status code
 */
static bme280_status_t set_power_mode(bme280_dev_t *dev, const enum BME280_PModes mode);

/**
 * @brief Load calibration parameters from BME280 into device struct for measurement conversion.
 *
 * @param dev bme280 device struct
 * @return bme280_status_t status code
 */
static bme280_status_t load_calibration(bme280_dev_t *dev);

/**
 * @brief API entry
 *
 */
bme280_status_t bme280_init(bme280_dev_t *dev) {
  bme280_status_t status;
  // chip verification
  status = _read(&dev->i2c, BME280_ID, &dev->chip_id, 1);
  if (status != BME280_OK) {
    return status;
  }
  if (dev->chip_id != BME280_CHIP_ID) {
    return BME280_VERIFICATION;
  }
  // hardware reset sensor
  status = bme280_reset(dev);
  if (status != BME280_OK) {
    return status;
  }
  status = load_calibration(dev);
  if (status != BME280_OK) {
    return status;
  }
  // Configure sensor for low frequency weather monitoring (3.5.1)
  // temperature, pressure and humidity measurement subsystem with 1x oversampling
  // NOTE: IIR filter is disabled by default
  status = configure_measurements(dev, BME280_OSRS_1X, BME280_OSRS_1X, BME280_OSRS_1X);
  if (status != BME280_OK) {
    return status;
  }
  return status;
}

/**
 * @brief Sensor reset
 */
bme280_status_t bme280_reset(bme280_dev_t *dev) {
  bme280_status_t status;
  uint8_t status_reg;
  uint8_t retries = 3;
  uint8_t payload = BME280_HW_RESET_KEY;
  status = _write(&dev->i2c, BME280_RESET, &payload, 1);
  if (status != BME280_OK) {
    return status;
  }
  // Table 1. the technical datasheet indicates bme280 takes 2ms to boot up.
  HAL_Delay(2);
  // wait for trimming parameters to be read into memory from non-volatile memory
  do {
    status = _read(&dev->i2c, BME280_STATUS, &status_reg, 1);
  } while ((retries--) && (status == BME280_OK) && (status_reg & BME280_STAT_UPDATE_MSK));
  // notify failure with NVM copy
  if (status_reg & BME280_STAT_UPDATE_MSK) {
    status = BME280_NVM_ERR;
  }
  return status;
}

/**
 * @brief Execute a sensor read using triggering (forced mode)
 */
bme280_status_t bme280_trigger_read(bme280_dev_t *dev, bme280_meas_t *measurements) {
  uint32_t msb;
  uint32_t lsb;
  uint32_t xlsb;
  bme280_status_t status;
  uint8_t rx_buf[8] = {0};
  measurements->humidity_raw = 0;
  measurements->pressure_raw = 0;
  measurements->temperature_raw = 0;
  measurements->temperature = 0.0f;
  measurements->pressure = 0.0f;
  measurements->humidity = 0.0f;
  // trigger acquisition and wait for conversion
  status = acq_trigger_and_wait(dev);
  if (status != BME280_OK) {
    return status;
  }
  // read ready
  status = _read(&dev->i2c, BME280_PRESS_MSB, rx_buf, 8);
  if (status != BME280_OK) {
    return status;
  }
  msb = (rx_buf[0] << 12);
  lsb = (rx_buf[1] << 4);
  xlsb = (rx_buf[2] >> 4);
  measurements->pressure_raw = msb | lsb | xlsb;
  msb = (rx_buf[3] << 12);
  lsb = (rx_buf[4] << 4);
  xlsb = (rx_buf[5] >> 4);
  measurements->temperature_raw = msb | lsb | xlsb;
  msb = (rx_buf[6] << 8);
  lsb = rx_buf[7];
  measurements->humidity_raw = msb | lsb;
  convert_temperature(measurements, &dev->calib_data);
  convert_pressure(measurements, &dev->calib_data);
  convert_humidity(measurements, &dev->calib_data);
  return status;
}

bme280_status_t bme280_sleep(bme280_dev_t *dev) {
  return set_power_mode(dev, BME280_SLEEP);
}

static void convert_temperature(bme280_meas_t *measurements, bme280_calib_t *calib_data) {
  double var1;
  double var2;
  var1 = (((double)measurements->temperature_raw) / 16384.0 - ((double)calib_data->dig_t1) / 1024.0);
  var1 = var1 * ((double)calib_data->dig_t2);
  var2 = (((double)measurements->temperature_raw) / 131072.0 - ((double)calib_data->dig_t1) / 8192.0);
  var2 = (var2 * var2) * ((double)calib_data->dig_t3);
  calib_data->t_fine = (int32_t)(var1 + var2);
  measurements->temperature = (var1 + var2) / 5120.0;
  // clamp outputs
  if (measurements->temperature < BME280_TEMP_MIN) {
    measurements->temperature = BME280_TEMP_MIN;
  } else if (measurements->temperature > BME280_TEMP_MAX) {
    measurements->temperature = BME280_TEMP_MAX;
  }
}

static void convert_pressure(bme280_meas_t *measurements, bme280_calib_t *calib_data) {
  double var1;
  double var2;
  double var3;
  var1 = ((double)calib_data->t_fine / 2.0) - 64000.0;
  var2 = var1 * var1 * ((double)calib_data->dig_p6) / 32768.0;
  var2 = var2 + var1 * ((double)calib_data->dig_p5) * 2.0;
  var2 = (var2 / 4.0) + (((double)calib_data->dig_p4) * 65536.0);
  var3 = ((double)calib_data->dig_p3) * var1 * var1 / 524288.0;
  var1 = (var3 + ((double)calib_data->dig_p2) * var1) / 524288.0;
  var1 = (1.0 + var1 / 32768.0) * ((double)calib_data->dig_p1);
  /* Avoid exception caused by division by zero */
  if (var1 > (0.0)) {
    measurements->pressure = 1048576.0 - (double)measurements->pressure_raw;
    measurements->pressure = (measurements->pressure - (var2 / 4096.0)) * 6250.0 / var1;
    var1 = ((double)calib_data->dig_p9) * measurements->pressure * measurements->pressure / 2147483648.0;
    var2 = measurements->pressure * ((double)calib_data->dig_p8) / 32768.0;
    measurements->pressure = measurements->pressure + (var1 + var2 + ((double)calib_data->dig_p7)) / 16.0;
    // clamp output
    if (measurements->pressure < BME280_PRES_MIN) {
      measurements->pressure = BME280_PRES_MIN;
    } else if (measurements->pressure > BME280_PRES_MAX) {
      measurements->pressure = BME280_PRES_MAX;
    }
  } else {
    measurements->pressure = BME280_PRES_MIN;
  }
}

static void convert_humidity(bme280_meas_t *measurements, bme280_calib_t *calib_data) {
  double var1;
  double var2;
  double var3;
  double var4;
  double var5;
  double var6;
  var1 = ((double)calib_data->t_fine) - 76800.0;
  var2 = (((double)calib_data->dig_h4) * 64.0 + (((double)calib_data->dig_h5) / 16384.0) * var1);
  var3 = measurements->humidity_raw - var2;
  var4 = ((double)calib_data->dig_h2) / 65536.0;
  var5 = (1.0 + (((double)calib_data->dig_h3) / 67108864.0) * var1);
  var6 = 1.0 + (((double)calib_data->dig_h6) / 67108864.0) * var1 * var5;
  var6 = var3 * var4 * (var5 * var6);
  measurements->humidity = var6 * (1.0 - ((double)calib_data->dig_h1) * var6 / 524288.0);
  // clamp output
  if (measurements->humidity > BME280_HUM_MAX) {
    measurements->humidity = BME280_HUM_MAX;
  } else if (measurements->humidity < BME280_HUM_MIN) {
    measurements->humidity = BME280_HUM_MIN;
  }
}

static bme280_status_t _read(I2C_HandleTypeDef *hi2c, uint8_t mem_address, uint8_t *rx_buffer, uint16_t size) {
  HAL_StatusTypeDef status;
  status = HAL_I2C_Mem_Read(hi2c, BME280_DEFAULT_DEV_ADDR, (uint16_t)mem_address, I2C_MEMADD_SIZE_8BIT, rx_buffer, size, HAL_MAX_DELAY);
  if (status == HAL_TIMEOUT || status == HAL_BUSY) {
    return BME280_TIMEOUT;
  } else if (status == HAL_ERROR) {
    return BME280_ERR;
  }
  return BME280_OK;
}

static bme280_status_t _write(I2C_HandleTypeDef *hi2c, uint16_t mem_address, uint8_t *tx_buffer, uint16_t size) {
  HAL_StatusTypeDef status;
  status = HAL_I2C_Mem_Write(hi2c, BME280_DEFAULT_DEV_ADDR, mem_address, I2C_MEMADD_SIZE_8BIT, tx_buffer, size, HAL_MAX_DELAY);
  if (status == HAL_TIMEOUT || status == HAL_BUSY) {
    return BME280_TIMEOUT;
  } else if (status == HAL_ERROR) {
    return BME280_ERR;
  }
  return BME280_OK;
}

static bme280_status_t acq_trigger_and_wait(bme280_dev_t *dev) {
  bme280_status_t status;
  uint8_t stat_reg = 0;
  uint8_t retries = 5;
  // set forced mode
  status = set_power_mode(dev, BME280_FORCED);
  if (status != BME280_OK) {
    return BME280_FORCE_ERR;
  }
  // wait for acquisition completion
  do {
    status = _read(&dev->i2c, BME280_STATUS, &stat_reg, 1);
  } while ((status == BME280_OK) && (retries--) && (stat_reg & BME280_STAT_MEAS_MSK));
  if (stat_reg & BME280_STAT_MEAS_MSK) {
    return BME280_MEAS_TIMEOUT;
  }
  return status;
}

static bme280_status_t load_calibration(bme280_dev_t *dev) {
  bme280_status_t status;
  uint8_t rx_buf[BME280_CALIB_BLK0_SIZE] = {0};
  bme280_calib_t *calib_data = &dev->calib_data;
  status = _read(&dev->i2c, BME280_CALIB00, rx_buf, BME280_CALIB_BLK0_SIZE);
  if (status != BME280_OK) {
    return status;
  }
  calib_data->dig_t1 = (uint16_t)(rx_buf[1] << 8) | (uint16_t)rx_buf[0];
  calib_data->dig_t2 = (int16_t)((rx_buf[3] << 8) | (int16_t)rx_buf[2]);
  calib_data->dig_t3 = (int16_t)((rx_buf[5] << 8) | (int16_t)rx_buf[4]);
  calib_data->dig_p1 = (uint16_t)((rx_buf[7] << 8) | (uint16_t)rx_buf[6]);
  calib_data->dig_p2 = (int16_t)((rx_buf[9] << 8) | (int16_t)rx_buf[8]);
  calib_data->dig_p3 = (int16_t)((rx_buf[11] << 8) | (int16_t)rx_buf[10]);
  calib_data->dig_p4 = (int16_t)((rx_buf[13] << 8) | (int16_t)rx_buf[12]);
  calib_data->dig_p5 = (int16_t)((rx_buf[15] << 8) | (int16_t)rx_buf[14]);
  calib_data->dig_p6 = (int16_t)((rx_buf[17] << 8) | (int16_t)rx_buf[16]);
  calib_data->dig_p7 = (int16_t)((rx_buf[19] << 8) | (int16_t)rx_buf[18]);
  calib_data->dig_p8 = (int16_t)((rx_buf[21] << 8) | (int16_t)rx_buf[20]);
  calib_data->dig_p9 = (int16_t)((rx_buf[23] << 8) | (int16_t)rx_buf[22]);
  calib_data->dig_h1 = (uint16_t)rx_buf[24];
  // clear buffer
  for (int i = 0; i < BME280_CALIB_BLK0_SIZE - 1; i++) {
    rx_buf[i] = 0;
  }
  status = _read(&dev->i2c, BME280_CALIB26, rx_buf, BME280_CALIB_BLK1_SIZE);
  if (status != BME280_OK) {
    return status;
  }
  calib_data->dig_h2 = (int16_t)(rx_buf[1] << 8) | (int16_t)rx_buf[0];
  calib_data->dig_h3 = (uint16_t)rx_buf[2];
  calib_data->dig_h4 = (int16_t)((rx_buf[3] << 4) | (int16_t)(rx_buf[4] & 0xF));
  calib_data->dig_h5 = (int16_t)((rx_buf[5] << 4) | (int16_t)(rx_buf[4] >> 4));
  calib_data->dig_h6 = (int16_t)rx_buf[6];
  return status;
}

static bme280_status_t configure_measurements(bme280_dev_t *dev, const enum BME280_OSRS temp_osrs, const enum BME280_OSRS press_osrs, const enum BME280_OSRS hum_osrs) {
  bme280_status_t status;
  uint8_t pload = 0x0;
  pload |= BME280_OSRS_T(temp_osrs);
  pload |= BME280_OSRS_P(press_osrs);
  status = _write(&dev->i2c, BME280_CTRL_MEAS, &pload, 1);
  if (status != BME280_OK) {
    return status;
  }
  pload = BME280_OSRS_H(hum_osrs);
  status = _write(&dev->i2c, BME280_CTRL_HUM, &pload, 1);
  return status;
}

static bme280_status_t set_power_mode(bme280_dev_t *dev, const enum BME280_PModes mode) {
  uint8_t ctrl_reg;
  bme280_status_t status;
  status = _read(&dev->i2c, BME280_CTRL_MEAS, &ctrl_reg, 1);
  if (status != BME280_OK) {
    return status;
  }
  ctrl_reg |= BME280_PMODE(mode);
  return _write(&dev->i2c, BME280_CTRL_MEAS, &ctrl_reg, 1);
}