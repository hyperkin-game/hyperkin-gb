
#ifndef GPSP_CONFIG_H
#define GPSP_CONFIG_H

/* Cache sizes and their config knobs */
#if defined(PSP)
  #define ROM_TRANSLATION_CACHE_SIZE (1024 * 512 * 4)
  #define RAM_TRANSLATION_CACHE_SIZE (1024 * 384)
  #define TRANSLATION_CACHE_LIMIT_THRESHOLD (1024)
#else
  #define ROM_TRANSLATION_CACHE_SIZE (1024 * 512 * 4 * 5)
  #define RAM_TRANSLATION_CACHE_SIZE (1024 * 384 * 2)
  #define TRANSLATION_CACHE_LIMIT_THRESHOLD (1024 * 32)
#endif

/* This is MIPS specific for now */
#define STUB_ARENA_SIZE  (16*1024)

/* Hash table size for ROM trans cache lookups */
#define ROM_BRANCH_HASH_SIZE (1024 * 64)

#endif
