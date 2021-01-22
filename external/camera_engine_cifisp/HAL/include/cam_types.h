#ifndef _CAM_TYPES_H_
#define _CAM_TYPES_H_

#include "ebase/types.h"
#include <sys/time.h>
#define HAL_HIST_BIN_N_MAX 32
#define HAL_ISP_SDG_SECTION_MAX 16
#define HAL_ISP_GOC_SECTION_MAX 34
#define HAL_ISP_DATA_TBL_SIZE_MAX 289
#define HAL_ISP_GRAD_TBL_SIZE_MAX 8
#define HAL_ISP_AFM_MAX_WINDOWS 3
#define HAL_ISP_HIST_GRID_ITEMS         25  /**< number of grid sub windows */
#define HAL_ISP_WDR_SECTION_MAX 32

#define HAL_ISP_BPC_MASK    (1 << 0)
#define HAL_ISP_BLS_MASK    (1 << 1)
#define HAL_ISP_SDG_MASK    (1 << 2)
#define HAL_ISP_HST_MASK    (1 << 3)
#define HAL_ISP_LSC_MASK    (1 << 4)
#define HAL_ISP_AWB_GAIN_MASK (1 << 5)
#define HAL_ISP_FLT_MASK  (1 << 6)
#define HAL_ISP_BDM_MASK  (1 << 7)
#define HAL_ISP_CTK_MASK  (1 << 8)
#define HAL_ISP_GOC_MASK  (1 << 9)
#define HAL_ISP_CPROC_MASK  (1 << 10)
#define HAL_ISP_AFC_MASK  (1 << 11)
#define HAL_ISP_AWB_MEAS_MASK (1 << 12)
#define HAL_ISP_IE_MASK (1 << 13)
#define HAL_ISP_AEC_MASK  (1 << 14)
#define HAL_ISP_WDR_MASK  (1 << 15)
#define HAL_ISP_DPF_MASK  (1 << 16)
#define HAL_ISP_DPF_STRENGTH_MASK (1 << 17)
#define HAL_ISP_3DNR_MASK  (1 << 18)
#define HAL_ISP_AWB_LSC_MASK (1 << 19)
#define HAL_ISP_ADPF_DPF_MASK (1 << 20)
#define HAL_ISP_ADPF_FLT_MASK (1 << 21)
#define HAL_ISP_AWB_CCM_MASK (1 << 22)
#define HAL_ISP_AWB_GAIN_ILLU_MASK (1 << 23)
#define HAL_ISP_AWB_REFGAIN_MASK (1 << 24)
#define HAL_ISP_AWB_CURVE_MASK (1 << 25)
#define HAL_ISP_AWB_WP_SET_MASK (1 << 26)
#define HAL_ISP_CALIBDB_GOC_MASK (1 << 27)
#define HAL_ISP_CALIBDB_CPROC_MASK (1 << 28)
#define HAL_ISP_NEW_3DNR_MASK  (1 << 29)


#define HAL_ISP_ALL_MASK  (0xffffffff)

enum HAL_ISP_SUB_MODULE_ID_e {
  HAL_ISP_BPC_ID,
  HAL_ISP_BLS_ID,
  HAL_ISP_SDG_ID,
  HAL_ISP_HST_ID,
  HAL_ISP_LSC_ID,
  HAL_ISP_AWB_GAIN_ID,
  HAL_ISP_FLT_ID,
  HAL_ISP_BDM_ID,
  HAL_ISP_CTK_ID,
  HAL_ISP_GOC_ID,
  HAL_ISP_CPROC_ID,
  HAL_ISP_AFC_ID,
  HAL_ISP_AWB_MEAS_ID,
  HAL_ISP_IE_ID,
  HAL_ISP_AEC_ID,
  HAL_ISP_WDR_ID,
  HAL_ISP_DPF_ID,
  HAL_ISP_DPF_STRENGTH_ID,
  HAL_ISP_3DNR_ID,
  HAL_ISP_NEW_3DNR_ID,
  HAL_ISP_AWB_LSC_ID,
  HAL_ISP_ADPF_DPF_ID,
  HAL_ISP_ADPF_FLT_ID,
  HAL_ISP_AWB_CCM_ID,
  HAL_ISP_AWB_GAIN_ILLU_ID,
  HAL_ISP_AWB_REFGAIN_ID,
  HAL_ISP_AWB_CURVE_ID,
  HAL_ISP_AWB_WP_SET_ID,
  HAL_ISP_AWB_WP_GET_ID,
  HAL_ISP_CALIBDB_GOC_ID,
  HAL_ISP_CALIBDB_CPROC_ID,
  HAL_ISP_MODULE_MAX_ID_ID,
};

enum CAMISP_CTRL_MODE
{
    CAMISP_CTRL_MASTER = 0,
    CAMISP_CTRL_SLAVE = 1
};

typedef struct frm_size_s {
  unsigned int width;
  unsigned int height;
} frm_size_t;

typedef enum RK_FRMAE_FORMAT_e {
  HAL_FRMAE_FMT_NV12,
  HAL_FRMAE_FMT_NV21,
  HAL_FRMAE_FMT_YVU420,
  HAL_FRMAE_FMT_RGB565,
  HAL_FRMAE_FMT_RGB32,
  HAL_FRMAE_FMT_YUV422P,
  HAL_FRMAE_FMT_NV16,
  HAL_FRMAE_FMT_YUYV,
  HAL_FRMAE_FMT_JPEG,
  HAL_FRMAE_FMT_MJPEG,
  HAL_FRMAE_FMT_H264,
  HAL_FRMAE_FMT_Y8,
  HAL_FRMAE_FMT_Y10,
  HAL_FRMAE_FMT_Y12,
  //RAW fmts
  HAL_FRMAE_FMT_SBGGR12,
  HAL_FRMAE_FMT_SGBRG12,
  HAL_FRMAE_FMT_SGRBG12,
  HAL_FRMAE_FMT_SRGGB12,
  HAL_FRMAE_FMT_SBGGR10,
  HAL_FRMAE_FMT_SGBRG10,
  HAL_FRMAE_FMT_SGRBG10,
  HAL_FRMAE_FMT_SRGGB10,
  HAL_FRMAE_FMT_SBGGR8,
  HAL_FRMAE_FMT_SGBRG8,
  HAL_FRMAE_FMT_SGRBG8,
  HAL_FRMAE_FMT_SRGGB8,
  HAL_FRMAE_FMT_MAX,
} RK_FRMAE_FORMAT;

typedef enum HAL_COLORSPACE_e {
  /* ITU-R 601 -- broadcast NTSC/PAL */
  HAL_COLORSPACE_SMPTE170M = 1,
  /* I know there will be cameras that send this.  So, this is
  * unspecified chromaticities and full 0-255 on each of the
  * Y'CbCr components
  */
  HAL_COLORSPACE_JPEG = 7,
} HAL_COLORSPACE;

typedef struct frm_info_s {
  frm_size_t frmSize;
  RK_FRMAE_FORMAT frmFmt;
  HAL_COLORSPACE colorSpace;
  unsigned int fps;
} frm_info_t;

