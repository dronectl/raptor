#include <gtest/gtest.h>

extern "C" {
#include "scpi/ieee.h"
#include "scpi/err.h"
#include "sysreg.h"
}

TEST(SCPIErrTest, SCPIErrQueueBasic) {
  scpi_error_init();
  scpi_error_push(SCPI_ERR_BAD_ARG);
  uint8_t stb;
  sysreg_get_u8(SYSREG_STB, &stb);
  EXPECT_EQ((stb & SYSREG_STB_ERR_QUEUE) != 0, true) << "Push to error queue did not set STB:SYSREG_STB_ERR_QUEUE bit";
  scpi_err_t err = scpi_error_pop();
  EXPECT_EQ(err, SCPI_ERR_BAD_ARG) << "Pushed error differs from popped error";
  sysreg_get_u8(SYSREG_STB, &stb);
  EXPECT_EQ((stb & SYSREG_STB_ERR_QUEUE) == 0, true) << "Emptying error queue did not clear STB:SYSREG_STB_ERR_QUEUE bit";
}

TEST(SCPIErrTest, SCPIErrQueueOverflow) {
  scpi_error_init();
  for (uint8_t i = 0; i < IEEE_SCPI_MAX_ERR_QUEUE_LEN; i++) {
    scpi_error_push(SCPI_ERR_BAD_ARG);
  }
  scpi_err_t err = SCPI_ERR_NULL;
  for (uint8_t i = 0; i < IEEE_SCPI_MAX_ERR_QUEUE_LEN - 1; i++) {
    err = scpi_error_pop();
  }
  EXPECT_EQ(err, SCPI_ERR_EQUEUE_OF) << "Overflow error not written to error queue after error queue overflow in compliance with IEEE 488.2";
}
