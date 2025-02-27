#ifndef __RESET_H__
#define __RESET_H__

#include "mmio.h"
#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3F10001c
#define PM_WDOG 0x3F100024

void reset(int tick);
void cancel_reset();

#endif