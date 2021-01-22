#ifndef __AF_INDEPENDENT_H__
#define __AF_INDEPENDENT_H__

//#include <oslayer/oslayer.h>
#include <common/list.h>
#include <common/return_codes.h>
#include "cam_ia_api/cameric.h"

typedef void* IsiSensorHandle_t;
typedef RESULT (IsiMdiInitMotoDriveMds_t)     ( IsiSensorHandle_t handle );
typedef RESULT (IsiMdiSetupMotoDrive_t)       ( IsiSensorHandle_t handle, uint32_t *pMaxStep );
typedef RESULT (IsiMdiFocusSet_t)             ( IsiSensorHandle_t handle, const uint32_t AbsStep );
typedef RESULT (IsiMdiFocusGet_t)             ( IsiSensorHandle_t handle, uint32_t *pAbsStep );
typedef RESULT (IsiMdiFocusCalibrate_t)       ( IsiSensorHandle_t handle );

struct IsiSensor_s
{
    /* AF functions */
    IsiMdiInitMotoDriveMds_t            *pIsiMdiInitMotoDriveMds;
    IsiMdiSetupMotoDrive_t              *pIsiMdiSetupMotoDrive;
    IsiMdiFocusSet_t                    *pIsiMdiFocusSet;
    IsiMdiFocusGet_t                    *pIsiMdiFocusGet;
    IsiMdiFocusCalibrate_t              *pIsiMdiFocusCalibrate;
};

extern struct IsiSensor_s g_IsiMdiCfg;

//------------------------------------------
RESULT IsiMdiSetup
(
    struct IsiSensor_s *pConfig
);

RESULT IsiMdiFocusSet
(
    void*   handle,
    const uint32_t      AbsStep
);

RESULT IsiMdiFocusGet
(
    void*   handle,
    uint32_t*            pAbsStep
);

RESULT IsiMdiInitMotoDrive
(
    void*   handle
);

RESULT IsiMdiSetupMotoDrive
(
    void*   handle,
    uint32_t*            pMaxStep
);

inline RESULT IsiMdiSetup
(
    struct IsiSensor_s *pConfig
) {
  if (pConfig != NULL) {
    g_IsiMdiCfg = *pConfig;
    return 0;
  }

  return -1;
}

inline RESULT IsiMdiFocusSet
(
    void*   handle,
    const uint32_t      AbsStep
) {
  if (g_IsiMdiCfg.pIsiMdiFocusSet != NULL) {
    g_IsiMdiCfg.pIsiMdiFocusSet(handle, AbsStep);
    return 0;
  }

  return -1;
}

inline RESULT IsiMdiFocusGet
(
    void*   handle,
    uint32_t*            pAbsStep
) {
  if (g_IsiMdiCfg.pIsiMdiFocusGet != NULL) {
    g_IsiMdiCfg.pIsiMdiFocusGet(handle, pAbsStep);
    return 0;
  }

  return -1;
}

inline RESULT IsiMdiInitMotoDrive
(
    void*   handle
) {
  if (g_IsiMdiCfg.pIsiMdiInitMotoDriveMds != NULL) {
    g_IsiMdiCfg.pIsiMdiInitMotoDriveMds(handle);
    return 0;
  }

  return -1;
}

inline RESULT IsiMdiSetupMotoDrive
(
    void*   handle,
    uint32_t*            pMaxStep
) {
  if (g_IsiMdiCfg.pIsiMdiSetupMotoDrive != NULL) {
    g_IsiMdiCfg.pIsiMdiSetupMotoDrive(handle, pMaxStep);
    return 0;
  }

  return -1;
}

//---------------------------------------
typedef enum CamerIcIspAfmWindowId_e {
  CAMERIC_ISP_AFM_WINDOW_INVALID  = 0,    /**< lower border (only for an internal evaluation) */
  CAMERIC_ISP_AFM_WINDOW_A        = 1,    /**< Window A (1st window) */
  CAMERIC_ISP_AFM_WINDOW_B        = 2,    /**< Window B (2nd window) */
  CAMERIC_ISP_AFM_WINDOW_C        = 3,    /**< Window C (3rd window) */
  CAMERIC_ISP_AFM_WINDOW_MAX,             /**< upper border (only for an internal evaluation) */
} CamerIcIspAfmWindowId_t;

/******************************************************************************/
/**
 *          CamerIcEventCb_t
 *
 *  @brief  Event callback
 *
 *  This callback is used to signal something to the application software,
 *  e.g. an error or an information.
 *
 *  @return void
 *
 *****************************************************************************/
typedef struct CamerIcEventCb_s {
  void*                pUserContext;  /**< user context */
} CamerIcEventCb_t;


/******************************************************************************/
/**
 * @struct  CamerIcAfmMeasuringResult_s
 *
 * @brief   A structure to represent a complete set of measuring values.
 *
 *****************************************************************************/
typedef struct CamerIcAfmMeasuringResult_s {
  uint32_t    SharpnessA;         /**< sharpness of window A */
  uint32_t    SharpnessB;         /**< sharpness of window B */
  uint32_t    SharpnessC;         /**< sharpness of window C */

  uint32_t    LuminanceA;         /**< luminance of window A */
  uint32_t    LuminanceB;         /**< luminance of window B */
  uint32_t    LuminanceC;         /**< luminance of window C */

  uint32_t    PixelCntA;
  uint32_t    PixelCntB;
  uint32_t    PixelCntC;

  struct Cam_Win   WindowA;      /* ddl@rock-chips.com: v1.6.0 */
  struct Cam_Win   WindowB;
  struct Cam_Win   WindowC;

} CamerIcAfmMeasuringResult_t;

/*****************************************************************************/
/**
 * @brief   CamerIc ISP AF module internal driver context.
 *
 *****************************************************************************/
typedef struct CamerIcIspAfmContext_s {
  bool_t                          enabled;        /**< measuring enabled */
  CamerIcEventCb_t                EventCb;

  struct Cam_Win                  WindowA;    /**< measuring window A */
  uint32_t                        PixelCntA;
  bool_t                          EnabledWdwA;
  struct Cam_Win                  WindowB;        /**< measuring window B */
  uint32_t                        PixelCntB;
  bool_t                          EnabledWdwB;
  struct Cam_Win                  WindowC;        /**< measuring window C */
  uint32_t                        PixelCntC;
  bool_t                          EnabledWdwC;

  uint32_t                        Threshold;
  uint32_t                        lum_shift;
  uint32_t                        afm_shift;
  uint32_t                        MaxPixelCnt;

  CamerIcAfmMeasuringResult_t     MeasResult;
} CamerIcIspAfmContext_t;
/*
typedef struct CamerIcDrvContext_s
{
  CamerIcIspAfmContext_t          *pIspAfmContext;
} CamerIcDrvContext_t;

typedef struct CamerIcDrvContext_s  *CamerIcDrvHandle_t;
*/

/******************************************************************************
 * CamerIcIspAfmMeasuringWindowIsEnabled()
 *****************************************************************************/
bool_t CamerIcIspAfmMeasuringWindowIsEnabled
(
    const CamerIcIspAfmWindowId_t   WdwId
);
#endif

