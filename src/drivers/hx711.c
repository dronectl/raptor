
#include "hx711.h"
#include "FreeRTOS.h"

void hx711_wait_ready(void);
void hx711_power_up(void);
void hx711_shutdown(void);

void hx711_init(hx711_config_t *hc) {
}

void hx711_deinit(void) {

}

uint32_t hx711_read(void) {
  portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
	portENTER_CRITICAL(&mux);
  portEXIT_CRITICAL(&mux)
}


