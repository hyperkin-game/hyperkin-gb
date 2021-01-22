#ifndef __CAM_HAL_TRACE_H_
#define __CAM_HAL_TRACE_H_
#ifndef ANDROID_OS
#include "ebase/trace.h"

USE_TRACER(CAMHAL_INFO);
USE_TRACER(CAMHAL_WARNING);
USE_TRACER(CAMHAL_ERROR);
extern int globalTraceLev;
#define ALOGD(...) TRACE(CAMHAL_INFO,__VA_ARGS__)
#define ALOGE(...) TRACE(CAMHAL_ERROR,__VA_ARGS__)
#define ALOGW(...) TRACE(CAMHAL_WARNING,__VA_ARGS__)
#define ALOGV(...) TRACE(CAMHAL_INFO,__VA_ARGS__)
#define TRACE_D(lev,...) \
  if(lev <= globalTraceLev) { \
    ALOGD(__VA_ARGS__); \
  }

#define LOGD ALOGD
#define LOGE ALOGE
#define LOGW ALOGW
#define LOGV ALOGV

#else
#include <utils/Log.h>
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "CamHal"
#define LOGD ALOGD
#define LOGE ALOGE
#define LOGW ALOGW
#define LOGV ALOGV
#define LOGI ALOGI
#define TRACE_D ALOGIF

#endif
#endif