class RK_HAL_FMT_STRING {
 public:
  static const char HAL_FMT_STRING_NV12[];
  static const char HAL_FMT_STRING_NV21[];
  static const char HAL_FMT_STRING_YVU420[];
  static const char HAL_FMT_STRING_RGB565[];
  static const char HAL_FMT_STRING_RGB32[];
  static const char HAL_FMT_STRING_YUV422P[];
  static const char HAL_FMT_STRING_NV16[];
  static const char HAL_FMT_STRING_YUYV[];
  static const char HAL_FMT_STRING_JPEG[];
  static const char HAL_FMT_STRING_MJPEG[];
  static const char HAL_FMT_STRING_H264[];
  static const char HAL_FMT_STRING_Y8[];
  static const char HAL_FMT_STRING_Y10[];
  static const char HAL_FMT_STRING_Y12[];
  static const char HAL_FMT_STRING_SBGGR12[];
  static const char HAL_FMT_STRING_SGBRG12[];
  static const char HAL_FMT_STRING_SGRBG12[];
  static const char HAL_FMT_STRING_SRGGB12[];
  static const char HAL_FMT_STRING_SBGGR10[];
  static const char HAL_FMT_STRING_SGBRG10[];
  static const char HAL_FMT_STRING_SGRBG10[];
  static const char HAL_FMT_STRING_SRGGB10[];
  static const char HAL_FMT_STRING_SBGGR8[];
  static const char HAL_FMT_STRING_SGBRG8[];
  static const char HAL_FMT_STRING_SGRBG8[];
  static const char HAL_FMT_STRING_SRGGB8[];
  static const char* hal_fmt_map_to_str(RK_FRMAE_FORMAT fmt) {
    switch (fmt) {
      case HAL_FRMAE_FMT_NV12 :
        return HAL_FMT_STRING_NV12;
      case HAL_FRMAE_FMT_NV21 :
        return HAL_FMT_STRING_NV21;
      case HAL_FRMAE_FMT_YVU420 :
        return HAL_FMT_STRING_YVU420;
      case HAL_FRMAE_FMT_RGB565 :
        return HAL_FMT_STRING_RGB565;
      case HAL_FRMAE_FMT_RGB32 :
        return HAL_FMT_STRING_RGB32;
      case HAL_FRMAE_FMT_YUV422P :
        return HAL_FMT_STRING_YUV422P;
      case HAL_FRMAE_FMT_NV16 :
        return HAL_FMT_STRING_NV16;
      case HAL_FRMAE_FMT_YUYV :
        return HAL_FMT_STRING_YUYV;
      case HAL_FRMAE_FMT_JPEG :
        return HAL_FMT_STRING_JPEG;
      case HAL_FRMAE_FMT_MJPEG :
        return HAL_FMT_STRING_MJPEG;
      case HAL_FRMAE_FMT_H264 :
        return HAL_FMT_STRING_H264;
      case HAL_FRMAE_FMT_Y8 :
        return HAL_FMT_STRING_Y8;
      case HAL_FRMAE_FMT_Y10 :
        return HAL_FMT_STRING_Y10;
      case HAL_FRMAE_FMT_Y12 :
        return HAL_FMT_STRING_Y12;
      case HAL_FRMAE_FMT_SBGGR12:
        return HAL_FMT_STRING_SBGGR12;
      case HAL_FRMAE_FMT_SGBRG12:
        return HAL_FMT_STRING_SGBRG12;
      case HAL_FRMAE_FMT_SGRBG12:
        return HAL_FMT_STRING_SGRBG12;
      case HAL_FRMAE_FMT_SRGGB12:
        return HAL_FMT_STRING_SRGGB12;
      case HAL_FRMAE_FMT_SBGGR10 :
        return HAL_FMT_STRING_SBGGR10;
      case HAL_FRMAE_FMT_SGBRG10:
        return HAL_FMT_STRING_SGBRG10;
      case HAL_FRMAE_FMT_SGRBG10:
        return HAL_FMT_STRING_SGRBG10;
      case HAL_FRMAE_FMT_SRGGB10:
        return HAL_FMT_STRING_SRGGB10;
      case HAL_FRMAE_FMT_SBGGR8 :
        return HAL_FMT_STRING_SBGGR8;
      case HAL_FRMAE_FMT_SGBRG8:
        return HAL_FMT_STRING_SGBRG8;
      case HAL_FRMAE_FMT_SGRBG8:
        return HAL_FMT_STRING_SGRBG8;
      case HAL_FRMAE_FMT_SRGGB8:
        return HAL_FMT_STRING_SRGGB8;
      default:
        return NULL;

    }
  };
};

#define HAL_WIN_REF_WIDTH 2048
#define HAL_WIN_REF_HEIGHT 2048

typedef struct HAL_Window {
  int left_hoff;
  int top_voff;
  int right_width;
  int bottom_height;
} HAL_Window_t;

enum HAL_IAMGE_EFFECT {
  HAL_EFFECT_NONE,
  HAL_EFFECT_MONO, //equal grayscale
  HAL_EFFECT_NEGATIVE,
  HAL_EFFECT_SEPIA,
  //HAL_EFFECT_COLOR,
  HAL_EFFECT_EMBOSS,
  HAL_EFFECT_SKETCH,
  HAL_EFFECT_SHARPEN
};

enum HAL_WB_MODE {
  HAL_WB_INVAL = -1,
  HAL_WB_INCANDESCENT = 0,
  HAL_WB_FLUORESCENT,
  HAL_WB_DAYLIGHT,
  HAL_WB_CLOUDY_DAYLIGHT,
  HAL_WB_AUTO,
  HAL_WB_SUNSET,
  HAL_WB_MANUAL_CCT,
  HAL_WB_BEACH,
  HAL_WB_SNOW,
  HAL_WB_CANDLE,
  HAL_WB_MAX
};

enum HAL_SCENE_MODE {
  HAL_SCENE_MODE_AUTO,
  HAL_SCENE_MODE_BACKLIGHT,
  HAL_SCENE_MODE_BEACH_SNOW,
  HAL_SCENE_MODE_CANDLE_LIGHT,
  HAL_SCENE_MODE_DAWN_DUSK,
  HAL_SCENE_MODE_FALL_COLORS,
  HAL_SCENE_MODE_FIREWORKS,
  HAL_SCENE_MODE_LANDSCAPE,
  HAL_SCENE_MODE_NIGHT,
  HAL_SCENE_MODE_PARTY_INDOOR,
  HAL_SCENE_MODE_PORTRAIT,
  HAL_SCENE_MODE_SPORTS,
  HAL_SCENE_MODE_SUNSET,
  HAL_SCENE_MODE_TEXT,
};

enum HAL_AE_FLK_MODE {
  HAL_AE_FLK_OFF = 0,
  HAL_AE_FLK_50,
  HAL_AE_FLK_60,
  HAL_AE_FLK_AUTO,
};

enum HAL_AE_STATE {
  HAL_AE_STATE_UNSTABLE = 0,
  HAL_AE_STATE_STABLE
};

enum HAL_AF_MODE {
  HAL_AF_MODE_NOT_SET = -1,
  HAL_AF_MODE_AUTO,
  HAL_AF_MODE_MACRO,
  HAL_AF_MODE_INFINITY,
  HAL_AF_MODE_FIXED,
  HAL_AF_MODE_EDOF,
  HAL_AF_MODE_CONTINUOUS_VIDEO,
  HAL_AF_MODE_CONTINUOUS_PICTURE,
};

enum USE_CASE {
  UC_PREVIEW,
  UC_CAPTURE,
  UC_RECORDING,
  UC_RAW
};

