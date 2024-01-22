
#ifndef __HEALTH_H__
#define __HEALTH_H__

/**
 * @brief Health FSM States
 *
 */
enum HealthState {
  HEALTH_NULL,    // unknown and null state
  HEALTH_RESET,   // reset state
  HEALTH_INIT,    // initialization state
  HEALTH_REPORT,  // alive reporting
  HEALTH_READ,    // sensor read
  HEALTH_SERVICE, // service watchdog
};

/**
 * @brief Low frequency alive data
 *
 */
typedef struct health_report_t {
  float timestamp;
  uint8_t status;
  float temperature;
  float humidity;
  float pressure;
} health_report_t;

void health_main(void *pv_params);

#endif // __HEALTH_H__