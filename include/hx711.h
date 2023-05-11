
#include <stdint.h>

#define MIN 0x80000
#define MAX 0x7FFFF

typedef struct hx711_config_t {
  uint8_t sck;
  uint8_t sda;
  uint8_t gain;
  uint16_t offset;
  uint16_t scale;

} hx711_config_t;

void hx711_init(hx711_config_t *hc);
void hx711_deinit(void);

/**
 * @brief 
 * 
 */
uint32_t hx711_read(void);