enum HAL_AE_OPERATION_MODE {
  HAL_AE_OPERATION_MODE_AUTO,
  HAL_AE_OPERATION_MODE_MANUAL,
  HAL_AE_OPERATION_MODE_LONG_EXPOSURE,
  HAL_AE_OPERATION_MODE_ACTION,
  HAL_AE_OPERATION_MODE_VIDEO_CONFERENCE,
  HAL_AE_OPERATION_MODE_PRODUCT_TEST,
  HAL_AE_OPERATION_MODE_ULL,
  HAL_AE_OPERATION_MODE_FIREWORKS,
};

enum HAL_FLASH_MODE {
  HAL_FLASH_OFF,
  HAL_FLASH_ON,
  HAL_FLASH_AUTO,
  HAL_FLASH_TORCH
};

enum HAL_AE_METERING_MODE {
  HAL_AE_METERING_MODE_AVERAGE,
  HAL_AE_METERING_MODE_CENTER,
  HAL_AE_METERING_MODE_SPOT,
  HAL_AE_METERING_MODE_MATRIX,
  HAL_AE_METERING_MODE_USER,
};

enum HAL_AF_STATUS {
  HAL_AF_STATUS_IDLE,
  HAL_AF_STATUS_BUSY,
  HAL_AF_STATUS_REACHED,
  HAL_AF_STATUS_FAILED,
};

enum HAL_3A_LOCKS {
  HAL_3A_LOCKS_NONE  = 0,
  HAL_3A_LOCKS_FOCUS   = 0x1,
  HAL_3A_LOCKS_EXPOSURE    = 0x2,
  HAL_3A_LOCKS_WB    = 0x4,
  HAL_3A_LOCKS_ALL   = HAL_3A_LOCKS_FOCUS
                       | HAL_3A_LOCKS_EXPOSURE | HAL_3A_LOCKS_WB,
};


enum HAL_ISO_MODE {
  HAL_ISO_MODE_AUTO,
  HAL_ISO_MODE_MANUAL,

};

enum HAL_FLIP_MODE {
  HAL_FLIP_NONE,
  HAL_FLIP_H,
  HAL_FLIP_V
};

enum HAL_DAYNIGHT_MODE {
  HAL_DAYNIGHT_AUTO,
  HAL_DAYNIGHT_DAY,
  HAL_DAYNIGHT_NIGHT
};

typedef struct HAL_FPS_INFO_s {
  unsigned int numerator;
  unsigned int denominator;
} HAL_FPS_INFO_t;

typedef struct HAL_RANGES_s {
  int min;
  int max;
  int step;
} HAL_RANGES_t;

enum JPEG_QUALITY {
  JPEG_QUALITY_MIN  = 1,
  JPEG_QUALITY_MAX   = 95 /* Limited quality limits required JPEG buffer size (SMS06118100) */
};

enum HAL_MODE_e {
  HAL_MODE_OFF,
  HAL_MODE_AUTO,
  HAL_MODE_MANUAL
};

struct HAL_AwbCfg {
  HAL_WB_MODE mode;
  HAL_Window win;
};

struct HAL_AecCfg {
  HAL_AE_FLK_MODE flk;
  HAL_AE_OPERATION_MODE mode;
  HAL_AE_METERING_MODE meter_mode;
  HAL_Window win;
  unsigned char meter_coeff[5*5];
  int ae_bias;
  float set_point;
  float aeMaxExpTime;
  float aeMaxExpGain;
  float api_set_fps;
};

struct HAL_AfcType {
  unsigned int contrast_af:1;
  unsigned int laser_af:1;
  unsigned int pdaf:1;
};

struct HAL_AfcCfg {
  HAL_AF_MODE mode;
  struct HAL_AfcType type;
  bool_t oneshot_trigger;
  unsigned int win_num;
  HAL_Window win_a;
  HAL_Window win_b;
  HAL_Window win_c;
};

struct HAL_SensorModeData {
  unsigned int isp_input_width;
  unsigned int isp_input_height;
  unsigned int isp_output_width;
  unsigned int isp_output_height;

  unsigned int horizontal_crop_offset;
  unsigned int vertical_crop_offset;
  unsigned int cropped_image_width;
  unsigned int cropped_image_height;

  float pixel_clock_freq_mhz;
  unsigned int pixel_periods_per_line;
  unsigned int line_periods_per_field;
  unsigned int sensor_output_height;
  unsigned int fine_integration_time_min;
  unsigned int fine_integration_time_max_margin;
  unsigned int coarse_integration_time_min;
  unsigned int coarse_integration_time_max_margin;
  unsigned char exposure_valid_frame;
  int exp_time;
  unsigned short gain;

};

struct HAL_ColorProcCfg {
  /*range:  0.0  - 1.992*/
  float contrast;
  /* -90 deg - +87.188*/
  float hue;
  /*range : -128  - +127 */
  signed char brightness;
  /*range:  0.0  - 1.992*/
  float saturation;
  float sharpness;
};

enum  HAL_ISP_WDR_MODE_e {
  HAL_ISP_WDR_MODE_BLOCK,
  HAL_ISP_WDR_MODE_GLOBAL,
  HAL_ISP_WDR_MODE_INVALID
};

struct HAL_WdrCfg {
  enum HAL_MODE_e mode;
  bool_t curve_enable;
  HAL_ISP_WDR_MODE_e curve_mode;
  uint16_t curve_dy[HAL_ISP_WDR_SECTION_MAX + 1];
  bool_t gain_max_clip_enable;
  uint8_t gain_max_value;
};

struct HAL_hist_stat {
  unsigned short usedCnt;
  unsigned int hist_bins[HAL_HIST_BIN_N_MAX];
};

enum HAL_BLS_MODE {
  HAL_BLS_MODE_FIXED = 0,
  HAL_BLS_MODE_AUTO = 1
};

enum HAL_BLS_WINCFG {
  HAL_BLS_WINCFG_OFF = 0,
  HAL_BLS_WINCFG_WIN1 = 1,
  HAL_BLS_WINCFG_WIN2 = 2,
  HAL_BLS_WINCFG_WIN1_2 = 3
};

typedef struct HAL_Bls_Win {
  uint16_t h_offs;
  uint16_t v_offs;
  uint16_t width;
  uint16_t height;
} HAL_Bls_Win_t;

struct HAL_ISP_bls_cfg_s {
  enum HAL_BLS_MODE mode;
  enum HAL_BLS_WINCFG win_cfg;
  HAL_Bls_Win_t win1;
  HAL_Bls_Win_t win2;
  uint8_t samples;
  int16_t fixed_red;
  int16_t fixed_greenR;
  int16_t fixed_greenB;
  int16_t fixed_blue;
};

struct HAL_ISP_sdg_cfg_s {
  /*
  gamma curve point definition y-axis (output)  RESTRICTION:
  each Y must be in the +2047/-2048 range compared to its predecessor
  (so that the difference between successive Y values is 12-bit signed !)
  Note: The reset values define a linear curve which has the same effect as bypass.
  Reset values are:
  Y_00 = 0x0000, Y_01 = 0x0100, Y_02 = 0x0200, Y_03 = 0x0300,
  Y_04 = 0x0400, Y_05 = 0x0500, Y_06 = 0x0600, Y_07 = 0x0700,
  Y_08 = 0x0800, Y_09 = 0x0900, Y_10 = 0x0A00, Y_11 = 0x0B00,
  Y_12 = 0x0C00, Y_13 = 0x0D00, Y_14 = 0x0E00, Y_15 = 0x0F00,
  Y_16 = 0x0FFF
  */
  /*
  So the gamma_dx steps would be 16 , 32 , 64 ,
  128 , 256 , 512 , 1024 , 2048 .
  */
  uint8_t  used_dx_cnt;
  uint16_t gamma_b[HAL_ISP_SDG_SECTION_MAX + 1];
  uint16_t gamma_g[HAL_ISP_SDG_SECTION_MAX + 1];
  uint16_t gamma_r[HAL_ISP_SDG_SECTION_MAX + 1];
  uint16_t gamma_dx[HAL_ISP_SDG_SECTION_MAX];
};

