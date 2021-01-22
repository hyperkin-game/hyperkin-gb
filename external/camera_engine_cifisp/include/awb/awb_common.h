/******************************************************************************
 *
 * Copyright 2016, Fuzhou Rockchip Electronics Co.Ltd. All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of:
 * Fuzhou Rockchip Electronics Co.Ltd .
 * 
 *
 *****************************************************************************/
#ifndef __AWB_COMMON_H__
#define __AWB_COMMON_H__

/**
 * @file awb_common.h
 *
 * @brief
 *
 *****************************************************************************/
/**
 * @page module_name_page Module Name
 * Describe here what this module does.
 *
 * For a detailed list of functions and implementation detail refer to:
 * - @ref module_name
 *
 * @defgroup AWBM Auto white Balance Module
 * @{
 *
 */
#include <ebase/types.h>
#include <common/return_codes.h>
#include <common/cam_types.h>
//#include "cameric.h"
#include <cam_calibdb/cam_calibdb_api.h>

#ifdef __cplusplus
extern "C"
{
#endif


/*****************************************************************************/
/**
 * @brief
 */
/*****************************************************************************/
typedef struct AwbContext_s* AwbHandle_t;           /**< handle to AWB context */



/*****************************************************************************/
/**
 * @brief
 */
/*****************************************************************************/
typedef enum AwbWorkingFlags_e {
  AWB_WORKING_FLAG_USE_DAMPING        = 0x01,
  AWB_WORKING_FLAG_USE_CC_OFFSET      = 0x02
} AwbWorkingFlags_t;



/*****************************************************************************/
/**
 * @brief
 *
 */
/*****************************************************************************/
typedef enum AwbRunMode_e {
  AWB_MODE_INVALID                    = 0,        /**< initialization value */
  AWB_MODE_MANUAL                     = 1,        /**< run manual white balance */
  AWB_MODE_AUTO                       = 2,        /**< run auto white balance */
  AWB_MODE_MAX
} AwbMode_t;



/*****************************************************************************/
/**
 * @brief   type for evaluatiing number of white pixel
 */
/*****************************************************************************/
typedef enum AwbNumWhitePixelEval_e {
  AWB_NUM_WHITE_PIXEL_INVALID         = 0,        /**< initialization value */
  AWB_NUM_WHITE_PIXEL_LTMIN           = 1,        /**< less than configured minimum */
  AWB_NUM_WHITE_PIXEL_GTMAX           = 2,        /**< greater than defined maximum */
  AWB_NUM_WHITE_PIXEL_TARGET_RANGE    = 3,        /**< in min max range */
  AWB_NUM_WHITE_PIXEL_MAX
} AwbNumWhitePixelEval_t;



/*****************************************************************************/
/**
 * @brief
 *
 */
/*****************************************************************************/
typedef struct AwbComponent_s {
  float   fRed;
  float   fGreen;
  float   fBlue;
} AwbComponent_t;



/*****************************************************************************/
/**
 * @brief   A structure/tupple to represent gain values for four (R,Gr,Gb,B)
 *          channels.
 *
 * @note    The gain values are represented as float numbers.
 */
/*****************************************************************************/
typedef struct AwbGains_s {
  float fRed;         /**< gain value for the red channel */
  float fGreenR;      /**< gain value for the green channel in red lines */
  float fGreenB;      /**< gain value for the green channel in blue lines */
  float fBlue;        /**< gain value for the blue channel */
} AwbGains_t;



/*****************************************************************************/
/**
 * @brief   A structure/tupple to represent gain values for four (R,Gr,Gb,B)
 *          channels.
 *
 * @note    The gain values are represented as signed numbers.
 */
/*****************************************************************************/
typedef struct AwbXTalkOffset_s {
  float fRed;         /**< value for the red channel */
  float fGreen;       /**< value for the green channel in red lines */
  float fBlue;        /**< value for the blue channel */
} AwbXTalkOffset_t;

enum AwbMeasuringResultType_e {
  RESULT_RGB,
  RESULT_YCbCr
};

/*****************************************************************************/
/**
 * @brief   This macro defines the number of used bins.
 *
 *****************************************************************************/
#define AWB_HIST_NUM_BINS           16  /**< number of bins */

/*****************************************************************************/
/**
 * @brief   This typedef represents the histogram which is measured by the
 *          CamerIC ISP histogram module.
 *
 *****************************************************************************/
typedef uint32_t AWBHistBins_t[AWB_HIST_NUM_BINS];

typedef struct AwbMeasuringResult_s {
  uint32_t    NoWhitePixel;           /**< number of white pixel */
  float     MeanY__G;               /**< Y/G  value in YCbCr/RGB Mode */
  float     MeanCb__B;              /**< Cb/B value in YCbCr/RGB Mode */
  float     MeanCr__R;              /**< Cr/R value in YCbCr/RGB Mode */
  enum AwbMeasuringResultType_e type;
} AwbMeasuringResult_t;


typedef struct AwbRunningInputParams_s {
  AwbMeasuringResult_t MesureResult;
  //histogram
  AWBHistBins_t HistBins;
  //AwbGains_t             Gains;          /**< current gains from hardware */

  //Cam3x3FloatMatrix_t        CtMatrix;        /**< current cross talk matrix from hardware */
  //AwbXTalkOffset_t         CtOffset;        /**< current cross talk offset from hardware */
  float                         fGain;
  float                         fIntegrationTime;
} AwbRunningInputParams_t;

enum AwbReconfigParams_e {
  AWB_RECONFIG_NONE,
  AWB_RECONFIG_GAINS    = 0x1,
  AWB_RECONFIG_CCMATRIX = 0x1 << 1,
  AWB_RECONFIG_CCOFFSET = 0x1 << 2,
  AWB_RECONFIG_LSCMATRIX = 0x1 << 3,
  AWB_RECONFIG_LSCSECTOR = 0x1 << 4,
  AWB_RECONFIG_MEASMODE = 0x1 << 5,
  AWB_RECONFIG_MEASCFG  = 0x1 << 6,
  AWB_RECONFIG_AWBWIN  = 0x1 << 7,

};
typedef struct AwbRunningOutputResult_s {
  uint32_t            validParam;
  AwbGains_t                      WbGains;
  Cam3x3FloatMatrix_t             CcMatrix;             /**< damped color correction matrix */
  Cam1x3FloatMatrix_t             CcOffset;             /**< damped color correction offset */
  CamLscMatrix_t                  LscMatrixTable;       /**< damped lsc matrix */
  CamerIcIspLscSectorConfig_t     SectorConfig;               /**< lsc grid */
  CamerIcIspAwbMeasuringMode_t    MeasMode;           /**< specifies the means measuring mode (YCbCr or RGB) */
  CamerIcAwbMeasuringConfig_t     MeasConfig;         /**< measuring config */
  Cam_Win_t           awbWin;
  uint8_t             DoorType;
  int err_code;
  char IllName[20]; //yamasaki
  CamLscProfileName_t     aLscpflName1;
  CamLscProfileName_t     aLscpflName2;
} AwbRunningOutputResult_t;


/*****************************************************************************/
/**
 *          AwbInstanceConfig_t
 *
 * @brief   AWB Module instance configuration structure
 *
 *****************************************************************************/
typedef struct AwbInstanceConfig_s {
  AwbHandle_t                     hAwb;               /**< handle returns by V11_AwbInit() */
} AwbInstanceConfig_t;



/*****************************************************************************/
/**
 *          AwbConfig_t
 *
 * @brief   AWB Module configuration structure
 *
 *****************************************************************************/
typedef struct AwbConfig_s {
  AwbMode_t                       Mode;               /**< White Balance working mode (MANUAL | AUTO) */
  uint32_t                idx;
  bool_t              damp;

  uint16_t                        width;              /**< picture width */
  uint16_t                        height;             /**< picture height */
  float                           framerate;          /**< frame rate */
  Cam_Win_t           awbWin;
  uint32_t                        Flags;              /**< working flags (@see AwbWorkingFlags_e) */
  CamCalibDbHandle_t              hCamCalibDb;        /**< calibration database handle */
  CamerIcIspAwbMeasuringMode_t    MeasMode;           /**< specifies the means measuring mode (YCbCr or RGB) */
  CamerIcAwbMeasuringConfig_t     MeasConfig;         /**< measuring config */
  float                           fStableDeviation;   /**< min deviation in percent to enter stable state */
  float                           fRestartDeviation;  /**< max tolerated deviation in precent for staying in stable state */

} AwbConfig_t;



/*****************************************************************************/
/**
 *          AwbRgProj_t
 *
 * @brief   AWB Projection Borders in R/G Layer
 *
 *****************************************************************************/
typedef struct AwbRgProj_s {
  float                           fRgProjIndoorMin;
  float                           fRgProjOutdoorMin;
  float                           fRgProjMax;
  float                           fRgProjMaxSky;

  float               fRgProjALimit;    //oyyf
  float             fRgProjAWeight;   //oyyf
  float               fRgProjYellowLimit;   //oyyf
  float             fRgProjIllToCwf;    //oyyf
  float             fRgProjIllToCwfWeight;  //oyyf
} AwbRgProj_t;

/*****************************************************************************/
/**
 *          AwbWhitePoint_t
 *
 * @brief
 *
 *****************************************************************************/
typedef struct AwbWhitePoint_s {
  uint16_t win_h_offs;
  uint16_t win_v_offs;
  uint16_t win_width;
  uint16_t win_height;
  uint8_t awb_mode;
  uint32_t cnt;
  uint8_t mean_y;
  uint8_t mean_cb;
  uint8_t mean_cr;
  uint16_t mean_r;
  uint16_t mean_b;
  uint16_t mean_g;

  uint8_t RefCr;
  uint8_t RefCb;
  uint8_t MinY;
  uint8_t MaxY;
  uint8_t MinC;
  uint8_t MaxCSum;

  float RgProjection;
  float RegionSize;
  float Rg_clipped;
  float Rg_unclipped;
  float Bg_clipped;
  float Bg_unclipped;
} AwbWhitePoint_t;


#ifdef __cplusplus
}
#endif

/* @} AWBM */



#endif /* __AWB_H__*/


