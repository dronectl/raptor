

#include "bme280.h"
#include <stdio.h>

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
  } else /* Invalid case */
  {
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

static void load_calibration(bme280_dev_t *dev) {
  uint8_t rx_buf[BME280_CALIB_BLK0_SIZE] = {0};
  bme280_calib_t *calib_data = &dev->calib_data;
  HAL_I2C_Mem_Read(&dev->i2c, BME280_DEFAULT_DEV_ADDR, BME280_CALIB00, I2C_MEMADD_SIZE_8BIT, rx_buf, BME280_CALIB_BLK0_SIZE, HAL_MAX_DELAY);
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
  HAL_I2C_Mem_Read(&dev->i2c, BME280_DEFAULT_DEV_ADDR, BME280_CALIB26, I2C_MEMADD_SIZE_8BIT, rx_buf, BME280_CALIB_BLK1_SIZE, HAL_MAX_DELAY);
  calib_data->dig_h2 = (int16_t)(rx_buf[1] << 8) | (int16_t)rx_buf[0];
  calib_data->dig_h3 = (uint16_t)rx_buf[2];
  calib_data->dig_h4 = (int16_t)((rx_buf[3] << 4) | (int16_t)(rx_buf[4] & 0xF));
  calib_data->dig_h5 = (int16_t)((rx_buf[5] << 4) | (int16_t)(rx_buf[4] >> 4));
  calib_data->dig_h6 = (int16_t)rx_buf[6];
  printf("BME280 | Loaded calibration parameters\r\n");
}

static void enable_measurements(bme280_dev_t *dev) {
  uint8_t pload = 0x0;
  pload |= BME280_OSRS_T(BME280_OSRS_1X);
  pload |= BME280_OSRS_P(BME280_OSRS_1X);
  HAL_I2C_Mem_Write(&dev->i2c, BME280_DEFAULT_DEV_ADDR, BME280_CTRL_MEAS, I2C_MEMADD_SIZE_8BIT, &pload, 1, HAL_MAX_DELAY);
  pload = BME280_OSRS_H(BME280_OSRS_1X);
  HAL_I2C_Mem_Write(&dev->i2c, BME280_DEFAULT_DEV_ADDR, BME280_CTRL_HUM, I2C_MEMADD_SIZE_8BIT, &pload, 1, HAL_MAX_DELAY);
}

static void set_power_mode(bme280_dev_t *dev, const enum BME280_PModes mode) {
  uint8_t ctrl_reg;
  HAL_I2C_Mem_Read(&dev->i2c, BME280_DEFAULT_DEV_ADDR, BME280_CTRL_MEAS, I2C_MEMADD_SIZE_8BIT, &ctrl_reg, 1, HAL_MAX_DELAY);
  ctrl_reg |= BME280_PMODE(mode);
  HAL_I2C_Mem_Write(&dev->i2c, BME280_DEFAULT_DEV_ADDR, BME280_CTRL_MEAS, I2C_MEMADD_SIZE_8BIT, &ctrl_reg, 1, HAL_MAX_DELAY);
}

static void verify(bme280_dev_t *dev) {
  // populate chip ID and verify
  HAL_I2C_Mem_Read(&dev->i2c, BME280_DEFAULT_DEV_ADDR, BME280_ID, I2C_MEMADD_SIZE_8BIT, &dev->chip_id, 1, HAL_MAX_DELAY);
  assert_param(dev->chip_id == BME280_CHIP_ID);
}

void bme280_init(bme280_dev_t *dev) {
  bme280_reset(dev);
  verify(dev);
  load_calibration(dev);
  // configure measurement subsystem
  enable_measurements(dev);
  // set power mode
  set_power_mode(dev, BME280_NORMAL);
  printf("BME280 | Sensor %d initialized\r\n", dev->chip_id);
}

void bme280_reset(bme280_dev_t *dev) {
  uint8_t pdata = BME280_HW_RESET_KEY;
  HAL_I2C_Mem_Write(&dev->i2c, BME280_DEFAULT_DEV_ADDR, BME280_RESET, I2C_MEMADD_SIZE_8BIT, &pdata, 1, HAL_MAX_DELAY);
  printf("BME280 | Reset\r\n");
}

void bme280_read(bme280_dev_t *dev, bme280_meas_t *measurements) {
  uint32_t msb;
  uint32_t lsb;
  uint32_t xlsb;
  uint8_t rx_buf[8] = {0};
  measurements->humidity_raw = 0;
  measurements->pressure_raw = 0;
  measurements->temperature_raw = 0;
  measurements->temperature = 0.0f;
  measurements->pressure = 0.0f;
  measurements->humidity = 0.0f;
  HAL_I2C_Mem_Read(&dev->i2c, BME280_DEFAULT_DEV_ADDR, BME280_PRESS_MSB, I2C_MEMADD_SIZE_8BIT, rx_buf, 8, HAL_MAX_DELAY);
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
  printf("BME280 | Measurement complete\r\n");
}

void bme280_sleep(bme280_dev_t *dev) {
  set_power_mode(dev, BME280_SLEEP);
}