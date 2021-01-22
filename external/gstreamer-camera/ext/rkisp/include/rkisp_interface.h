#ifndef RKISP_X3A_INTERFACE
#define RKISP_X3A_INTERFACE

#include "ia_types.h"
#include "ia_cmc_types.h"
#include "ia_mkn_types.h"
#include "ia_abstraction.h"
#include "ia_aiq_types.h"
#include "ia_isp_types.h"
#include "ia_coordinate.h"
#include "ia_isp_2_2.h"
#include "ia_aiq.h"
#include "ia_cmc_parser.h"
#include "ia_mkn_encoder.h"

#include "rk_intel_bufmgr.h"

#include <cam_ia_api/cam_ia10_engine_api.h>

#include <calib_xml/calibdb.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef USE_CAM_IA_ENGINE
/*********************STATS**************
* X3A CTRL STATS
*****************************************/
#define CIFISP_AE_MEAN_MAX 25
#define CIFISP_HIST_BIN_N_MAX 16

typedef struct AecStat_s {
  unsigned char  exp_mean[CIFISP_AE_MEAN_MAX];
  unsigned int hist_bins[CIFISP_HIST_BIN_N_MAX];
} AecStat_t;

typedef struct CamerIcAwbMeasuringResult_s {
  uint32_t    NoWhitePixel;           /**< number of white pixel */
  uint8_t     MeanY__G;               /**< Y/G  value in YCbCr/RGB Mode */
  uint8_t     MeanCb__B;              /**< Cb/B value in YCbCr/RGB Mode */
  uint8_t     MeanCr__R;              /**< Cr/R value in YCbCr/RGB Mode */
} CamerIcAwbMeasuringResult_t;

struct CamIA10_Stats {
  unsigned int meas_type;
  AecStat_t aec;
  CamerIcAwbMeasuringResult_t awb;
};
//**************************************************

/**********************RESULTS********************
* X3A CTRL RESULTS
***************************************************/
typedef enum AecMeasuringMode_e {
  AEC_MEASURING_MODE_INVALID    = 0,    /**< invalid histogram measuring mode   */
  AEC_MEASURING_MODE_1          = 1,    /**< Y = (R + G + B) x (85/256)         */
  AEC_MEASURING_MODE_2          = 2,    /**< Y = 16 + 0.25R + 0.5G + 0.1094B    */
  AEC_MEASURING_MODE_MAX,
} AecMeasuringMode_t;

typedef struct Cam_Win {
  unsigned short h_offs;
  unsigned short v_offs;
  unsigned short h_size;
  unsigned short v_size;
} Cam_Win_t;

typedef enum CamerIcIspHistMode_e {
  CAMERIC_ISP_HIST_MODE_INVALID       = 0,    /**< lower border (only for an internal evaluation) */
  CAMERIC_ISP_HIST_MODE_RGB_COMBINED  = 1,    /**< RGB combined histogram */
  CAMERIC_ISP_HIST_MODE_R             = 2,    /**< R histogram */
  CAMERIC_ISP_HIST_MODE_G             = 3,    /**< G histogram */
  CAMERIC_ISP_HIST_MODE_B             = 4,    /**< B histogram */
  CAMERIC_ISP_HIST_MODE_Y             = 5,    /**< luminance histogram */
  CAMERIC_ISP_HIST_MODE_MAX,                                  /**< upper border (only for an internal evaluation) */
} CamerIcIspHistMode_t;

typedef struct AecResult_s {
  float coarse_integration_time;
  float analog_gain_code_global;
  int regIntegrationTime;
  int regGain;
  float PixelClockFreqMHZ;
  float PixelPeriodsPerLine;
  float LinePeriodsPerField;

  AecMeasuringMode_t meas_mode;
  struct Cam_Win meas_win;
  unsigned int actives;
  unsigned char GridWeights[CIFISP_AE_MEAN_MAX];
  uint8_t stepSize;
  CamerIcIspHistMode_t HistMode;
  float gainFactor;
  float gainBias;
  bool_t aoe_enable;
  bool auto_adjust_fps;
  enum LIGHT_MODE DON_LightMode;
  float DON_Fac;
  float MeanLuma;
  float MaxGainRange;
  uint8_t Night_Trigger;
  uint8_t Night_Mode;
  float overHistPercent;

  float MinGain;
  float MaxGain;
  float MinIntegrationTime;
  float MaxIntegrationTime;
} AecResult_t;

