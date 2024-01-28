
#ifndef __HX711_H__
#define __HX711_H__

#include <stdint.h>
#define MIN 0x80000
#define MAX 0x7FFFF

typedef struct hx711_config_t {
  // uint8_t sck;
  // uint8_t sda;
  uint8_t gain;
  uint16_t offset;
  uint16_t slope;

} hx711_config_t;

void hx711_init(void);
uint32_t hx711_read(void);
void hx711_task(void *pv_params);
void hx711_shutdown(void);
void hx711_boot(void);

#endif // __HX711_H__