struct HAL_ISP_flt_cfg_s {
  uint8_t denoise_level;
  uint8_t sharp_level;
  int light_mode; 
};

enum HAL_ISP_GAMMA_SEG_MODE_e {
  HAL_ISP_GAMMA_SEG_MODE_LOGARITHMIC   = 1,    /**< logarithmic segmentation from 0 to 4095
                                                             (64,64,64,64,128,128,128,128,256,256,256,512,512,512,512,512) */
  HAL_ISP_GAMMA_SEG_MODE_EQUIDISTANT   = 2,    /**< equidistant segmentation from 0 to 4095
                                                             (256, 256, ... ) */
};
struct HAL_ISP_goc_cfg_s {
  /*
  Gamma_out curve point definition y-axis (output) for all color components (red,green,blue)
  RESTRICTION: The difference between two Y_n (dy = Y_n - Y_n-1) is restricted to +511/-512 (10 bit signed)!
  */
  HAL_ISP_GAMMA_SEG_MODE_e  mode;
  uint8_t  used_cnt;
  uint16_t gamma_y[HAL_ISP_GOC_SECTION_MAX];
  int light_mode; 
};

enum HAL_ISP_COLOR_RANGE_e {
  HAL_ISP_COLOR_RANGE_OUT_BT601         = 1,
  HAL_ISP_COLOR_RANGE_OUT_FULL_RANGE    = 2,
};

struct HAL_ISP_cproc_cfg_s {
  enum USE_CASE use_case;
  HAL_ISP_COLOR_RANGE_e range;
  struct HAL_ColorProcCfg cproc;
};


struct HAL_ISP_ie_cfg_s {
  enum HAL_IAMGE_EFFECT                     mode;           /**< working mode (see @ref CamerIcIeMode_e) */
  HAL_ISP_COLOR_RANGE_e                    range;          /**< working range (see @ref CamerIcIeRange_e) */
  /* range: 0 to 15.75*/
  float sharp_fact;
  /*
  Threshold for coring function. This value is used to avoid amplifying noise too much by suppressing sharpening
  for small gradients. Higher value means less sharpening for smooth edges. Threshold zero means no coring,
  so all gradients are treated the same. Threshold 255 means nearly no sharpening. An absolute value for the
  highpass signal is defined here. The highpass signal is truncated at the defined level.
  */
  uint8_t sharp_thd;
} ;

struct HAL_ISP_lsc_cfg_s {
  bool_t lsc_enable;
  unsigned int lsc_max_data_tbl[HAL_ISP_DATA_TBL_SIZE_MAX];
  uint16_t data_table_cnt;
  uint16_t grad_table_cnt;
  uint16_t LscRDataTbl[HAL_ISP_DATA_TBL_SIZE_MAX];    /**< correction values of R color part */
  uint16_t LscGRDataTbl[HAL_ISP_DATA_TBL_SIZE_MAX];   /**< correction values of G (red lines) color part */
  uint16_t LscGBDataTbl[HAL_ISP_DATA_TBL_SIZE_MAX];   /**< correction values of G (blue lines) color part  */
  uint16_t LscBDataTbl[HAL_ISP_DATA_TBL_SIZE_MAX];    /**< correction values of B color part  */
  uint16_t LscXGradTbl[HAL_ISP_GRAD_TBL_SIZE_MAX];    /**< multiplication factors of x direction  */
  uint16_t LscYGradTbl[HAL_ISP_GRAD_TBL_SIZE_MAX];    /**< multiplication factors of y direction  */
  uint16_t LscXSizeTbl[HAL_ISP_GRAD_TBL_SIZE_MAX];    /**< sector sizes of x direction            */
  uint16_t LscYSizeTbl[HAL_ISP_GRAD_TBL_SIZE_MAX];    /**< sector sizes of y direction            */
};

struct HAL_ISP_awb_gain_cfg_s {
  float gain_red;
  float gain_green_r;
  float gain_blue;
  float gain_green_b;
};

struct HAL_ISP_awb_cfg_s {
  struct HAL_ISP_awb_gain_cfg_s wb_gain;
  uint8_t DoorType;
  char IllName[20];
};


enum HAL_ISP_CTK_UPDATE_e {
  HAL_ISP_CTK_UPDATE_CC_MATRIX = 1,
  HAL_ISP_CTK_UPDATE_CC_OFFSET = 2,
};
struct HAL_ISP_ctk_cfg_s {
  /* 3*3 matrix */
  /* each item range: -8 to 7.996*/
  float coeff0;
  float coeff1;
  float coeff2;
  float coeff3;
  float coeff4;
  float coeff5;
  float coeff6;
  float coeff7;
  float coeff8;
  /* offset for the crosstalk correction matrix */
  /* range : -2048.0 - 2047.0 */
  float ct_offset_r;
  float ct_offset_g;
  float ct_offset_b;
  int update_mask;
};

struct HAL_ISP_aec_cfg_s {
  /** mode 0: < Y = 16 + 0.25R + 0.5G + 0.1094B */
  /** mode 1:< Y = (R + G + B) x (85/256) */
  uint8_t       mode;       /**< measuring mode */
  HAL_Window      win;
  /* used when aec disabled */
  float exp_time;
  float exp_gain;
};

struct HAL_ISP_afc_cfg_s {
  uint8_t num_afm_win;  /* max CIFISP_AFM_MAX_WINDOWS */
  HAL_Window afm_win[HAL_ISP_AFM_MAX_WINDOWS];
  uint32_t thres;
  uint32_t var_shift;
};

enum HAL_ISP_AWB_MEASURING_MODE_e {
  HAL_ISP_AWB_MEASURING_MODE_YCBCR      = 1,      /**< near white discrimination mode using YCbCr color space */
  HAL_ISP_AWB_MEASURING_MODE_RGB        = 2,      /**< RGB based measurement mode */
} ;

struct HAL_ISP_awb_meas_cfg_s {
  HAL_ISP_AWB_MEASURING_MODE_e mode;
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
  HAL_Window  win;
  /* only used for manual mode*/
  int illuIndex;
};

struct HAL_ISP_dpcc_cfg_s {
  uint32_t isp_dpcc_mode;
  uint32_t isp_dpcc_output_mode;
  uint32_t isp_dpcc_set_use;

  uint32_t isp_dpcc_methods_set_1;
  uint32_t isp_dpcc_methods_set_2;
  uint32_t isp_dpcc_methods_set_3;

  uint32_t isp_dpcc_line_thresh_1;
  uint32_t isp_dpcc_line_mad_fac_1;
  uint32_t isp_dpcc_pg_fac_1;
  uint32_t isp_dpcc_rnd_thresh_1;
  uint32_t isp_dpcc_rg_fac_1;

  uint32_t isp_dpcc_line_thresh_2;
  uint32_t isp_dpcc_line_mad_fac_2;
  uint32_t isp_dpcc_pg_fac_2;
  uint32_t isp_dpcc_rnd_thresh_2;
  uint32_t isp_dpcc_rg_fac_2;

