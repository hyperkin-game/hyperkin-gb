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
#ifndef __AWB_V10_H__
#define __AWB_V10_H__

/**
 * @file awb_v10.h
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
#include "awb_common.h"
#ifdef __cplusplus
extern "C"
{
#endif




/*****************************************************************************/
/**
 * @brief   This function initializes the Auto White Balance Module.
 *
 * @param   handle      AWB instance handle
 * @param   pInstConfig pointer instance configuration structure
 *
 * @return  Returns the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_INVALID_PARM
 * @retval  RET_OUTOFMEM
 *
 *****************************************************************************/
RESULT V10_AwbInit
(
    AwbInstanceConfig_t* pInstConfig
);

/******************************************************************************
 * V10_AwbConfigure()
 *****************************************************************************/
RESULT V10_AwbConfigure
(
    AwbHandle_t handle,
    AwbConfig_t* pConfig
);

/******************************************************************************
 * V10_AwbReConfigure()
 *****************************************************************************/
RESULT V10_AwbReConfigure
(
    AwbHandle_t handle,
    AwbConfig_t* pConfig
);



/*****************************************************************************/
/**
 * @brief   The function releases/frees the Auto White Balance module.
 *
 * @param   handle      AWB instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT V10_AwbRelease
(
    AwbHandle_t handle
);


/*****************************************************************************/
/**
 * @brief   This function returns true if the AWB hit the convergence criteria.
 *
 * @param   handle      AWB instance handle
 *
 * @return  Returns the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_WRONG_STATE
 *
 *****************************************************************************/
RESULT V10_AwbStable
(
    AwbHandle_t handle,
    bool_t*      pStable
);



/*****************************************************************************/
/**
 * @brief   The function
 *
 * @param   handle      AWB instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT V10_AwbStart
(
    AwbHandle_t         handle,
    AwbConfig_t*     pcfg
);



/*****************************************************************************/
/**
 * @brief   The function stops the auto white balance.
 *
 * @param   handle      AWB instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT V10_AwbStop
(
    AwbHandle_t         handle
);


/*****************************************************************************/
/**
 * @brief   The function returns current status values of the AWB.
 *
 * @param   handle      AWB instance handle
 * @param   pRunning    pointer to return current run-state of AWB module
 * @param   pMode       pointer to return current operation mode of AWB module
 * @param   pIlluIdx    pointer to return current start profile index
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT V10_AwbStatus
(
    AwbHandle_t     handle,
    bool_t*          pRunning,      /**< BOOL_TRUE: running, BOOL_FALSE: stopped */
    AwbMode_t*       pMode,
    uint32_t*        pIlluIdx,
    AwbRgProj_t*     pRgProj
);


/******************************************************************************
 * V10_AwbSettled()
 *****************************************************************************/
RESULT V10_AwbSettled
(
    AwbHandle_t handle,
    bool_t*      pSettled,
    uint32_t*  pDNoWhitePixel
);

/*****************************************************************************/
/**
 * @brief   The function starts AWB processing
 *
 * @param   handle      AWB instance handle
 * @param   pMeasResult pointer tu current AWB measuring data from hardware
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT V10_AwbRun
(
    AwbHandle_t                         handle,
    AwbRunningInputParams_t* pMeasResult,
    AwbRunningOutputResult_t* pOutResult
);



/*****************************************************************************/
/**
 * @brief   The function
 *
 * @param   handle  AWB instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT V10_AwbTryLock
(
    AwbHandle_t handle
);



/*****************************************************************************/
/**
 * @brief   The function
 *
 * @param   handle  AWB instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT V10_AwbUnLock
(
    AwbHandle_t handle
);

RESULT V10_AwbGetGainParam
(
    AwbHandle_t handle,
    float* f_RgProj,
    float* f_s,
    float* f_s_Max1,
    float* f_s_Max2,
    float* f_Bg1,
    float* f_Rg1,
    float* f_Bg2,
    float* f_Rg2
);

RESULT V10_AwbGetIlluEstInfo
(
    AwbHandle_t handle,
    float* ExpPriorIn,
    float* ExpPriorOut,
    char (*name)[20],
    float likehood[],
    float wight[],
    int* curIdx,
    int* region,
    int* count
);

RESULT V10_AwbSetForceWbGain
(
    AwbHandle_t handle,
    bool_t forceWbGainFlag,
    float fRGain,
    float fGrGain,
    float fGbGain,
    float fBGain
);

RESULT V10_AwbSetForceIllumination
(
    AwbHandle_t handle,
    bool_t forceIlluFlag,
    char *illName
);

RESULT V10_AwbSetForceWpMeas
(
    AwbHandle_t handle,
    AwbMeasuringResult_t* pMeasResult
);

/*RESULT V10_AwbSetRefWbGain
(
    AwbHandle_t handle,
    float fRGain,
    float fGrGain,
    float fGbGain,
    float fBGain,
    char *illName
);*/

RESULT V10_AwbGetForceWbGain
(
    AwbHandle_t handle,
    bool_t* forceWbGainFlag,
    float* fRGain,
    float* fGrGain,
    float* fGbGain,
    float* fBGain
);

RESULT V10_AwbGetForceIllumination
(
    AwbHandle_t handle,
    bool_t* forceIlluFlag,
    char* illName
);

/*RESULT V10_AwbGetRefWbGain
(
    AwbHandle_t handle,
    float* fRGain,
    float* fGrGain,
    float* fGbGain,
    float* fBGain,
    char* illName
);*/

RESULT V10_AwbGetWhitePoint
(
    AwbHandle_t handle,
    AwbWhitePoint_t* wp
);

RESULT V10_AwbGetCcInfo
(
    AwbHandle_t handle,
    char* aCCDnName,
    char* aCCUpName,
    Cam3x3FloatMatrix_t* CcMatrix,
    Cam1x3FloatMatrix_t* CcOffset
);

#ifdef __cplusplus
}
#endif

/* @} AWBM */


#endif /* __AWB_V10_H__*/
