#ifndef __HX711_H__
#define __HX711_H__

#include <stdint.h>
#include "hw_config.h"

/**
 * @brief ADC output in 24-bit 2's complement
 **/
#define HX711_ADC_MIN 0x80000
#define HX711_ADC_MAX 0x7FFFF



typedef int hx711_status_t;

#define HX711_OK (hx711_status_t)0

/**
 * @brief Table 3: HX711 Channel and Gain Select Modes.
 * Enum values match number of clock pulses required to configure next SOC.
 */
enum HX711InputSel {
  HX711_CHA_GAIN128=1, // Channel A (default) @ 128
  HX711_CHB_GAIN32=2, // Channel B @ 32 
  HX711_CHA_GAIN64=3, // Channel A (default) @ 64
};


/**
 * @brief struct hx711_settings_t - Load cell channel and calibration settings
 * NOTE: The XFW HX711 board used ties pin XI to DVDD which configures it to 80 Hz sampling
 */
typedef struct hx711_settings_t {
  enum HX711InputSel input_select; // channel and gain select
  int32_t offset; // V/V zero
  float gain; // measurement scaling factor
} hx711_settings_t;

/**
 * @brief Initialize HX711
 *
 * @return status code
 */
hx711_status_t hx711_init(hx711_settings_t *settings);

/**
 * @brief Read converted ADC measurement from HX711 and configure next conversion
 *
 * @param data conversion result
 * @param settings settings struct for load cell
 * @return status code
 */
hx711_status_t hx711_read(float *data, hx711_settings_t *settings);

/**
 * @brief Shutdown HX711
 *
 * @return status code
 */
hx711_status_t hx711_sleep(void);

/**
 * @brief Soft reset HX711
 *
 * @return status code
 */
hx711_status_t hx711_reset(void);

#endif // __HX711_H__