  uint32_t isp_dpcc_line_thresh_3;
  uint32_t isp_dpcc_line_mad_fac_3;
  uint32_t isp_dpcc_pg_fac_3;
  uint32_t isp_dpcc_rnd_thresh_3;
  uint32_t isp_dpcc_rg_fac_3;

  uint32_t isp_dpcc_ro_limits;
  uint32_t isp_dpcc_rnd_offs;
};

enum  HAL_ISP_HIST_MODE_e {
  HAL_ISP_HIST_MODE_RGB_COMBINED  = 1,    /**< RGB combined histogram */
  HAL_ISP_HIST_MODE_R             = 2,    /**< R histogram */
  HAL_ISP_HIST_MODE_G             = 3,    /**< G histogram */
  HAL_ISP_HIST_MODE_B             = 4,    /**< B histogram */
  HAL_ISP_HIST_MODE_Y             = 5,    /**< luminance histogram */
} ;

struct HAL_ISP_hst_cfg_s {
  HAL_ISP_HIST_MODE_e mode;
  HAL_Window         win;
  /*range of every weight : 0 to 16 */
  uint8_t     weight[HAL_ISP_HIST_GRID_ITEMS];
};

/*TODOS*/
struct HAL_ISP_dpf_cfg_s {

};

struct HAL_ISP_dpf_strength_cfg_s {
  float r;
  float g;
  float b;
};

struct HAL_ISP_wdr_cfg_s {
  bool_t wdr_enable;
  HAL_ISP_WDR_MODE_e mode;
  uint8_t  dx_used_cnt;
  union {
    uint16_t wdr_block_dy[HAL_ISP_WDR_SECTION_MAX + 1];
    uint16_t wdr_global_dy[HAL_ISP_WDR_SECTION_MAX + 1];
  } wdr_dy;
  /*
   range:  8(2^3) to  (2^10),should be multiply by 2.
  */
  uint16_t wdr_dx[HAL_ISP_WDR_SECTION_MAX];
  uint16_t wdr_noiseratio;
  uint16_t wdr_bestlight;
  uint32_t wdr_gain_off1;
  uint16_t wdr_pym_cc;
  uint8_t wdr_epsilon;
  uint8_t wdr_lvl_en;
  bool_t wdr_flt_sel;
  bool_t wdr_gain_max_clip_enable;
  uint8_t wdr_gain_max_value;
  uint8_t wdr_bavg_clip;
  bool_t wdr_nonl_segm;
  bool_t wdr_nonl_open;
  bool_t wdr_nonl_mode1;
  uint32_t wdr_coe0;
  uint32_t wdr_coe1;
  uint32_t wdr_coe2;
  uint32_t wdr_coe_off;
};

struct HAL_ISP_bdm_cfg_s {
  unsigned char demosaic_th;
};

struct HAL_ISP_dsp3dnr_cfg_ex_s {
  unsigned int noise_level;
  unsigned char full_range_flag;//[1]
  unsigned char blk16_sad_th_ratio;
  char blk16_shf_bit_1;
  char blk32_shf_bit_1;
  char blk128_shf_bit_1;//[2]
  char blk16_shf_bit_0;
  char blk32_shf_bit_0;
  char blk128_shf_bit_0;
  char night_flag;//[3]
  char blank[3];
};
struct HAL_ISP_DSP3DNR_cfg_s {
  unsigned char luma_sp_nr_en;
  unsigned char luma_te_nr_en;
  unsigned char chrm_sp_nr_en;
  unsigned char chrm_te_nr_en;
  unsigned char shp_en;
  unsigned char luma_sp_nr_level;         // control the strength of spatial luma denoise
  unsigned char luma_te_nr_level;         // control the strength of temporal luma denoise
  unsigned char chrm_sp_nr_level;         // control the strength of spatial luma denoise
  unsigned char chrm_te_nr_level;         // control the strength of spatial luma denoise
  unsigned char shp_level;           // control sharpness strenth
  
  //5x5 luma spatial weight table,8bit for the center point,6bit for the other point,
  //low 30bit is useful in w0 w1 w3 w4,6 6 8 6 6 in w2,all these weight are int type.
  unsigned char luma_default;      // 1 use level define,0 use those parameters below
  unsigned char luma_sp_rad;      //spatial bilateral filter size
  unsigned char luma_te_max_bi_num;      //temporal max bilateral frame num
  uint32_t luma_w0;            //
  uint32_t luma_w1;
  uint32_t luma_w2;
  uint32_t luma_w3;
  uint32_t luma_w4;
  
  //5x5 chroma spatial weight table,8bit for the center point,6bit for the other point,
  //low 30bit is useful in w0 w1 w3 w4,6 6 8 6 6 in w2,all these weight are unsigned int type.
  unsigned char chrm_default;      // 1 use level define,0 use those parameters below
  unsigned char chrm_sp_rad;      //chroma spatial bilateral filter size
  unsigned char chrm_te_max_bi_num;      //temporal max bilateral frame num
  uint32_t chrm_w0;            //
  uint32_t chrm_w1;
  uint32_t chrm_w2;
  uint32_t chrm_w3;
  uint32_t chrm_w4;
  
  unsigned char shp_default;      // 1 use level define,0 use those parameters below
  //5x5 sharpness weight table,8bit for the center point,6bit for the other point,
  //low 30bit is useful in w0 w1 w3 w4,6 6 8 6 6 in w2,all these weight are int type.
  uint32_t src_shp_w0;            //
  uint32_t src_shp_w1;
  uint32_t src_shp_w2;
  uint32_t src_shp_w3;
  uint32_t src_shp_w4;
  //threshold from 0 to 31
  unsigned char src_shp_thr;
  //shift bit of  sum of weight.
  unsigned char src_shp_div;
  //luma sharpness enable flag
  unsigned char src_shp_l;
  //chroma sharpness enable flag
  unsigned char src_shp_c;

  union {
    struct HAL_ISP_dsp3dnr_cfg_ex_s ex;
    int32_t reserves[32];
  } reserves;
};

struct HAL_3DnrLevelCfg {
  unsigned char luma_sp_nr_en;
  unsigned char luma_te_nr_en;
  unsigned char chrm_sp_nr_en;
  unsigned char chrm_te_nr_en;
  unsigned char shp_en;
  unsigned char luma_sp_nr_level;         // control the strength of spatial luma denoise
  unsigned char luma_te_nr_level;         // control the strength of temporal luma denoise
  unsigned char chrm_sp_nr_level;         // control the strength of spatial luma denoise
  unsigned char chrm_te_nr_level;         // control the strength of spatial luma denoise
  unsigned char shp_level;           // control sharpness strenth
};

struct HAL_3DnrParamCfg {
  uint16_t noise_coef_num;
  uint16_t noise_coef_den;

  //5x5 luma spatial weight table,8bit for the center point,6bit for the other point,
  //low 30bit is useful in w0 w1 w3 w4,6 6 8 6 6 in w2,all these weight are int type.
  unsigned char luma_default;      // 1 use level define,0 use those parameters below
  unsigned char luma_sp_rad;      //spatial bilateral filter size
  unsigned char luma_te_max_bi_num;      //temporal max bilateral frame num
  uint32_t luma_w0;            //
  uint32_t luma_w1;
  uint32_t luma_w2;
  uint32_t luma_w3;
  uint32_t luma_w4;

