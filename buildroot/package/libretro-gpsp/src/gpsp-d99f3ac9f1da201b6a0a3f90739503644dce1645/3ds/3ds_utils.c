
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <3ds.h>
#include "3ds_utils.h"

typedef s32 (*ctr_callback_type)(void);

static void ctr_invalidate_ICache_kernel(void)
{
   __asm__ volatile(
      "cpsid aif\n\t"
      "mov r0, #0\n\t"
      "mcr p15, 0, r0, c7, c5, 0\n\t");
}

static void ctr_flush_DCache_kernel(void)
{
   __asm__ volatile(
      "cpsid aif\n\t"
      "mov r0, #0\n\t"
      "mcr p15, 0, r0, c7, c10, 0\n\t");

}

void ctr_invalidate_ICache(void)
{
   svcBackdoor((ctr_callback_type)ctr_invalidate_ICache_kernel);

}

void ctr_flush_DCache(void)
{
   svcBackdoor((ctr_callback_type)ctr_flush_DCache_kernel);
}


void ctr_flush_invalidate_cache(void)
{
   ctr_flush_DCache();
   ctr_invalidate_ICache();
}
