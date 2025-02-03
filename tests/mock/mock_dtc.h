#pragma once

#include "gmock/gmock.h"

extern "C" {
#include "dtc.h"
}

class MockDTC {
public:

  MOCK_METHOD(void, dtc_post_event, (const enum DTCID));
};

MockDTC *mock_dtc = nullptr;

// C-style wrapper functions for the mocks
extern "C" {

void dtc_post_event(const enum DTCID event) {
  return mock_dtc->dtc_post_event(event);
}
}
