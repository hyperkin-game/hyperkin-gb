#ifndef __AUDIO_SERVER_H__
#define __AUDIO_SERVER_H__

#include <stdio.h>

#define LOG_TAG "AudioService"

#define LOG_DEBUG_LEVEL (1)
#define LOG_ERROR_FLAG (4)
#define LOG_WARING_FLAG (3)
#define LOG_INFO_FLAG (2)
#define LOG_DEBUG_FLAG (1)

#define LOG_PRINTF(level, format, ...) \
    do { \
        if (level > LOG_DEBUG_LEVEL) { \
            printf("[%s]: " format "\n", LOG_TAG, ##__VA_ARGS__); \
        } \
    } while(0)
#define log_info(format, ...) LOG_PRINTF(LOG_INFO_FLAG, format, ##__VA_ARGS__)
#define log_dbg(format, ...) LOG_PRINTF(LOG_DEBUG_FLAG, format, ##__VA_ARGS__)
#define log_warn(format, ...) LOG_PRINTF(LOG_WARING_FLAG, format, ##__VA_ARGS__)
#define log_err(format, ...) LOG_PRINTF(LOG_ERROR_FLAG, format, ##__VA_ARGS__)

#endif // __AUDIO_SERVER_H__