typedef struct CamerIcGains_s {
  uint16_t Red;                       /**< gain value for the red channel */
  uint16_t GreenR;                    /**< gain value for the green-red channel */
  uint16_t GreenB;                    /**< gain value for the green-blue channel */
  uint16_t Blue;                      /**< gain value for the blue channel */
} CamerIcGains_t;

typedef struct CamerIc3x3Matrix_s {
  uint32_t    Coeff[9U];               /**< array of 3x3 float values */
} CamerIc3x3Matrix_t;

typedef struct CamerIcXTalkOffset_s {
  uint16_t Red;                       /**< offset value for the red channel */
  uint16_t Green;                     /**< offset value for the green channel */
  uint16_t Blue;                      /**< offset value for the blue channel */
} CamerIcXTalkOffset_t;

typedef struct Cam17x17UShortMatrix_s {
  uint16_t uCoeff[17 * 17];
} Cam17x17UShortMatrix_t;

typedef struct CamLscMatrix_s {
  Cam17x17UShortMatrix_t  LscMatrix[CAM_4CH_COLOR_COMPONENT_MAX];
} CamLscMatrix_t;

typedef struct CamerIcIspLscSectorConfig_s {
  uint16_t LscXGradTbl[CAEMRIC_GRAD_TBL_SIZE];    /**< multiplication factors of x direction  */
  uint16_t LscYGradTbl[CAEMRIC_GRAD_TBL_SIZE];    /**< multiplication factors of y direction  */
  uint16_t LscXSizeTbl[CAEMRIC_GRAD_TBL_SIZE];    /**< sector sizes of x direction            */
  uint16_t LscYSizeTbl[CAEMRIC_GRAD_TBL_SIZE];    /**< sector sizes of y direction            */
} CamerIcIspLscSectorConfig_t;

typedef enum CamerIcIspAwbMeasuringMode_e {
  CAMERIC_ISP_AWB_MEASURING_MODE_INVALID    = 0,      /**< lower border (only for an internal evaluation) */
  CAMERIC_ISP_AWB_MEASURING_MODE_YCBCR      = 1,      /**< near white discrimination mode using YCbCr color space */
  CAMERIC_ISP_AWB_MEASURING_MODE_RGB        = 2,      /**< RGB based measurement mode */
  CAMERIC_ISP_AWB_MEASURING_MODE_MAX,                 /**< upper border (only for an internal evaluation) */
} CamerIcIspAwbMeasuringMode_t;

typedef struct CamerIcAwbMeasuringConfig_s {
  uint8_t MaxY;           /**< YCbCr Mode: only pixels values Y <= ucMaxY contribute to WB measurement (set to 0 to disable this feature) */
  /**< RGB Mode  : unused */
  uint8_t RefCr_MaxR;     /**< YCbCr Mode: Cr reference value */
  /**< RGB Mode  : only pixels values R < MaxR contribute to WB measurement */
  uint8_t MinY_MaxG;      /**< YCbCr Mode: only pixels values Y >= ucMinY contribute to WB measurement */
  /**< RGB Mode  : only pixels values G < MaxG contribute to WB measurement */
  uint8_t RefCb_MaxB;     /**< YCbCr Mode: Cb reference value */
  /**< RGB Mode  : only pixels values B < MaxB contribute to WB measurement */
  uint8_t MaxCSum;        /**< YCbCr Mode: chrominance sum maximum value, only consider pixels with Cb+Cr smaller than threshold for WB measurements */
  /**< RGB Mode  : unused */
  uint8_t MinC;           /**< YCbCr Mode: chrominance minimum value, only consider pixels with Cb/Cr each greater than threshold value for WB measurements */
  /**< RGB Mode  : unused */
} CamerIcAwbMeasuringConfig_t;

typedef struct CamIA10_AWB_Result_s {
  int actives;
  CamerIcGains_t awbGains;
  CamerIc3x3Matrix_t CcMatrix;
  CamerIcXTalkOffset_t CcOffset;
  CamLscMatrix_t                  LscMatrixTable;       /**< damped lsc matrix */
  CamerIcIspLscSectorConfig_t     SectorConfig;               /**< lsc grid */
  CamerIcIspAwbMeasuringMode_t    MeasMode;           /**< specifies the means measuring mode (YCbCr or RGB) */
  CamerIcAwbMeasuringConfig_t     MeasConfig;         /**< measuring config */
  Cam_Win_t           awbWin;
  uint8_t               DoorType;
  int err_code;
} CamIA10_AWB_Result_t;
#endif

/* -------- CamIsp10CtrItf interface -----------*/
int
rkisp_start(void* &engine, int vidFd, const char* ispNode, const char* tuningFile);