  //5x5 chroma spatial weight table,8bit for the center point,6bit for the other point,
  //low 30bit is useful in w0 w1 w3 w4,6 6 8 6 6 in w2,all these weight are unsigned int type.
  unsigned char chrm_default;      // 1 use level define,0 use those parameters below
  unsigned char chrm_sp_rad;      //chroma spatial bilateral filter size
  unsigned char chrm_te_max_bi_num;      //temporal max bilateral frame num
  uint32_t chrm_w0;            //
  uint32_t chrm_w1;
  uint32_t chrm_w2;
  uint32_t chrm_w3;
  uint32_t chrm_w4;

  unsigned char shp_default;      // 1 use level define,0 use those parameters below
  //5x5 sharpness weight table,8bit for the center point,6bit for the other point,
  //low 30bit is useful in w0 w1 w3 w4,6 6 8 6 6 in w2,all these weight are int type.
  uint32_t src_shp_w0;            //
  uint32_t src_shp_w1;
  uint32_t src_shp_w2;
  uint32_t src_shp_w3;
  uint32_t src_shp_w4;
  //threshold from 0 to 31
  unsigned char src_shp_thr;
  //shift bit of  sum of weight.
  unsigned char src_shp_div;
  //luma sharpness enable flag
  unsigned char src_shp_l;
  //chroma sharpness enable flag
  unsigned char src_shp_c;
};

struct HAL_3DnrCfg{
  unsigned char Enable;
  struct HAL_3DnrLevelCfg level_cfg;
  struct HAL_3DnrParamCfg param_cfg;
};

struct HAL_New3Dnr_ynr_params_s {
	uint32_t enable_ynr;        // Set to 1 by default
	uint32_t enable_tnr;        // Set to 1 by default, it will be disabled when enable_ynr=0
	uint32_t enable_iir;        // Set to 0 by default, it will be disabled when enable_ynr=0
	uint32_t ynr_time_weight;        // Denoise weight of time, valid range: 1 - 4, default 3
	uint32_t ynr_spat_weight;        // Denoise weight of spatial, valid range: 0 - 28, default 16
	uint32_t reserved[4];
};

struct HAL_New3Dnr_uvnr_params_s {
	uint32_t enable_uvnr;       // Set to 1 by default
	uint32_t uvnr_weight;       // Denoise weight for uvnr, valid range: 4 - 16, default 12
	uint32_t reserved[4];
};

struct HAL_New3Dnr_sharp_params_s {
	uint32_t enable_sharp;      // Set to 1 by default, enable DSP sharpness algorithm
	uint32_t sharp_weight;      // Sharpness weight, valid range: 0 - 4, defalut 2
	uint32_t reserved[4];
};

struct HAL_New3DnrCfg_s {
	uint32_t enable_3dnr;
	
	struct HAL_New3Dnr_ynr_params_s ynr;
	struct HAL_New3Dnr_uvnr_params_s uvnr;
	struct HAL_New3Dnr_sharp_params_s sharp;
	
	uint32_t enable_dpc;        // Set to 1 by default, enable DSP dpc algorithm
	uint32_t reserved[4];
};


enum HAL_FLT_DENOISE_LEVEL_e {
  HAL_FLT_DENOISE_LEVEL_0,
  HAL_FLT_DENOISE_LEVEL_1,
  HAL_FLT_DENOISE_LEVEL_2,
  HAL_FLT_DENOISE_LEVEL_3,
  HAL_FLT_DENOISE_LEVEL_4,
  HAL_FLT_DENOISE_LEVEL_5,
  HAL_FLT_DENOISE_LEVEL_6,
  HAL_FLT_DENOISE_LEVEL_7,
  HAL_FLT_DENOISE_LEVEL_8,
  HAL_FLT_DENOISE_LEVEL_9,
  HAL_FLT_DENOISE_LEVEL_10
};

enum HAL_FLT_SHARPENING_LEVEL_e {
  HAL_FLT_SHARPENING_LEVEL_0,
  HAL_FLT_SHARPENING_LEVEL_1,
  HAL_FLT_SHARPENING_LEVEL_2,
  HAL_FLT_SHARPENING_LEVEL_3,
  HAL_FLT_SHARPENING_LEVEL_4,
  HAL_FLT_SHARPENING_LEVEL_5,
  HAL_FLT_SHARPENING_LEVEL_6,
  HAL_FLT_SHARPENING_LEVEL_7,
  HAL_FLT_SHARPENING_LEVEL_8,
  HAL_FLT_SHARPENING_LEVEL_9,
  HAL_FLT_SHARPENING_LEVEL_10
};

struct HAL_Buffer_MetaData {
  struct HAL_ISP_awb_cfg_s awb;
  struct HAL_ISP_flt_cfg_s flt;
  struct HAL_ISP_wdr_cfg_s wdr;
  struct HAL_ISP_dpf_strength_cfg_s dpf_strength;
  struct HAL_ISP_dpf_cfg_s dpf;
  struct HAL_ISP_DSP3DNR_cfg_s dsp_3DNR;
  struct HAL_New3DnrCfg_s newDsp3DNR;
  struct HAL_ISP_ctk_cfg_s ctk;
  struct HAL_ISP_lsc_cfg_s lsc;
  struct HAL_ISP_goc_cfg_s goc;
  struct timeval timStamp;
  float exp_time;
  float exp_gain;
  float MeanLuma;
  float DON_Fac;
  uint8_t LightMode;
  float overHistPercent;
  bool_t enabled[HAL_ISP_MODULE_MAX_ID_ID + 1];
  void* metedata_drv;
  float maxGainRange;
};

enum HAL_ISP_ACTIVE_MODE {
  /*disable this module*/
  HAL_ISP_ACTIVE_FALSE,
  /*use user settings*/
  HAL_ISP_ACTIVE_SETTING,
  /*use default settings*/
  HAL_ISP_ACTIVE_DEFAULT,
  /*use calibration settings*/
  //HAL_ISP_ACTIVE_CALIB
};

#define HAL_ISP_LSC_NAME_LEN         25
#define HAL_ISP_LSC_SIZE_TBL_LEN     8
#define HAL_ISP_LSC_MATRIX_COLOR_NUM 4
#define HAL_ISP_LSC_MATRIX_TBL_LEN   289

struct HAL_ISP_Lsc_Profile_s {
  int8_t    LscName[HAL_ISP_LSC_NAME_LEN];

  uint16_t  LscSectors;
  uint16_t  LscNo;
  uint16_t  LscXo;
  uint16_t  LscYo;

  uint16_t  LscXSizeTbl[HAL_ISP_LSC_SIZE_TBL_LEN];
  uint16_t  LscYSizeTbl[HAL_ISP_LSC_SIZE_TBL_LEN];

  uint16_t  LscMatrix[HAL_ISP_LSC_MATRIX_COLOR_NUM][HAL_ISP_LSC_MATRIX_TBL_LEN];
};

struct HAL_ISP_Lsc_Query_s {
  int8_t    LscNameUp[HAL_ISP_LSC_NAME_LEN];
  int8_t    LscNameDn[HAL_ISP_LSC_NAME_LEN];
};

#define HAL_ISP_ILL_NAME_LEN    20
struct HAL_ISP_AWB_CCM_SET_s {
  int8_t ill_name[HAL_ISP_ILL_NAME_LEN];
  float coeff00;
  float coeff01;
  float coeff02;
  float coeff10;
  float coeff11;
  float coeff12;
  float coeff20;
  float coeff21;
  float coeff22;
  float ct_offset_r;
  float ct_offset_g;
  float ct_offset_b;
};

