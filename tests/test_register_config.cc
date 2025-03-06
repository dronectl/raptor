
#include <gtest/gtest.h>

#include "mock_system.h"

extern "C" {
#include "registers.h"
}

class RegisterConfigTestFixture : public ::testing::Test {
protected:
  MockSystem m_system;

  void SetUp() override {
    mock_system = &m_system;
  }

  void TearDown() override {
    mock_system = nullptr;
  }
};


TEST(RegisterConfigTest, RegisterConfigUUID) {
}