int
rkisp_stop(void* &engine);

/* -------- CamIA10EngineItf interface ---------*/
int
rkisp_iq_init(void* engine, const char* tuningFile/*, struct CamIA10_DyCfg* ia_dcfg*/);

void
rkisp_iq_deinit(void* engine);

int
rkisp_iq_statistics_set(void* engine, struct CamIA10_Stats* ia_stats);

int
rkisp_iq_ae_run(void* engine);

int
rkisp_iq_get_aec_result(void* engine, AecResult_t* result);

int
rkisp_iq_af_run(void* engine);

int
rkisp_iq_get_af_result(void* engine, CamIA10_AFC_Result_t* result) ;

int
rkisp_iq_awb_run(void* engine);

int
rkisp_iq_get_awb_result(void* engine, CamIA10_AWB_Result_t* result);

int rkisp_getAeTime(void* &engine, float &time);
int rkisp_getAeGain(void* &engine, float &gain);
int rkisp_getAeMaxExposureTime(void* &engine, float &time);
int rkisp_getAeMaxExposureGain(void* &engine, float &gain);
int rkisp_setAeMaxExposureTime(void* &engine, float time);
int rkisp_setAeMaxExposureGain(void* &engine, float gain);
int rkisp_getAeMeanLuma(void* &engine, int &meanLuma);
int rkisp_setWhiteBalance(void* &engine, HAL_WB_MODE wbMode);
int rkisp_setAeMode(void* &engine, enum HAL_AE_OPERATION_MODE aeMode);
int rkisp_setManualGainAndTime(void* &engine, float hal_gain, float hal_time);
int rkisp_setAntiBandMode(void* &engine, enum HAL_AE_FLK_MODE flkMode);
int rkisp_setAeBias(void* &engine, int aeBias);
int rkisp_setFps(void* &engine, HAL_FPS_INFO_t fps);
int rkisp_setAeWindow(void* &engine, int left_hoff, int top_voff, int right_width, int bottom_height);
int rkisp_getAeWindow(void* &engine, int &left_hoff, int &top_voff, int &right_width, int &bottom_height);
int rkisp_setExposureMeterMode(void* &engine, enum HAL_AE_METERING_MODE aeMeterMode);
int rkisp_getExposureMeterMode(void* &engine, enum HAL_AE_METERING_MODE& aeMeterMode);
int rkisp_setExposureMeterCoeff(void* &engine, unsigned char meter_coeff[]);
int rkisp_getExposureMeterCoeff(void* &engine, unsigned char meter_coeff[]);
int rkisp_setAeSetPoint(void* &engine, float set_point);
int rkisp_getAeSetPoint(void* &engine, float &set_point);
int rkisp_set3ALocks(void* &engine, int locks);
int rkisp_get3ALocks(void* &engine, int& curLocks);
int rkisp_setFocusMode(void* &engine, enum HAL_AF_MODE fcMode);
int rkisp_getFocusMode(void* &engine, enum HAL_AF_MODE& fcMode);
int rkisp_setFocusWin(void* &engine, HAL_Window_t afwin);
int rkisp_getFocusWin(void* &engine, HAL_Window_t& afwin);
int rkisp_trigggerAf(void* &engine, bool trigger);

int rkisp_getBrightness(void* &engine, int& brightVal);
int rkisp_getContrast(void* &engine, int& contrast);
int rkisp_getSaturation(void* &engine, int& sat);
int rkisp_getHue(void* &engine, int& hue);
int rkisp_setBrightness(void* &engine, int brightVal);
int rkisp_setContrast(void* &engine, int contrast);
int rkisp_setSaturation(void* &engine, int sat);
int rkisp_setHue(void* &engine, int hue);
int rkisp_setFilterLevel(void* &engine, enum HAL_MODE_e mode,
	enum HAL_FLT_DENOISE_LEVEL_e denoise, enum HAL_FLT_SHARPENING_LEVEL_e sharp);
int rkisp_getFilterLevel(enum HAL_MODE_e& mode,
	enum HAL_FLT_DENOISE_LEVEL_e& denoise, enum HAL_FLT_SHARPENING_LEVEL_e& sharp);

int rkisp_setNightMode(void* &engine, bool night_mode);
int rkisp_getNightMode(void* &engine, bool& night_mode);
int rkisp_setDayNightSwitch(void* &engine, enum HAL_DAYNIGHT_MODE sw);
int rkisp_getDayNightSwitch(void* &engine, enum HAL_DAYNIGHT_MODE& sw);

#ifdef __cplusplus
}
#endif

#endif
