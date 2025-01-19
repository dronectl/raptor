
#include <gtest/gtest.h>

extern "C" {
#include "hsm.h"
}

TEST(HsmTest, HsmInitTest) {
  hsm_init(NULL);
}

