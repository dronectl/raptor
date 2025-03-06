#pragma once

#include "gmock/gmock.h"

extern "C" {
#include "system.h"
}

class MockSystem {
public:
  MOCK_METHOD(enum register_status, system_write_uuid, (const uint32_t));
  MOCK_METHOD(enum register_status, system_read_uuid, (uint32_t*));
};

MockSystem *mock_system = nullptr;

// C-style wrapper functions for the mocks
extern "C" {

enum register_status system_write_uuid(const uint32_t uuid) {
  return mock_system->system_write_uuid(uuid);
}

enum register_status system_read_uuid(uint32_t* uuid) {
  return mock_system->system_read_uuid(uuid);
}

}