struct HAL_ISP_AWB_CCM_GET_s {
  int8_t name_up[HAL_ISP_ILL_NAME_LEN];
  int8_t name_dn[HAL_ISP_ILL_NAME_LEN];
  float coeff00;
  float coeff01;
  float coeff02;
  float coeff10;
  float coeff11;
  float coeff12;
  float coeff20;
  float coeff21;
  float coeff22;
  float ct_offset_r;
  float ct_offset_g;
  float ct_offset_b;
};

struct HAL_ISP_AWB_s {
  float r_gain;
  float gr_gain;
  float gb_gain;
  float b_gain;
  uint8_t lock_ill;
  uint8_t ill_name[HAL_ISP_ILL_NAME_LEN];
};

#define HAL_ISP_AWBFADE2PARM_LEN  6
struct HAL_ISP_AWB_White_Point_Set_s {
  uint16_t win_h_offs;
  uint16_t win_v_offs;
  uint16_t win_width;
  uint16_t win_height;
  uint8_t awb_mode;
  float afFade[HAL_ISP_AWBFADE2PARM_LEN];
  float afmaxCSum_br[HAL_ISP_AWBFADE2PARM_LEN];
  float afmaxCSum_sr[HAL_ISP_AWBFADE2PARM_LEN];
  float afminC_br[HAL_ISP_AWBFADE2PARM_LEN];
  float afMaxY_br[HAL_ISP_AWBFADE2PARM_LEN];
  float afMinY_br[HAL_ISP_AWBFADE2PARM_LEN];
  float afminC_sr[HAL_ISP_AWBFADE2PARM_LEN];
  float afMaxY_sr[HAL_ISP_AWBFADE2PARM_LEN];
  float afMinY_sr[HAL_ISP_AWBFADE2PARM_LEN];
  float afRefCb[HAL_ISP_AWBFADE2PARM_LEN];
  float afRefCr[HAL_ISP_AWBFADE2PARM_LEN];

  float fRgProjIndoorMin;
  float fRgProjOutdoorMin;
  float fRgProjMax;
  float fRgProjMaxSky;
  float fRgProjALimit;
  float fRgProjAWeight;
  float fRgProjYellowLimitEnable;
  float fRgProjYellowLimit;
  float fRgProjIllToCwfEnable;
  float fRgProjIllToCwf;
  float fRgProjIllToCwfWeight;
  float fRegionSize;
  float fRegionSizeInc;
  float fRegionSizeDec;

  uint32_t cnt;
  uint8_t mean_y;
  uint8_t mean_cb;
  uint8_t mean_cr;
  uint16_t mean_r;
  uint16_t mean_b;
  uint16_t mean_g;
};


struct HAL_ISP_AWB_White_Point_Get_s {
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
};


#define HAL_ISP_CURVE_NAME_LEN    20
#define HAL_ISP_AWBCLIPPARM_LEN   16
struct HAL_ISP_AWB_Curve_s {
  float f_N0_Rg;
  float f_N0_Bg;
  float f_d;
  float Kfactor;

  float afRg1[HAL_ISP_AWBCLIPPARM_LEN];
  float afMaxDist1[HAL_ISP_AWBCLIPPARM_LEN];
  float afRg2[HAL_ISP_AWBCLIPPARM_LEN];
  float afMaxDist2[HAL_ISP_AWBCLIPPARM_LEN];
  float afGlobalFade1[HAL_ISP_AWBCLIPPARM_LEN];
  float afGlobalGainDistance1[HAL_ISP_AWBCLIPPARM_LEN];
  float afGlobalFade2[HAL_ISP_AWBCLIPPARM_LEN];
  float afGlobalGainDistance2[HAL_ISP_AWBCLIPPARM_LEN];
};

struct HAL_ISP_AWB_RefGain_s {
  int8_t ill_name[HAL_ISP_ILL_NAME_LEN];
  float refRGain;
  float refGrGain;
  float refGbGain;
  float refBGain;
};

#define HAL_ISP_GOC_SCENE_NAME_LEN    20
#define HAL_ISP_GOC_GAMMA_NUM    34

enum HAL_ISP_GOC_WDR_STATUS {
  HAL_ISP_GOC_NORMAL,
  HAL_ISP_GOC_WDR_ON
};

enum HAL_ISP_GOC_CFG_MODE {
  HAL_ISP_GOC_CFG_MODE_LOGARITHMIC = 1,
  HAL_ISP_GOC_CFG_MODE_EQUIDISTANT
};

struct HAL_ISP_GOC_s {
  int8_t scene_name[HAL_ISP_GOC_SCENE_NAME_LEN];
  enum HAL_ISP_GOC_WDR_STATUS wdr_status;
  enum HAL_ISP_GOC_CFG_MODE cfg_mode;
  uint16_t gamma_y[HAL_ISP_GOC_GAMMA_NUM];
};

enum HAL_ISP_CPROC_MODE {
  HAL_ISP_CPROC_PREVIEW,
  HAL_ISP_CPROC_CAPTURE,
  HAL_ISP_CPROC_VIDEO
};

struct HAL_ISP_CPROC_s {
  enum HAL_ISP_CPROC_MODE mode;
  float cproc_contrast;
  float cproc_hue;
  float cproc_saturation;
  int8_t cproc_brightness;
};


#define HAL_ISP_ADPF_DPF_NAME_LEN  20
#define HAL_ISP_ADPF_DPF_NLL_COEFF_LEN  17
struct HAL_ISP_ADPF_DPF_s {
  int8_t  dpf_name[HAL_ISP_ADPF_DPF_NAME_LEN];
  uint8_t dpf_enable;
  uint8_t nll_segment;
  uint16_t nll_coeff[HAL_ISP_ADPF_DPF_NLL_COEFF_LEN];
  uint16_t sigma_green;
  uint16_t sigma_redblue;
  float gradient;
  float offset;
  float fRed;
  float fGreenR;
  float fGreenB;
  float fBlue;
};

#define HAL_ISP_FLT_CURVE_NUM 5
#define HAL_ISP_FLT_NAME_LEN  20
struct HAL_ISP_FLT_Denoise_Curve_s {
  uint8_t denoise_gain[HAL_ISP_FLT_CURVE_NUM];
  uint8_t denoise_level[HAL_ISP_FLT_CURVE_NUM];
};

struct HAL_ISP_FLT_Sharp_Curve_s {
  uint8_t sharp_gain[HAL_ISP_FLT_CURVE_NUM];
  uint8_t sharp_level[HAL_ISP_FLT_CURVE_NUM];
};

struct HAL_ISP_FLT_Level_Conf_s {
  uint8_t grn_stage1;
  uint8_t chr_h_mode;
  uint8_t chr_v_mode;
  uint32_t thresh_bl0;
  uint32_t thresh_bl1;
  uint32_t thresh_sh0;
  uint32_t thresh_sh1;
  uint32_t fac_sh1;
  uint32_t fac_sh0;
  uint32_t fac_mid;
  uint32_t fac_bl0;
  uint32_t fac_bl1;
};

struct HAL_ISP_FLT_Set_s {
  int8_t  filter_name[HAL_ISP_FLT_NAME_LEN];
  uint8_t scene_mode;
  uint8_t filter_enable;
  struct HAL_ISP_FLT_Denoise_Curve_s denoise;
  struct HAL_ISP_FLT_Sharp_Curve_s sharp;
  uint8_t level_conf_enable;
  uint8_t level;
  struct HAL_ISP_FLT_Level_Conf_s level_conf;
};

