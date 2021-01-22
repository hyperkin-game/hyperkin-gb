#ifndef __DDR_TEST_H_
#define __DDR_TEST_H_
#include "rk_pcba_test_led.h"

#ifdef PCBA_3308
#define DDR_CAPACITY 256
#endif

#ifdef PCBA_PX3SE
#define DDR_CAPACITY 1010
#endif

#ifdef PCBA_3229GVA
#define DDR_CAPACITY 256
#endif

void *ddr_test(void *argv);

#endif
