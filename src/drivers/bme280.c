

#include "bme280.h"
#include <stdio.h>

// RX buffer
uint8_t rx_buf[8] = {0};

void bme280_init(I2C_HandleTypeDef *i2c_handle) {
  // wake device
  bme280_hw_reset(i2c_handle);
  // read chip metadata
  bme280_verify(i2c_handle);
  // configure measurement subsystem
  bme280_enable_measurements(i2c_handle);
  // set power mode
  bme280_set_power_mode(i2c_handle, BME280_NORMAL);
  printf("BME280 | Sensor initialized");
}

void bme280_verify(I2C_HandleTypeDef *i2c_handle) {
  uint8_t chip_id;
  HAL_I2C_Mem_Read(i2c_handle, BME280_DEFAULT_DEV_ADDR, BME280_ID, I2C_MEMADD_SIZE_8BIT,&chip_id, 1,HAL_MAX_DELAY);
  assert_param(chip_id == BME280_CHIP_ID);
}

void bme280_enable_measurements(I2C_HandleTypeDef *i2c_handle){
  uint8_t pload = 0x0;
  pload |= BME280_OSRS_T(BME280_OSRS_1X);
  pload |= BME280_OSRS_P(BME280_OSRS_1X);
  HAL_I2C_Mem_Write(i2c_handle, BME280_DEFAULT_DEV_ADDR, BME280_CTRL_MEAS, I2C_MEMADD_SIZE_8BIT,&pload, 1, HAL_MAX_DELAY);
  pload = BME280_OSRS_H(BME280_OSRS_1X);
  HAL_I2C_Mem_Write(i2c_handle, BME280_DEFAULT_DEV_ADDR, BME280_CTRL_HUM, I2C_MEMADD_SIZE_8BIT,&pload, 1, HAL_MAX_DELAY);
}

void bme280_set_power_mode(I2C_HandleTypeDef *i2c_handle, const enum BME280_PModes mode) {
  uint8_t ctrl_reg;
  HAL_I2C_Mem_Read(i2c_handle, BME280_DEFAULT_DEV_ADDR, BME280_CTRL_MEAS, I2C_MEMADD_SIZE_8BIT, &ctrl_reg, 1, HAL_MAX_DELAY);
  ctrl_reg |= BME280_PMODE(mode);
  HAL_I2C_Mem_Write(i2c_handle, BME280_DEFAULT_DEV_ADDR, BME280_CTRL_MEAS, I2C_MEMADD_SIZE_8BIT, &ctrl_reg, 1, HAL_MAX_DELAY);
}

void bme280_hw_reset(I2C_HandleTypeDef *i2c_handle) {
  uint8_t pdata = BME280_HW_RESET_KEY;
  HAL_I2C_Mem_Write(i2c_handle, BME280_DEFAULT_DEV_ADDR, BME280_RESET, I2C_MEMADD_SIZE_8BIT,&pdata, 1, HAL_MAX_DELAY);
}

void bme280_read(I2C_HandleTypeDef *i2c_handle, bme280_meas_t *measurements) {
  uint32_t msb;
  uint32_t lsb;
  uint32_t xlsb;
  measurements->humidity_raw = 0;
  measurements->pressure_raw = 0;
  measurements->temperature_raw = 0;
  measurements->temperature = 0.0f;
  measurements->pressure = 0.0f;
  measurements->humidity = 0.0f;
  // use DMA read
  HAL_I2C_Mem_Read_DMA(i2c_handle, BME280_DEFAULT_DEV_ADDR, BME280_PRESS_MSB, I2C_MEMADD_SIZE_8BIT, rx_buf, 8);
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
  printf("BME280 | Completed read");
}

void bme280_sleep(I2C_HandleTypeDef *i2c_handle) {
  bme280_set_power_mode(i2c_handle, BME280_SLEEP);
}