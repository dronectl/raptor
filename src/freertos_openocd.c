#include "FreeRTOS.h"

#ifdef __GNUC__
#define USED __attribute__((used))
#else
#define USED
#endif

const volatile int USED uxTopUsedPriority = configMAX_PRIORITIES - 1;