struct HAL_ISP_FLT_Get_s {
  int8_t  filter_name[HAL_ISP_FLT_NAME_LEN];
  uint8_t filter_enable;
  struct HAL_ISP_FLT_Denoise_Curve_s denoise;
  struct HAL_ISP_FLT_Sharp_Curve_s sharp;
  uint8_t level_conf_enable;
  uint8_t is_level_exit;
  struct HAL_ISP_FLT_Level_Conf_s level_conf;
};

struct HAL_ISP_cfg_s {
  /* black level subtraction */
  struct HAL_ISP_bls_cfg_s* bls_cfg;
  /* sensor degamma */
  struct HAL_ISP_sdg_cfg_s* sdg_cfg;
  /* filter */
  struct HAL_ISP_flt_cfg_s* flt_cfg;
  /* gamma out correction*/
  struct HAL_ISP_goc_cfg_s* goc_cfg;
  /*color process*/
  struct HAL_ISP_cproc_cfg_s* cproc_cfg;
  /* image effect */
  struct HAL_ISP_ie_cfg_s* ie_cfg;
  /* lens shading correct */
  struct HAL_ISP_lsc_cfg_s* lsc_cfg;
  /* white balance gain */
  struct HAL_ISP_awb_gain_cfg_s* awb_gain_cfg;
  /* cross talk(color correction matrix)*/
  struct HAL_ISP_ctk_cfg_s* ctk_cfg;
  /* exposure measure*/
  struct HAL_ISP_aec_cfg_s* aec_cfg;
  /* denoise pre filter*/
  struct HAL_ISP_dpf_cfg_s* dpf_cfg;
  struct HAL_ISP_dpf_strength_cfg_s* dpf_strength_cfg;
  /* auto focus control measure*/
  struct HAL_ISP_afc_cfg_s* afc_cfg;
  /* white balance measure*/
  struct HAL_ISP_awb_meas_cfg_s* awb_cfg;
  /* wide dynamic range*/
  struct HAL_ISP_wdr_cfg_s* wdr_cfg;
  /* dead pixel correction */
  struct HAL_ISP_dpcc_cfg_s* dpcc_cfg;
  /* histogram measure*/
  struct HAL_ISP_hst_cfg_s* hst_cfg;
  /* bayer demosaic*/
  struct HAL_ISP_bdm_cfg_s* bdm_cfg;
  /* 3dnr */
  struct HAL_3DnrCfg* dsp_3dnr_cfg;
  /*new 3dnr */
  struct HAL_New3DnrCfg_s* newDsp3DNR_cfg;
  /* awb lsc profile */
  struct HAL_ISP_Lsc_Profile_s* awb_lsc_pfl;
  /* adpf dpf */
  struct HAL_ISP_ADPF_DPF_s* adpf_dpf;
  /* adpf flt */
  struct HAL_ISP_FLT_Set_s* adpf_flt;
  /* awb ccm profile */
  struct HAL_ISP_AWB_CCM_SET_s* awb_ccm_pfl;
  /* awb gain illu */
  struct HAL_ISP_AWB_s* awb_gain_illu_pfl;
  /* awb refgain */
  struct HAL_ISP_AWB_RefGain_s* awb_refgain;
  /* awb curve */
  struct HAL_ISP_AWB_Curve_s* awb_curve;
  /* awb wp */
  struct HAL_ISP_AWB_White_Point_Set_s* awb_wp_set;
  /* calibdb goc */
  struct HAL_ISP_GOC_s* calibdb_goc_cfg;
  /* calibdb cproc */
  struct HAL_ISP_CPROC_s* calibdb_cproc_cfg;
  /* need updated sub modules*/
  uint32_t updated_mask;
  /* enabled */
  enum HAL_ISP_ACTIVE_MODE enabled[HAL_ISP_MODULE_MAX_ID_ID + 1];
};


#define HAL_ISP_STORE_PATH_LEN         32

enum HAL_ISP_CAP_FORMAT {
  HAL_ISP_FMT_YUV420 = 0x18,
  HAL_ISP_FMT_YUV422 = 0x1E,
  HAL_ISP_FMT_RAW10 = 0x2B,
  HAL_ISP_FMT_RAW12 = 0x2C
};

enum HAL_ISP_CAP_RESULT {
  HAL_ISP_CAP_FINISH,
  HAL_ISP_CAP_RUNNING
};

enum HAL_ISP_AE_MODE {
  HAL_ISP_AE_MANUAL,
  HAL_ISP_AE_AUTO
};

struct HAL_ISP_Cap_Req_s {
  uint8_t cap_id;
  int8_t  store_path[HAL_ISP_STORE_PATH_LEN];
  enum HAL_ISP_CAP_FORMAT cap_format;
  uint8_t cap_num;
  uint16_t cap_height;
  uint16_t cap_width;
  enum HAL_ISP_AE_MODE ae_mode;
  uint8_t exp_time_h;
  uint8_t exp_time_l;
  uint8_t exp_gain_h;
  uint8_t exp_gain_l;
  uint16_t af_code;
};

struct HAL_ISP_Cap_Result_s {
  uint8_t cap_id;
  enum HAL_ISP_CAP_RESULT result;
};


#define HAL_ISP_SYS_INFO_LEN         32
#define HAL_ISP_SENSOR_RESOLUTION_NUM         4

struct HAL_ISP_Sensor_Reso_s {
  uint16_t width;
  uint16_t height;
};

struct HAL_ISP_OTP_Info_s {
  uint8_t awb_otp:1;
  uint8_t lsc_otp:1;
};

struct HAL_ISP_Sys_Info_s {
  uint8_t major_ver;
  uint8_t minor_ver;
  int8_t  platform[HAL_ISP_SYS_INFO_LEN];
  int8_t  sensor[HAL_ISP_SYS_INFO_LEN];
  int8_t  module[HAL_ISP_SYS_INFO_LEN];
  int8_t  lens[HAL_ISP_SYS_INFO_LEN];
  struct HAL_ISP_OTP_Info_s otp_info;
  uint8_t max_exp_time_h;
  uint8_t max_exp_time_l;
  uint8_t max_exp_gain_h;
  uint8_t max_exp_gain_l;
  uint8_t reso_num;
  struct HAL_ISP_Sensor_Reso_s reso[HAL_ISP_SENSOR_RESOLUTION_NUM];
  uint8_t sensor_fmt;
};

struct HAL_ISP_Sensor_Mirror_s {
  uint8_t horizontal_mirror:1;
  uint8_t vertical_mirror:1;
};

struct HAL_ISP_Sensor_Info_s {
  uint8_t exp_time_h;
  uint8_t exp_time_l;
  uint8_t exp_gain_h;
  uint8_t exp_gain_l;
  struct HAL_ISP_Sensor_Mirror_s mirror_info;
  uint16_t frame_length_lines;
  uint16_t line_length_pck;
  uint32_t vt_pix_clk_freq_hz;
  uint8_t binning;
};

struct HAL_ISP_Sensor_Exposure_s {
  enum HAL_ISP_AE_MODE ae_mode;
  uint8_t exp_time_h;
  uint8_t exp_time_l;
  uint8_t exp_gain_h;
  uint8_t exp_gain_l;
};

struct HAL_ISP_Sensor_Reg_s {
  uint8_t reg_addr_len;
  uint16_t reg_addr;
  uint8_t reg_data_len;
  uint16_t reg_data;
};

#define HAL_ISP_IQ_PATH_LEN    32
struct HAL_ISP_Reboot_Req_s {
  uint8_t reboot;
  int8_t  iq_path[HAL_ISP_IQ_PATH_LEN];
};

#endif
