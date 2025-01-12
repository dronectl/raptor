#ifndef __DTC_H__
#define __DTC_H__

#include <stdint.h>

// TODO should be part of protocol schema
enum DTCID {
  DTCID_NONE,
  DTCID_COUNT,
  DTCID_HSM_UNHANDLED_EVENT,
};

struct dtc_event {
  uint32_t timestamp;
  enum DTCID event;
};

void dtc_post_event(const enum DTCID event);

#endif // __DTC_H__
