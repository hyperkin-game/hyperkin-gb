#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include "CamIsp1xCtrItf.h"
#include "cam_ia_api/cam_ia10_trace.h"

using namespace std;

#define CAMERA_ISP_DEV_NAME   "/dev/video1"
#define TUNNING_FILE_PATH  "/data/tunning.xml"

#if 0
static bool HAL_AE_FLK_MODE_to_AecEcmFlickerPeriod_t(
    HAL_AE_FLK_MODE in, AecEcmFlickerPeriod_t* out) {
  switch (in) {
    case HAL_AE_FLK_OFF:
      *out = AEC_EXPOSURE_CONVERSION_FLICKER_OFF;
      return true;
    case HAL_AE_FLK_50:
      *out = AEC_EXPOSURE_CONVERSION_FLICKER_100HZ;
      return true;
    case HAL_AE_FLK_60:
      *out = AEC_EXPOSURE_CONVERSION_FLICKER_120HZ;
      return true;
    default:
      return false;
  }
}

#endif

CamIsp1xCtrItf::CamIsp1xCtrItf(CamHwItf* camHwItf, int devFd):
  mInitialized(0) {
  //ALOGD("%s: E", __func__);

  mStartCnt = 0;
  mStreaming = false;
  mIspFd = -1;
  mLenPos = -1;
  m3AMeasType = HAL_3A_MEAS_INVALID;
  mFrmStartTime = 0;
  mFrmEndTime = 0;
  memset(&mCamIA_DyCfg, 0x00, sizeof(struct CamIA10_DyCfg));
  mCamIA_DyCfg.len_pos = -1;
  mAeStableCnt = 0;
  osMutexInit(&mApiLock);
  mCamHwItf = camHwItf;
  mDevFd = devFd;
  //ALOGD("%s: x", __func__);
}

CamIsp1xCtrItf::~CamIsp1xCtrItf() {
  //ALOGD("%s: E", __func__);

  deInit();
  osMutexDestroy(&mApiLock);

  //ALOGD("%s: x", __func__);
}

bool CamIsp1xCtrItf::init(const char* tuningFile,
                          const char* ispDev, enum CAMISP_CTRL_MODE ctrl_mode) {
  return true;
}

bool CamIsp1xCtrItf::deInit() {
  //ALOGD("%s: E", __func__);

  osMutexLock(&mApiLock);
  if (mIspFd >= 0) {
    int ret;
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (mStreaming) {
      ret = ioctl(mIspFd, VIDIOC_STREAMOFF, &type);
      if (ret < 0) {
        ALOGE("%s: Failed to stop stream", __func__);
      }
    }

    for (int i = 0; i < CAM_ISP_NUM_OF_STAT_BUFS; i++) {
      if (mIspStatBuf[i]) {
        munmap(mIspStatBuf[i], mIspStatBufSize);
        mIspStatBuf[i] = NULL;
      }
    }

    if (mIspFd >= 0)
      close(mIspFd);

    mIspStatBufSize = 0;
    mIspFd = -1;
    //mInitialized = false;
  }
  mCamIAEngine.reset();
  osMutexUnlock(&mApiLock);

  //ALOGD("%s: x", __func__);
  return true;

}

bool CamIsp1xCtrItf::configure(const Configuration& config) {
  bool ret = true;
  bool wdr_curve_chg = false;

  osMutexLock(&mApiLock);
  //HAL_AE_FLK_MODE_to_AecEcmFlickerPeriod_t(
  //  config.aec_cfg.flk,
  //  &mCamIA_DyCfg.aec.flicker);
  mCamIA_DyCfg.aec_cfg = config.aec_cfg;
  if ((mCamIA_DyCfg.aec_cfg.win.right_width == 0) ||
      (mCamIA_DyCfg.aec_cfg.win.bottom_height == 0)) {
    mCamIA_DyCfg.aec_cfg.win.right_width = 2048;
    mCamIA_DyCfg.aec_cfg.win.bottom_height = 2048;
    mCamIA_DyCfg.aec_cfg.win.left_hoff = 0;
    mCamIA_DyCfg.aec_cfg.win.top_voff = 0;
  }

  mCamIA_DyCfg.afc_cfg = config.afc_cfg;
  if (mCamIA_DyCfg.afc_cfg.mode != HAL_AF_MODE_NOT_SET) {
    if ((mCamIA_DyCfg.afc_cfg.win_a.right_width == 0) ||
        (mCamIA_DyCfg.afc_cfg.win_a.bottom_height == 0)) {
      int width, height, x, y;
      int outWidth, outHeight;

      outWidth = 2048;
      outHeight = 2048;
      width  = outWidth * 0.15;
      height = outHeight * 0.15;
      x = ( outWidth  >> 1 ) - ( width  >> 1 );
      y = ( outHeight >> 1 ) - ( height >> 1 );

      mCamIA_DyCfg.afc_cfg.win_a.left_hoff = x;
      mCamIA_DyCfg.afc_cfg.win_a.top_voff = y;
      mCamIA_DyCfg.afc_cfg.win_a.right_width = x + width;
      mCamIA_DyCfg.afc_cfg.win_a.bottom_height = y + height;
      mCamIA_DyCfg.afc_cfg.win_num = 1;
      //mCamIA_DyCfg.afc_cfg.mode == HAL_AF_MODE_CONTINUOUS_VIDEO;

      ALOGD("%s: af default win %d, %d, %d, %d", __func__, x, y, width, height);
    }
  }

  mCamIA_DyCfg.aaa_locks = config.aaa_locks;
  mCamIA_DyCfg.awb_cfg = config.awb_cfg;
  mCamIA_DyCfg.flash_mode = config.flash_mode;
  mCamIA_DyCfg.uc = config.uc;

  mCamIA_DyCfg.sensor_mode.isp_input_width =
      config.sensor_mode.isp_input_width;
  mCamIA_DyCfg.sensor_mode.isp_input_height =
      config.sensor_mode.isp_input_height;
  mCamIA_DyCfg.sensor_mode.isp_output_width =
      config.sensor_mode.isp_output_width;
  mCamIA_DyCfg.sensor_mode.isp_output_height =
      config.sensor_mode.isp_output_height;
  mCamIA_DyCfg.sensor_mode.horizontal_crop_offset =
      config.sensor_mode.horizontal_crop_offset;
  mCamIA_DyCfg.sensor_mode.vertical_crop_offset =
      config.sensor_mode.vertical_crop_offset;
  mCamIA_DyCfg.sensor_mode.cropped_image_width =
      config.sensor_mode.cropped_image_width;
  mCamIA_DyCfg.sensor_mode.cropped_image_height =
      config.sensor_mode.cropped_image_height;
  mCamIA_DyCfg.sensor_mode.pixel_clock_freq_mhz =
      config.sensor_mode.pixel_clock_freq_mhz;
  mCamIA_DyCfg.sensor_mode.pixel_periods_per_line =
      config.sensor_mode.pixel_periods_per_line;
  mCamIA_DyCfg.sensor_mode.line_periods_per_field =
      config.sensor_mode.line_periods_per_field;
  mCamIA_DyCfg.sensor_mode.sensor_output_height =
      config.sensor_mode.sensor_output_height;
  mCamIA_DyCfg.sensor_mode.fine_integration_time_min =
      config.sensor_mode.fine_integration_time_min;
  mCamIA_DyCfg.sensor_mode.fine_integration_time_max_margin =
      config.sensor_mode.fine_integration_time_max_margin;
  mCamIA_DyCfg.sensor_mode.coarse_integration_time_min =
      config.sensor_mode.coarse_integration_time_min;
  mCamIA_DyCfg.sensor_mode.coarse_integration_time_max_margin =
      config.sensor_mode.coarse_integration_time_max_margin;

  osMutexUnlock(&mApiLock);

  //cproc
  osMutexLock(&mApiLock);
  if (((config.cproc.brightness == -120) ||
      (config.cproc.contrast == -200.0f) ||
      (config.cproc.hue == -200.0f) ||
      (config.cproc.saturation == -200.0f)) &&
      (memcmp(&config.cproc, &mCamIA_DyCfg.cproc, sizeof(config.cproc)))) {
	struct HAL_ISP_cfg_s cfg ;
	cfg.updated_mask = 0;
	cfg.cproc_cfg = NULL;
	cfg.updated_mask |= HAL_ISP_CPROC_MASK;
	cfg.enabled[HAL_ISP_CPROC_ID] = HAL_ISP_ACTIVE_DEFAULT;
	mCamIA_DyCfg.cproc = config.cproc;
	osMutexUnlock(&mApiLock);
	configureISP(&cfg);
  } else if ((config.cproc.contrast != mCamIA_DyCfg.cproc.contrast)
      || (config.cproc.hue != mCamIA_DyCfg.cproc.hue)
      || (config.cproc.brightness != mCamIA_DyCfg.cproc.brightness)
      || (config.cproc.saturation != mCamIA_DyCfg.cproc.saturation)
      || (config.cproc.sharpness != mCamIA_DyCfg.cproc.sharpness)) {
    struct HAL_ISP_cfg_s cfg ;
    struct HAL_ISP_cproc_cfg_s cproc_cfg;
    cproc_cfg.range = HAL_ISP_COLOR_RANGE_OUT_FULL_RANGE;
    cfg.updated_mask = 0;
    cproc_cfg.cproc.contrast = config.cproc.contrast;
    cproc_cfg.cproc.hue = config.cproc.hue;
    cproc_cfg.cproc.brightness = config.cproc.brightness;
    cproc_cfg.cproc.saturation = config.cproc.saturation;
    cproc_cfg.cproc.sharpness = config.cproc.sharpness;
    cfg.cproc_cfg = &cproc_cfg;
    cfg.updated_mask |= HAL_ISP_CPROC_MASK;
    cfg.enabled[HAL_ISP_CPROC_ID] = HAL_ISP_ACTIVE_SETTING;
    if ((fabs(config.cproc.contrast -  1.0f) < 0.001)
        && (fabs(config.cproc.hue -  0.0f) < 0.001)
        && (fabs(config.cproc.saturation -  1.0f) < 0.001)
        && (fabs(config.cproc.sharpness -  0.0f) < 0.001)
        && (config.cproc.brightness == 0)) 
      cfg.enabled[HAL_ISP_CPROC_ID] = HAL_ISP_ACTIVE_FALSE;
      mCamIA_DyCfg.cproc = config.cproc;
      osMutexUnlock(&mApiLock);
      configureISP(&cfg);
  } else
    osMutexUnlock(&mApiLock);

  osMutexLock(&mApiLock);
  if (config.ie_mode != mCamIA_DyCfg.ie_mode) {
    struct HAL_ISP_cfg_s cfg;
    struct HAL_ISP_ie_cfg_s ie_cfg;
    cfg.updated_mask = 0;
    cfg.ie_cfg = &ie_cfg;
    ie_cfg.range = HAL_ISP_COLOR_RANGE_OUT_FULL_RANGE;
    ie_cfg.mode = config.ie_mode;
    cfg.updated_mask |= HAL_ISP_IE_MASK;
    cfg.enabled[HAL_ISP_IE_ID] = HAL_ISP_ACTIVE_SETTING;
    if (ie_cfg.mode == HAL_EFFECT_NONE)
      cfg.enabled[HAL_ISP_IE_ID] = HAL_ISP_ACTIVE_FALSE;
    mCamIA_DyCfg.ie_mode = config.ie_mode;
    osMutexUnlock(&mApiLock);
    configureISP(&cfg);
  } else
    osMutexUnlock(&mApiLock);

  osMutexLock(&mApiLock);
  for (int i = 0; i < HAL_ISP_WDR_SECTION_MAX + 1; i++) {
    if (config.wdr_cfg.curve_dy[i] != mCamIA_DyCfg.wdr_cfg.curve_dy[i]) {
      wdr_curve_chg = true;
      break;
    }
  }
  if ((config.wdr_cfg.mode != mCamIA_DyCfg.wdr_cfg.mode) ||
    (config.wdr_cfg.gain_max_clip_enable != mCamIA_DyCfg.wdr_cfg.gain_max_clip_enable) ||
    (config.wdr_cfg.gain_max_value != mCamIA_DyCfg.wdr_cfg.gain_max_value) ||
    (config.wdr_cfg.curve_enable != mCamIA_DyCfg.wdr_cfg.curve_enable) ||
    (config.wdr_cfg.curve_mode != mCamIA_DyCfg.wdr_cfg.curve_mode) ||
    (wdr_curve_chg == true)) {
    struct HAL_ISP_cfg_s cfg;
    struct HAL_ISP_wdr_cfg_s wdr_cfg;
    enum HAL_ISP_WDR_MODE_e wdr_mode;
    RESULT result;

    cfg.updated_mask = 0;
    cfg.wdr_cfg = NULL;
    cfg.updated_mask |= HAL_ISP_WDR_MASK;
    if (config.wdr_cfg.mode == HAL_MODE_OFF) {
      cfg.enabled[HAL_ISP_WDR_ID] = HAL_ISP_ACTIVE_FALSE;
    } else if (config.wdr_cfg.mode == HAL_MODE_AUTO) {
      cfg.enabled[HAL_ISP_WDR_ID] = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      cfg.enabled[HAL_ISP_WDR_ID] = HAL_ISP_ACTIVE_SETTING;

      if (mCamIAEngine.get() == NULL) {
        ALOGE("fail to get CamIAEngine");
        osMutexUnlock(&mApiLock);
        return false;
      }
      if (config.wdr_cfg.gain_max_clip_enable) {
        cfg.enabled[HAL_ISP_WDR_ID] = HAL_ISP_ACTIVE_SETTING;
        mCamIAEngine->getWdrConfig(&wdr_cfg, HAL_ISP_WDR_MODE_INVALID);
        wdr_cfg.wdr_gain_max_value = config.wdr_cfg.gain_max_value;
        cfg.wdr_cfg = &wdr_cfg;
      }
      if (config.wdr_cfg.curve_enable) {
        cfg.enabled[HAL_ISP_WDR_ID] = HAL_ISP_ACTIVE_SETTING;

        if (config.wdr_cfg.curve_mode == HAL_ISP_WDR_MODE_BLOCK) {
          mCamIAEngine->getWdrConfig(&wdr_cfg, HAL_ISP_WDR_MODE_BLOCK);
          wdr_cfg.mode = HAL_ISP_WDR_MODE_BLOCK;
          memcpy(wdr_cfg.wdr_dy.wdr_block_dy,
              config.wdr_cfg.curve_dy, sizeof(config.wdr_cfg.curve_dy));
        } else {
          mCamIAEngine->getWdrConfig(&wdr_cfg, HAL_ISP_WDR_MODE_GLOBAL);
          wdr_cfg.mode = HAL_ISP_WDR_MODE_GLOBAL;
          memcpy(wdr_cfg.wdr_dy.wdr_global_dy,
              config.wdr_cfg.curve_dy, sizeof(config.wdr_cfg.curve_dy));
        }
        cfg.wdr_cfg = &wdr_cfg;
      }
    }

    mCamIA_DyCfg.wdr_cfg = config.wdr_cfg;
    osMutexUnlock(&mApiLock);
    configureISP(&cfg);
  } else
    osMutexUnlock(&mApiLock);

  osMutexLock(&mApiLock);
  if ((config.dsp3dnr_mode != mCamIA_DyCfg.dsp3dnr_mode) ||
    (memcmp(&config.dsp3dnr_level, &mCamIA_DyCfg.dsp3dnr_level, sizeof(config.dsp3dnr_level)) != 0) ||
    (memcmp(&config.dsp3dnr_param, &mCamIA_DyCfg.dsp3dnr_param, sizeof(config.dsp3dnr_param)) != 0)) {
    struct HAL_ISP_cfg_s cfg;
    struct HAL_3DnrCfg dsp_3dnr_cfg;

    cfg.updated_mask = 0;
    cfg.dsp_3dnr_cfg = NULL;
    cfg.updated_mask |= HAL_ISP_3DNR_MASK;

    if (config.dsp3dnr_mode == HAL_MODE_OFF) {
      cfg.enabled[HAL_ISP_3DNR_ID] = HAL_ISP_ACTIVE_FALSE;
    } else if (config.dsp3dnr_mode == HAL_MODE_AUTO) {
      cfg.enabled[HAL_ISP_3DNR_ID] = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      cfg.enabled[HAL_ISP_3DNR_ID] = HAL_ISP_ACTIVE_SETTING;
      cfg.dsp_3dnr_cfg = &dsp_3dnr_cfg;

      dsp_3dnr_cfg.level_cfg.luma_sp_nr_en = config.dsp3dnr_level.luma_sp_nr_en;
      dsp_3dnr_cfg.level_cfg.luma_te_nr_en = config.dsp3dnr_level.luma_te_nr_en;
      dsp_3dnr_cfg.level_cfg.chrm_sp_nr_en = config.dsp3dnr_level.chrm_sp_nr_en;
      dsp_3dnr_cfg.level_cfg.chrm_te_nr_en = config.dsp3dnr_level.chrm_te_nr_en;
      dsp_3dnr_cfg.level_cfg.shp_en = config.dsp3dnr_level.shp_en;
      dsp_3dnr_cfg.level_cfg.luma_sp_nr_level = config.dsp3dnr_level.luma_sp_nr_level;
      dsp_3dnr_cfg.level_cfg.luma_te_nr_level = config.dsp3dnr_level.luma_te_nr_level;
      dsp_3dnr_cfg.level_cfg.chrm_sp_nr_level = config.dsp3dnr_level.chrm_sp_nr_level;
      dsp_3dnr_cfg.level_cfg.chrm_te_nr_level = config.dsp3dnr_level.chrm_te_nr_level;
      dsp_3dnr_cfg.level_cfg.shp_level = config.dsp3dnr_level.shp_level;

      dsp_3dnr_cfg.param_cfg.noise_coef_num = config.dsp3dnr_param.noise_coef_num;
      dsp_3dnr_cfg.param_cfg.noise_coef_den = config.dsp3dnr_param.noise_coef_den;
      dsp_3dnr_cfg.param_cfg.luma_default = config.dsp3dnr_param.luma_default;
      dsp_3dnr_cfg.param_cfg.luma_sp_rad = config.dsp3dnr_param.luma_sp_rad;
      dsp_3dnr_cfg.param_cfg.luma_te_max_bi_num = config.dsp3dnr_param.luma_te_max_bi_num;
      dsp_3dnr_cfg.param_cfg.luma_w0 = config.dsp3dnr_param.luma_w0;
      dsp_3dnr_cfg.param_cfg.luma_w1 = config.dsp3dnr_param.luma_w1;
      dsp_3dnr_cfg.param_cfg.luma_w2 = config.dsp3dnr_param.luma_w2;
      dsp_3dnr_cfg.param_cfg.luma_w3 = config.dsp3dnr_param.luma_w3;
      dsp_3dnr_cfg.param_cfg.luma_w4 = config.dsp3dnr_param.luma_w4;
      dsp_3dnr_cfg.param_cfg.chrm_default = config.dsp3dnr_param.chrm_default;
      dsp_3dnr_cfg.param_cfg.chrm_sp_rad = config.dsp3dnr_param.chrm_sp_rad;
      dsp_3dnr_cfg.param_cfg.chrm_te_max_bi_num = config.dsp3dnr_param.chrm_te_max_bi_num;
      dsp_3dnr_cfg.param_cfg.chrm_w0 = config.dsp3dnr_param.chrm_w0;
      dsp_3dnr_cfg.param_cfg.chrm_w1 = config.dsp3dnr_param.chrm_w1;
      dsp_3dnr_cfg.param_cfg.chrm_w2 = config.dsp3dnr_param.chrm_w2;
      dsp_3dnr_cfg.param_cfg.chrm_w3 = config.dsp3dnr_param.chrm_w3;
      dsp_3dnr_cfg.param_cfg.chrm_w4 = config.dsp3dnr_param.chrm_w4;
      dsp_3dnr_cfg.param_cfg.shp_default = config.dsp3dnr_param.shp_default;
      dsp_3dnr_cfg.param_cfg.src_shp_w0 = config.dsp3dnr_param.src_shp_w0;
      dsp_3dnr_cfg.param_cfg.src_shp_w1 = config.dsp3dnr_param.src_shp_w1;
      dsp_3dnr_cfg.param_cfg.src_shp_w2 = config.dsp3dnr_param.src_shp_w2;
      dsp_3dnr_cfg.param_cfg.src_shp_w3 = config.dsp3dnr_param.src_shp_w3;
      dsp_3dnr_cfg.param_cfg.src_shp_w4 = config.dsp3dnr_param.src_shp_w4;
      dsp_3dnr_cfg.param_cfg.src_shp_thr = config.dsp3dnr_param.src_shp_thr;
      dsp_3dnr_cfg.param_cfg.src_shp_div = config.dsp3dnr_param.src_shp_div;
      dsp_3dnr_cfg.param_cfg.src_shp_l = config.dsp3dnr_param.src_shp_l;
      dsp_3dnr_cfg.param_cfg.src_shp_c = config.dsp3dnr_param.src_shp_c;
    }

    mCamIA_DyCfg.dsp3dnr_mode = config.dsp3dnr_mode;
    mCamIA_DyCfg.dsp3dnr_level = config.dsp3dnr_level;
    mCamIA_DyCfg.dsp3dnr_param = config.dsp3dnr_param;
    osMutexUnlock(&mApiLock);
    configureISP(&cfg);
  } else
    osMutexUnlock(&mApiLock);

  osMutexLock(&mApiLock);
  if ((config.newDsp3dnr_mode != mCamIA_DyCfg.newDsp3dnr_mode) ||
    (memcmp(&config.newDsp3dnr_cfg, &mCamIA_DyCfg.newDsp3dnr_cfg, sizeof(config.newDsp3dnr_cfg)) != 0 )){
	struct HAL_ISP_cfg_s cfg;
    struct HAL_New3DnrCfg_s newDsp3dnr_cfg;

    cfg.updated_mask = 0;
    cfg.newDsp3DNR_cfg = NULL;
    cfg.updated_mask |= HAL_ISP_NEW_3DNR_MASK;

    if (config.newDsp3dnr_mode == HAL_MODE_OFF) {
      cfg.enabled[HAL_ISP_NEW_3DNR_ID] = HAL_ISP_ACTIVE_FALSE;
    } else if (config.newDsp3dnr_mode == HAL_MODE_AUTO) {
      cfg.enabled[HAL_ISP_NEW_3DNR_ID] = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      cfg.enabled[HAL_ISP_NEW_3DNR_ID] = HAL_ISP_ACTIVE_SETTING;
      cfg.newDsp3DNR_cfg = &newDsp3dnr_cfg;  
    }

	mCamIA_DyCfg.newDsp3dnr_mode = config.newDsp3dnr_mode;
	mCamIA_DyCfg.newDsp3dnr_cfg = config.newDsp3dnr_cfg;
	osMutexUnlock(&mApiLock);
    configureISP(&cfg);
	
  } else
    osMutexUnlock(&mApiLock);

  osMutexLock(&mApiLock);
  if ((config.bls_mode != mCamIA_DyCfg.bls_mode) ||
    memcmp(&config.bls_cfg, &mCamIA_DyCfg.bls_cfg, sizeof(config.bls_cfg))) {
    struct HAL_ISP_cfg_s cfg;
    struct HAL_ISP_bls_cfg_s bls_cfg;

    cfg.updated_mask = 0;
    cfg.updated_mask |= HAL_ISP_BLS_MASK;
    cfg.bls_cfg = NULL;
    if (config.bls_mode == HAL_MODE_OFF)
      cfg.enabled[HAL_ISP_BLS_ID] = HAL_ISP_ACTIVE_FALSE;
    else if (config.bls_mode == HAL_MODE_AUTO)
      cfg.enabled[HAL_ISP_BLS_ID] = HAL_ISP_ACTIVE_DEFAULT;
    else {
      cfg.enabled[HAL_ISP_BLS_ID] = HAL_ISP_ACTIVE_SETTING;
      bls_cfg = config.bls_cfg;
      cfg.bls_cfg = &bls_cfg;
    }

    mCamIA_DyCfg.bls_mode = config.bls_mode;
    mCamIA_DyCfg.bls_cfg = config.bls_cfg;
    osMutexUnlock(&mApiLock);
    configureISP(&cfg);
  } else
    osMutexUnlock(&mApiLock);

  osMutexLock(&mApiLock);
  if ((config.goc_mode != mCamIA_DyCfg.goc_mode) ||
    memcmp(&config.api_goc_cfg, &mCamIA_DyCfg.api_goc_cfg, sizeof(config.api_goc_cfg))) {
    struct HAL_ISP_cfg_s cfg;
    struct HAL_ISP_GOC_s calibdb_goc_cfg;

    cfg.updated_mask = 0;
    cfg.updated_mask |= HAL_ISP_CALIBDB_GOC_MASK;
    cfg.calibdb_goc_cfg = NULL;
    if (config.goc_mode == HAL_MODE_OFF) {
      cfg.enabled[HAL_ISP_CALIBDB_GOC_ID] = HAL_ISP_ACTIVE_FALSE;
      calibdb_goc_cfg = config.api_goc_cfg;
      cfg.calibdb_goc_cfg = &calibdb_goc_cfg;
    } else if (config.goc_mode == HAL_MODE_AUTO) {
      cfg.enabled[HAL_ISP_CALIBDB_GOC_ID] = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      cfg.enabled[HAL_ISP_CALIBDB_GOC_ID] = HAL_ISP_ACTIVE_SETTING;
      calibdb_goc_cfg = config.api_goc_cfg;
      cfg.calibdb_goc_cfg = &calibdb_goc_cfg;
    }

    mCamIA_DyCfg.goc_mode = config.goc_mode;
    mCamIA_DyCfg.api_goc_cfg = config.api_goc_cfg;
    osMutexUnlock(&mApiLock);
    configureISP(&cfg);
  } else
    osMutexUnlock(&mApiLock);

  osMutexLock(&mApiLock);
  if ((config.cproc_mode != mCamIA_DyCfg.cproc_mode) ||
    memcmp(&config.api_cproc_cfg, &mCamIA_DyCfg.api_cproc_cfg, sizeof(config.api_cproc_cfg))) {
    struct HAL_ISP_cfg_s cfg;
    struct HAL_ISP_CPROC_s calibdb_cproc_cfg;

    cfg.updated_mask = 0;
    cfg.updated_mask |= HAL_ISP_CALIBDB_CPROC_MASK;
    cfg.calibdb_cproc_cfg = NULL;
    if (config.cproc_mode == HAL_MODE_OFF) {
      cfg.enabled[HAL_ISP_CALIBDB_CPROC_ID] = HAL_ISP_ACTIVE_FALSE;
      calibdb_cproc_cfg = config.api_cproc_cfg;
      cfg.calibdb_cproc_cfg = &calibdb_cproc_cfg;
    } else if (config.cproc_mode == HAL_MODE_AUTO) {
      cfg.enabled[HAL_ISP_CALIBDB_CPROC_ID] = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      cfg.enabled[HAL_ISP_CALIBDB_CPROC_ID] = HAL_ISP_ACTIVE_SETTING;
      calibdb_cproc_cfg = config.api_cproc_cfg;
      cfg.calibdb_cproc_cfg = &calibdb_cproc_cfg;
    }

    mCamIA_DyCfg.cproc_mode = config.cproc_mode;
    mCamIA_DyCfg.api_cproc_cfg = config.api_cproc_cfg;
    osMutexUnlock(&mApiLock);
    configureISP(&cfg);
  } else
    osMutexUnlock(&mApiLock);

  osMutexLock(&mApiLock);
  if ((config.flt_mode != mCamIA_DyCfg.flt_mode) ||
    (config.flt_denoise != mCamIA_DyCfg.flt_denoise) ||
    (config.flt_sharp != mCamIA_DyCfg.flt_sharp)) {
    struct HAL_ISP_cfg_s cfg;
    struct HAL_ISP_flt_cfg_s flt_cfg;

    cfg.updated_mask = 0;
    cfg.flt_cfg = NULL;
    cfg.updated_mask |= HAL_ISP_FLT_MASK;
    if (config.flt_mode == HAL_MODE_OFF) {
      cfg.enabled[HAL_ISP_FLT_ID] = HAL_ISP_ACTIVE_FALSE;
    } else if (config.flt_mode == HAL_MODE_AUTO) {
      cfg.enabled[HAL_ISP_FLT_ID] = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      cfg.enabled[HAL_ISP_FLT_ID] = HAL_ISP_ACTIVE_SETTING;
      flt_cfg.denoise_level = config.flt_denoise;
      flt_cfg.sharp_level = config.flt_sharp;
      flt_cfg.light_mode = 0;
      cfg.flt_cfg = &flt_cfg;
    }

    mCamIA_DyCfg.flt_mode = config.flt_mode;
    mCamIA_DyCfg.flt_denoise = config.flt_denoise;
    mCamIA_DyCfg.flt_sharp = config.flt_sharp;
    osMutexUnlock(&mApiLock);
    configureISP(&cfg);
  } else
    osMutexUnlock(&mApiLock);

  if ((config.awb_lsc_mode != mCamIA_DyCfg.awb_lsc_mode) ||
    memcmp(&config.awb_lsc_pfl, &mCamIA_DyCfg.awb_lsc_pfl, sizeof(config.awb_lsc_pfl))) {
    struct HAL_ISP_cfg_s cfg;
    struct HAL_ISP_Lsc_Profile_s awb_lsc_pfl;

    cfg.updated_mask = 0;
    cfg.updated_mask |= HAL_ISP_AWB_LSC_MASK;
    cfg.awb_lsc_pfl = NULL;
    if (config.awb_lsc_mode == HAL_MODE_OFF)
      cfg.enabled[HAL_ISP_AWB_LSC_ID] = HAL_ISP_ACTIVE_FALSE;
    else if (config.awb_lsc_mode == HAL_MODE_AUTO)
      cfg.enabled[HAL_ISP_AWB_LSC_ID] = HAL_ISP_ACTIVE_DEFAULT;
    else {
      cfg.enabled[HAL_ISP_AWB_LSC_ID] = HAL_ISP_ACTIVE_SETTING;
      awb_lsc_pfl = config.awb_lsc_pfl;
      cfg.awb_lsc_pfl = &awb_lsc_pfl;
    }

    mCamIA_DyCfg.awb_lsc_mode = config.awb_lsc_mode;
    mCamIA_DyCfg.awb_lsc_pfl = config.awb_lsc_pfl;
    osMutexUnlock(&mApiLock);
    if (config.awb_lsc_mode != HAL_MODE_AUTO)
      configureISP(&cfg);
  } else
    osMutexUnlock(&mApiLock);

  osMutexLock(&mApiLock);
  if (memcmp(&config.adpf_dpf_cfg, &mCamIA_DyCfg.adpf_dpf_cfg, sizeof(config.adpf_dpf_cfg))) {
    struct HAL_ISP_cfg_s cfg;
    struct HAL_ISP_ADPF_DPF_s adpf_dpf;

    cfg.updated_mask = 0;
    cfg.updated_mask |= HAL_ISP_ADPF_DPF_MASK;
    cfg.adpf_dpf = NULL;

    if (config.adpf_dpf_cfg.dpf_enable == HAL_MODE_OFF)
      cfg.enabled[HAL_ISP_ADPF_DPF_ID] = HAL_ISP_ACTIVE_FALSE;
    else
      cfg.enabled[HAL_ISP_ADPF_DPF_ID] = HAL_ISP_ACTIVE_SETTING;

    adpf_dpf = config.adpf_dpf_cfg;
    cfg.adpf_dpf = &adpf_dpf;
    mCamIA_DyCfg.adpf_dpf_cfg = config.adpf_dpf_cfg;
    osMutexUnlock(&mApiLock);
    configureISP(&cfg);
  } else
    osMutexUnlock(&mApiLock);

  osMutexLock(&mApiLock);
  if (memcmp(&config.adpf_flt_cfg, &mCamIA_DyCfg.adpf_flt_cfg, sizeof(config.adpf_flt_cfg))) {
    struct HAL_ISP_cfg_s cfg;
    struct HAL_ISP_FLT_Set_s adpf_flt;

    cfg.updated_mask = 0;
    cfg.updated_mask |= HAL_ISP_ADPF_FLT_MASK;
    cfg.adpf_flt = NULL;
    if (config.adpf_flt_cfg.filter_enable == HAL_MODE_OFF)
      cfg.enabled[HAL_ISP_ADPF_FLT_ID] = HAL_ISP_ACTIVE_FALSE;
    else
      cfg.enabled[HAL_ISP_ADPF_FLT_ID] = HAL_ISP_ACTIVE_SETTING;

    adpf_flt = config.adpf_flt_cfg;
    cfg.adpf_flt = &adpf_flt;

    mCamIA_DyCfg.adpf_flt_cfg = config.adpf_flt_cfg;
    osMutexUnlock(&mApiLock);
    configureISP(&cfg);
  } else
    osMutexUnlock(&mApiLock);

  osMutexLock(&mApiLock);
  if ((config.awb_ccm_mode != mCamIA_DyCfg.awb_ccm_mode) ||
    memcmp(&config.awb_ccm_pfl, &mCamIA_DyCfg.awb_ccm_pfl, sizeof(config.awb_ccm_pfl))) {
    struct HAL_ISP_cfg_s cfg;
    struct HAL_ISP_AWB_CCM_SET_s awb_ccm_pfl;

    cfg.updated_mask = 0;
    cfg.updated_mask |= HAL_ISP_AWB_CCM_MASK;
    cfg.awb_ccm_pfl = NULL;
    if (config.awb_ccm_mode == HAL_MODE_OFF)
      cfg.enabled[HAL_ISP_AWB_CCM_ID] = HAL_ISP_ACTIVE_FALSE;
    else if (config.awb_ccm_mode == HAL_MODE_AUTO)
      cfg.enabled[HAL_ISP_AWB_CCM_ID] = HAL_ISP_ACTIVE_DEFAULT;
    else {
      cfg.enabled[HAL_ISP_AWB_CCM_ID] = HAL_ISP_ACTIVE_SETTING;
      awb_ccm_pfl = config.awb_ccm_pfl;
      cfg.awb_ccm_pfl = &awb_ccm_pfl;
    }

    mCamIA_DyCfg.awb_ccm_mode = config.awb_ccm_mode;
    mCamIA_DyCfg.awb_ccm_pfl = config.awb_ccm_pfl;
    osMutexUnlock(&mApiLock);
    if (config.awb_ccm_mode != HAL_MODE_AUTO)
      configureISP(&cfg);
  } else
    osMutexUnlock(&mApiLock);

  osMutexLock(&mApiLock);
  if ((config.awb_gain_illu_mode != mCamIA_DyCfg.awb_gain_illu_mode) ||
    memcmp(&config.awb_gain_illu_pfl, &mCamIA_DyCfg.awb_gain_illu_pfl, sizeof(config.awb_gain_illu_pfl))) {
    struct HAL_ISP_cfg_s cfg;
    struct HAL_ISP_AWB_s awb_gain_illu_pfl;

    cfg.updated_mask = 0;
    cfg.updated_mask |= HAL_ISP_AWB_GAIN_ILLU_MASK;
    cfg.awb_gain_illu_pfl = NULL;
    if (config.awb_gain_illu_mode == HAL_MODE_OFF) {
      cfg.enabled[HAL_ISP_AWB_GAIN_ILLU_ID] = HAL_ISP_ACTIVE_FALSE;
      awb_gain_illu_pfl = config.awb_gain_illu_pfl;
      cfg.awb_gain_illu_pfl = &awb_gain_illu_pfl;
    } else if (config.awb_gain_illu_mode == HAL_MODE_AUTO)
      cfg.enabled[HAL_ISP_AWB_GAIN_ILLU_ID] = HAL_ISP_ACTIVE_DEFAULT;
    else {
      cfg.enabled[HAL_ISP_AWB_GAIN_ILLU_ID] = HAL_ISP_ACTIVE_SETTING;
      awb_gain_illu_pfl = config.awb_gain_illu_pfl;
      cfg.awb_gain_illu_pfl = &awb_gain_illu_pfl;
    }

    mCamIA_DyCfg.awb_gain_illu_mode = config.awb_gain_illu_mode;
    mCamIA_DyCfg.awb_gain_illu_pfl = config.awb_gain_illu_pfl;
    osMutexUnlock(&mApiLock);
    if (config.awb_gain_illu_mode != HAL_MODE_AUTO)
      configureISP(&cfg);
  } else
    osMutexUnlock(&mApiLock);

  osMutexLock(&mApiLock);
  if (memcmp(&config.awb_refgain, &mCamIA_DyCfg.awb_refgain, sizeof(config.awb_refgain))) {
    struct HAL_ISP_cfg_s cfg;
    struct HAL_ISP_AWB_RefGain_s awb_refgain;

    cfg.updated_mask = 0;
    cfg.updated_mask |= HAL_ISP_AWB_REFGAIN_MASK;
    cfg.awb_refgain = NULL;

    if ((config.awb_refgain.refRGain == 0) && (config.awb_refgain.refGrGain == 0) &&
        (config.awb_refgain.refGbGain == 0) && (config.awb_refgain.refBGain == 0))
      cfg.enabled[HAL_ISP_AWB_REFGAIN_ID] = HAL_ISP_ACTIVE_FALSE;
    else
      cfg.enabled[HAL_ISP_AWB_REFGAIN_ID] = HAL_ISP_ACTIVE_SETTING;

    awb_refgain = config.awb_refgain;
    cfg.awb_refgain = &awb_refgain;
    mCamIA_DyCfg.awb_refgain = config.awb_refgain;
    osMutexUnlock(&mApiLock);
    configureISP(&cfg);
  } else
    osMutexUnlock(&mApiLock);

  osMutexLock(&mApiLock);
  if (memcmp(&config.awb_curve, &mCamIA_DyCfg.awb_curve, sizeof(config.awb_curve))) {
    struct HAL_ISP_cfg_s cfg;
    struct HAL_ISP_AWB_Curve_s awb_curve;

    cfg.updated_mask = 0;
    cfg.updated_mask |= HAL_ISP_AWB_CURVE_MASK;
    cfg.awb_curve = NULL;

    if (config.awb_curve.Kfactor == 0)
      cfg.enabled[HAL_ISP_AWB_CURVE_ID] = HAL_ISP_ACTIVE_FALSE;
    else
      cfg.enabled[HAL_ISP_AWB_CURVE_ID] = HAL_ISP_ACTIVE_SETTING;

    awb_curve = config.awb_curve;
    cfg.awb_curve = &awb_curve;
    mCamIA_DyCfg.awb_curve = config.awb_curve;
    osMutexUnlock(&mApiLock);
    configureISP(&cfg);
  } else
    osMutexUnlock(&mApiLock);

  osMutexLock(&mApiLock);
  if (memcmp(&config.awb_wp_set, &mCamIA_DyCfg.awb_wp_set, sizeof(config.awb_wp_set))) {
    struct HAL_ISP_cfg_s cfg;
    struct HAL_ISP_AWB_White_Point_Set_s awb_wp_set;

    cfg.updated_mask = 0;
    cfg.updated_mask |= HAL_ISP_AWB_WP_SET_MASK;
    cfg.awb_wp_set = NULL;

    if (config.awb_wp_set.win_width == 0)
      cfg.enabled[HAL_ISP_AWB_WP_SET_ID] = HAL_ISP_ACTIVE_FALSE;
    else
      cfg.enabled[HAL_ISP_AWB_WP_SET_ID] = HAL_ISP_ACTIVE_SETTING;

    awb_wp_set = config.awb_wp_set;
    cfg.awb_wp_set = &awb_wp_set;
    mCamIA_DyCfg.awb_wp_set = config.awb_wp_set;
    osMutexUnlock(&mApiLock);
    configureISP(&cfg);
  } else
    osMutexUnlock(&mApiLock);

  return ret;
}

/* control ISP module directly*/
bool CamIsp1xCtrItf::configureISP(const void* config) {

  osMutexLock(&mApiLock);
  struct HAL_ISP_cfg_s* cfg = (struct HAL_ISP_cfg_s*)config;
  bool_t ret = BOOL_TRUE;
  /* following configs may confilt with 3A algorithm */
  if (cfg->updated_mask & HAL_ISP_HST_MASK) {
    if (cfg->enabled[HAL_ISP_HST_ID] && cfg->hst_cfg) {
      mHstNeededUpdate = BOOL_TRUE;
      mHstEnabled = HAL_ISP_ACTIVE_SETTING;
      hst_cfg = *cfg->hst_cfg;
    } else if (!cfg->enabled[HAL_ISP_HST_ID]) {
      mHstNeededUpdate = BOOL_TRUE;
      mHstEnabled = HAL_ISP_ACTIVE_FALSE;
    } else if (cfg->enabled[HAL_ISP_HST_ID]) {
      mHstNeededUpdate = BOOL_TRUE;
      mHstEnabled = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      mHstNeededUpdate = BOOL_FALSE;
      ALOGE("%s:can't config ISP hst !", __func__);
      goto config_end;
    }
  }

  if (cfg->updated_mask & HAL_ISP_AEC_MASK) {
    if (cfg->enabled[HAL_ISP_AEC_ID] && cfg->aec_cfg) {
      mAecNeededUpdate = BOOL_TRUE;
      mAecEnabled = HAL_ISP_ACTIVE_SETTING;
      aec_cfg = *cfg->aec_cfg;
    } else if (!cfg->enabled[HAL_ISP_AEC_ID]) {
      mAecNeededUpdate = BOOL_TRUE;
      mAecEnabled = HAL_ISP_ACTIVE_FALSE;
      if (cfg->aec_cfg) {
        aec_cfg = *cfg->aec_cfg;
      } else {
        aec_cfg.exp_time = 0.0f;
        aec_cfg.exp_gain = 0.0f;
      }
    } else if (cfg->enabled[HAL_ISP_AEC_ID]) {
      mAecNeededUpdate = BOOL_TRUE;
      mAecEnabled = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      mAecNeededUpdate = BOOL_FALSE;
      ALOGE("%s:can't config ISP aec !", __func__);
      goto config_end;
    }
  }

  if (cfg->updated_mask & HAL_ISP_LSC_MASK) {
    if (cfg->enabled[HAL_ISP_LSC_ID] && cfg->lsc_cfg) {
      mLscNeededUpdate = BOOL_TRUE;
      mLscEnabled = HAL_ISP_ACTIVE_SETTING;
      lsc_cfg = *cfg->lsc_cfg;
    } else if (!cfg->enabled[HAL_ISP_LSC_ID]) {
      mLscNeededUpdate = BOOL_TRUE;
      mLscEnabled = HAL_ISP_ACTIVE_FALSE;
    } else if (cfg->enabled[HAL_ISP_LSC_ID]) {
      mLscNeededUpdate = BOOL_TRUE;
      mLscEnabled = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      mLscNeededUpdate = BOOL_FALSE;
      ALOGE("%s:can't config ISP lsc !", __func__);
      goto config_end;
    }
  }

  if (cfg->updated_mask & HAL_ISP_AWB_GAIN_MASK) {
    if (cfg->enabled[HAL_ISP_AWB_GAIN_ID] && cfg->awb_gain_cfg) {
      mAwbGainNeededUpdate = BOOL_TRUE;
      mAwbEnabled = HAL_ISP_ACTIVE_SETTING;
      awb_gain_cfg = *cfg->awb_gain_cfg;
    } else if (!cfg->enabled[HAL_ISP_AWB_GAIN_ID]) {
      mAwbGainNeededUpdate = BOOL_TRUE;
      mAwbEnabled = HAL_ISP_ACTIVE_FALSE;
    } else if (cfg->enabled[HAL_ISP_AWB_GAIN_ID]) {
      mAwbGainNeededUpdate = BOOL_TRUE;
      mAwbEnabled = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      mAwbGainNeededUpdate = BOOL_FALSE;
      ALOGE("%s:can't config ISP awb gain !", __func__);
      goto config_end;
    }
  }

  if (cfg->updated_mask & HAL_ISP_BPC_MASK) {
    if (cfg->enabled[HAL_ISP_BPC_ID] && cfg->dpcc_cfg) {
      mDpccNeededUpdate = BOOL_TRUE;
      mDpccEnabled = HAL_ISP_ACTIVE_SETTING;
      dpcc_cfg = *cfg->dpcc_cfg;
    } else if (!cfg->enabled[HAL_ISP_BPC_ID]) {
      mDpccNeededUpdate = BOOL_TRUE;
      mDpccEnabled = HAL_ISP_ACTIVE_FALSE;
    } else if (cfg->enabled[HAL_ISP_BPC_ID]) {
      mDpccNeededUpdate = BOOL_TRUE;
      mDpccEnabled = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      mDpccNeededUpdate = BOOL_FALSE;
      ALOGE("%s:can't config ISP dpcc !", __func__);
      goto config_end;
    }
  }

  if (cfg->updated_mask & HAL_ISP_SDG_MASK) {
    if (cfg->enabled[HAL_ISP_SDG_ID] && cfg->sdg_cfg) {
      mSdgNeededUpdate = BOOL_TRUE;
      mSdgEnabled = HAL_ISP_ACTIVE_SETTING;
      sdg_cfg = *cfg->sdg_cfg;
    } else if (!cfg->enabled[HAL_ISP_SDG_ID]) {
      mSdgNeededUpdate = BOOL_TRUE;
      mSdgEnabled = HAL_ISP_ACTIVE_FALSE;
    } else if (cfg->enabled[HAL_ISP_SDG_ID]) {
      mSdgNeededUpdate = BOOL_TRUE;
      mSdgEnabled = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      mSdgNeededUpdate = BOOL_FALSE;
      ALOGE("%s:can't config ISP sdg !", __func__);
      goto config_end;
    }
  }

  if (cfg->updated_mask & HAL_ISP_CTK_MASK) {
    if (cfg->enabled[HAL_ISP_CTK_ID] && cfg->ctk_cfg) {
      mCtkNeededUpdate = BOOL_TRUE;
      mCtkEnabled = HAL_ISP_ACTIVE_SETTING;
      ctk_cfg = *cfg->ctk_cfg;
    } else if (!cfg->enabled[HAL_ISP_CTK_ID]) {
      mCtkNeededUpdate = BOOL_TRUE;
      mCtkEnabled = HAL_ISP_ACTIVE_FALSE;
    } else if (cfg->enabled[HAL_ISP_CTK_ID]) {
      mCtkNeededUpdate = BOOL_TRUE;
      mCtkEnabled = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      mCtkNeededUpdate = BOOL_FALSE;
      ALOGE("%s:can't config ISP ctk !", __func__);
      goto config_end;
    }
  }

  if (cfg->updated_mask & HAL_ISP_AWB_MEAS_MASK) {
    if (cfg->enabled[HAL_ISP_AWB_MEAS_ID] && cfg->awb_cfg) {
      mAwbMeNeededUpdate = BOOL_TRUE;
      mAwbMeEnabled = HAL_ISP_ACTIVE_SETTING;
      awb_cfg = *cfg->awb_cfg;
    } else if (!cfg->enabled[HAL_ISP_AWB_MEAS_ID]) {
      mAwbMeNeededUpdate = BOOL_TRUE;
      mAwbMeEnabled = HAL_ISP_ACTIVE_FALSE;
      if (cfg->awb_cfg) {
        awb_cfg.illuIndex =  cfg->awb_cfg->illuIndex;
      } else {
        awb_cfg.illuIndex =  -1;
      }
    } else if (cfg->enabled[HAL_ISP_AWB_MEAS_ID]) {
      mAwbMeNeededUpdate = BOOL_TRUE;
      mAwbMeEnabled = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      mAwbMeNeededUpdate = BOOL_FALSE;
      ALOGE("%s:can't config ISP awb measure !", __func__);
      goto config_end;
    }
  }

  if (cfg->updated_mask & HAL_ISP_AFC_MASK) {
    if (cfg->enabled[HAL_ISP_AFC_ID] && cfg->afc_cfg) {
      mAfcNeededUpdate = BOOL_TRUE;
      mAfcEnabled = HAL_ISP_ACTIVE_SETTING;
      afc_cfg = *cfg->afc_cfg;
    } else if (!cfg->enabled[HAL_ISP_AFC_ID]) {
      mAfcNeededUpdate = BOOL_TRUE;
      mAfcEnabled = HAL_ISP_ACTIVE_FALSE;
    } else if (cfg->enabled[HAL_ISP_AFC_ID]) {
      mAfcNeededUpdate = BOOL_TRUE;
      mAfcEnabled = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      mAfcNeededUpdate = BOOL_FALSE;
      ALOGE("%s:can't config ISP afc !", __func__);
      goto config_end;
    }
  }

  if (cfg->updated_mask & HAL_ISP_DPF_MASK) {
    if (cfg->enabled[HAL_ISP_DPF_ID] && cfg->dpf_cfg) {
      mDpfNeededUpdate = BOOL_TRUE;
      mDpfEnabled = HAL_ISP_ACTIVE_SETTING;
      dpf_cfg = *cfg->dpf_cfg;
    } else if (!cfg->enabled[HAL_ISP_DPF_ID]) {
      mDpfNeededUpdate = BOOL_TRUE;
      mDpfEnabled = HAL_ISP_ACTIVE_FALSE;
    } else if (cfg->enabled[HAL_ISP_DPF_ID]) {
      mDpfNeededUpdate = BOOL_TRUE;
      mDpfEnabled = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      mDpfNeededUpdate = BOOL_FALSE;
      ALOGE("%s:can't config ISP dpf !", __func__);
      goto config_end;
    }
  }

  if (cfg->updated_mask & HAL_ISP_DPF_STRENGTH_MASK) {
    if (cfg->enabled[HAL_ISP_DPF_STRENGTH_ID] && cfg->dpf_strength_cfg) {
      mDpfStrengthNeededUpdate = BOOL_TRUE;
      mDpfStrengthEnabled = HAL_ISP_ACTIVE_SETTING;
      dpf_strength_cfg = *cfg->dpf_strength_cfg;
    } else if (!cfg->enabled[HAL_ISP_DPF_STRENGTH_ID]) {
      mDpfStrengthNeededUpdate = BOOL_TRUE;
      mDpfStrengthEnabled = HAL_ISP_ACTIVE_FALSE;
    } else if (cfg->enabled[HAL_ISP_DPF_STRENGTH_ID]) {
      mDpfStrengthNeededUpdate = BOOL_TRUE;
      mDpfStrengthEnabled = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      mDpfStrengthNeededUpdate = BOOL_FALSE;
      ALOGE("%s:can't config ISP dpf strength!", __func__);
      goto config_end;
    }
  }

  /* following configs may confilt with user settings */

  if (cfg->updated_mask & HAL_ISP_CPROC_MASK) {
    if (cfg->enabled[HAL_ISP_CPROC_ID] && cfg->cproc_cfg) {
      mCprocNeededUpdate = BOOL_TRUE;
      mCprocEnabled = HAL_ISP_ACTIVE_SETTING;
      cproc_cfg = *cfg->cproc_cfg;
    } else if (!cfg->enabled[HAL_ISP_CPROC_ID]) {
      mCprocNeededUpdate = BOOL_TRUE;
      mCprocEnabled = HAL_ISP_ACTIVE_FALSE;
    } else if (cfg->enabled[HAL_ISP_CPROC_ID]) {
      mCprocNeededUpdate = BOOL_TRUE;
      mCprocEnabled = HAL_ISP_ACTIVE_DEFAULT;
      //cproc_cfg = *cfg->cproc_cfg;
    } else {
      mCprocNeededUpdate = BOOL_FALSE;
      ALOGE("%s:can't config ISP cproc!", __func__);
      goto config_end;
    }
  }

  if (cfg->updated_mask & HAL_ISP_IE_MASK) {
    if (cfg->enabled[HAL_ISP_IE_ID] && cfg->ie_cfg) {
      mIeNeededUpdate = BOOL_TRUE;
      mIeEnabled = HAL_ISP_ACTIVE_SETTING;
      ie_cfg = *cfg->ie_cfg;
    } else if (!cfg->enabled[HAL_ISP_IE_ID]) {
      mIeNeededUpdate = BOOL_TRUE;
      mIeEnabled = HAL_ISP_ACTIVE_FALSE;
    } else if (cfg->enabled[HAL_ISP_IE_ID]) {
      mIeNeededUpdate = BOOL_TRUE;
      mIeEnabled = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      mIeNeededUpdate = BOOL_FALSE;
      ALOGE("%s:can't config ISP ie!", __func__);
      goto config_end;
    }
  }

  /* can config free*/
  if (cfg->updated_mask & HAL_ISP_GOC_MASK) {
    if (cfg->enabled[HAL_ISP_GOC_ID]== HAL_ISP_ACTIVE_SETTING && cfg->goc_cfg) {
      mGocNeededUpdate = BOOL_TRUE;
      mGocEnabled = HAL_ISP_ACTIVE_SETTING;
      goc_cfg = *cfg->goc_cfg;
    } else if (!cfg->enabled[HAL_ISP_GOC_ID]) {
      mGocNeededUpdate = BOOL_TRUE;
      mGocEnabled = HAL_ISP_ACTIVE_FALSE;
    } else if (cfg->enabled[HAL_ISP_GOC_ID]) {
      mGocNeededUpdate = BOOL_TRUE;
      mGocEnabled = HAL_ISP_ACTIVE_DEFAULT;
      if(cfg->goc_cfg){
        goc_cfg = *cfg->goc_cfg;
      }
    } else {
      mGocNeededUpdate = BOOL_FALSE;
      ALOGE("%s:can't config ISP goc!", __func__);
      goto config_end;
    }
  }

  if (cfg->updated_mask & HAL_ISP_FLT_MASK) {
    if (cfg->enabled[HAL_ISP_FLT_ID] && cfg->flt_cfg) {
      mFltNeededUpdate = BOOL_TRUE;
      mFltEnabled = HAL_ISP_ACTIVE_SETTING;
      flt_cfg = *cfg->flt_cfg;
    } else if (!cfg->enabled[HAL_ISP_FLT_ID]) {
      mFltNeededUpdate = BOOL_TRUE;
      mFltEnabled = HAL_ISP_ACTIVE_FALSE;
    } else if (cfg->enabled[HAL_ISP_FLT_ID]) {
      mFltNeededUpdate = BOOL_TRUE;
      mFltEnabled = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      mFltNeededUpdate = BOOL_FALSE;
      ALOGE("%s:can't config ISP flt!", __func__);
      goto config_end;
    }
  }

  if (cfg->updated_mask & HAL_ISP_BDM_MASK) {
    if (cfg->enabled[HAL_ISP_BDM_ID] && cfg->bdm_cfg) {
      mBdmNeededUpdate = BOOL_TRUE;
      mBdmEnabled = HAL_ISP_ACTIVE_SETTING;
      bdm_cfg = *cfg->bdm_cfg;
    } else if (!cfg->enabled[HAL_ISP_BDM_ID]) {
      mBdmNeededUpdate = BOOL_TRUE;
      mBdmEnabled = HAL_ISP_ACTIVE_FALSE;
    } else if (cfg->enabled[HAL_ISP_BDM_ID]) {
      mBdmNeededUpdate = BOOL_TRUE;
      mBdmEnabled = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      mBdmNeededUpdate = BOOL_FALSE;
      ALOGE("%s:can't config ISP bdm!", __func__);
      goto config_end;
    }
  }

  if (cfg->updated_mask & HAL_ISP_BLS_MASK) {
    if (cfg->enabled[HAL_ISP_BLS_ID] && cfg->bls_cfg) {
      mBlsNeededUpdate = BOOL_TRUE;
      mBlsEnabled = HAL_ISP_ACTIVE_SETTING;
      bls_cfg = *cfg->bls_cfg;
    } else if (!cfg->enabled[HAL_ISP_BLS_ID]) {
      mBlsNeededUpdate = BOOL_TRUE;
      mBlsEnabled = HAL_ISP_ACTIVE_FALSE;
    } else if (cfg->enabled[HAL_ISP_BLS_ID]) {
      mBlsNeededUpdate = BOOL_TRUE;
      mBlsEnabled = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      mBlsNeededUpdate = BOOL_FALSE;
      ALOGE("%s:can't config bls !", __func__);
      goto config_end;
    }
  }

  if (cfg->updated_mask & HAL_ISP_WDR_MASK) {
    if (cfg->enabled[HAL_ISP_WDR_ID] && cfg->wdr_cfg) {
      mWdrNeededUpdate = BOOL_TRUE;
      mWdrEnabled = HAL_ISP_ACTIVE_SETTING;
      wdr_cfg = *cfg->wdr_cfg;
    } else if (!cfg->enabled[HAL_ISP_WDR_ID]) {
      mWdrNeededUpdate = BOOL_TRUE;
      mWdrEnabled = HAL_ISP_ACTIVE_FALSE;
    } else if (cfg->enabled[HAL_ISP_WDR_ID]) {
      mWdrNeededUpdate = BOOL_TRUE;
      mWdrEnabled = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      mWdrNeededUpdate = BOOL_FALSE;
      ALOGE("%s:can't config ISP bdm!", __func__);
      goto config_end;
    }
  }

  if (cfg->updated_mask & HAL_ISP_3DNR_MASK) {
    if (cfg->enabled[HAL_ISP_3DNR_ID] && cfg->dsp_3dnr_cfg) {
      m3DnrNeededUpdate = BOOL_TRUE;
      m3DnrEnabled = HAL_ISP_ACTIVE_SETTING;
      dsp_3dnr_cfg = *cfg->dsp_3dnr_cfg;
    } else if (!cfg->enabled[HAL_ISP_3DNR_ID]) {
      m3DnrNeededUpdate = BOOL_TRUE;
      m3DnrEnabled = HAL_ISP_ACTIVE_FALSE;
    } else if (cfg->enabled[HAL_ISP_3DNR_ID]) {
      m3DnrNeededUpdate = BOOL_TRUE;
      m3DnrEnabled = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      m3DnrNeededUpdate = BOOL_FALSE;
      ALOGE("%s:can't config dsp 3dnr!", __func__);
    }
  }

  if (cfg->updated_mask & HAL_ISP_NEW_3DNR_MASK) {
    if (cfg->enabled[HAL_ISP_NEW_3DNR_ID] && cfg->newDsp3DNR_cfg) {
      mNew3DnrNeededUpdate = BOOL_TRUE;
      mNew3DnrEnabled = HAL_ISP_ACTIVE_SETTING;
      new_dsp_3dnr_cfg = *cfg->newDsp3DNR_cfg;
    } else if (!cfg->enabled[HAL_ISP_NEW_3DNR_ID]) {
      mNew3DnrNeededUpdate = BOOL_TRUE;
      mNew3DnrEnabled = HAL_ISP_ACTIVE_FALSE;
    } else if (cfg->enabled[HAL_ISP_NEW_3DNR_ID]) {
      mNew3DnrNeededUpdate = BOOL_TRUE;
      mNew3DnrEnabled = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      mNew3DnrNeededUpdate = BOOL_FALSE;
      ALOGE("%s:can't config new dsp 3dnr!", __func__);
    }
  }

  if (cfg->updated_mask & HAL_ISP_AWB_LSC_MASK) {
    if (cfg->enabled[HAL_ISP_AWB_LSC_ID] && cfg->awb_lsc_pfl) {
      mAwbLscNeededUpdate = BOOL_TRUE;
      mAwbLscEnabled = HAL_ISP_ACTIVE_SETTING;
      awb_lsc_pfl = *cfg->awb_lsc_pfl;
    } else if (!cfg->enabled[HAL_ISP_AWB_LSC_ID]) {
      mAwbLscNeededUpdate = BOOL_TRUE;
      mAwbLscEnabled = HAL_ISP_ACTIVE_FALSE;
    } else if (cfg->enabled[HAL_ISP_AWB_LSC_ID]) {
      mAwbLscNeededUpdate = BOOL_TRUE;
      mAwbLscEnabled = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      mAwbLscNeededUpdate = BOOL_FALSE;
      ALOGE("%s:can't config AWB Lsc!", __func__);
      goto config_end;
    }
  }

  if (cfg->updated_mask & HAL_ISP_ADPF_DPF_MASK) {
    if (cfg->enabled[HAL_ISP_ADPF_DPF_ID] && cfg->adpf_dpf) {
      mAdpfDpfNeededUpdate = BOOL_TRUE;
      mAdpfDpfEnabled = HAL_ISP_ACTIVE_SETTING;
      adpf_dpf = *cfg->adpf_dpf;
    } else if (!cfg->enabled[HAL_ISP_ADPF_DPF_ID]) {
      mAdpfDpfNeededUpdate = BOOL_TRUE;
      mAdpfDpfEnabled = HAL_ISP_ACTIVE_FALSE;
      adpf_dpf = *cfg->adpf_dpf;
    } else if (cfg->enabled[HAL_ISP_ADPF_DPF_ID]) {
      mAdpfDpfNeededUpdate = BOOL_TRUE;
      mAdpfDpfEnabled = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      mAdpfDpfNeededUpdate = BOOL_FALSE;
      ALOGE("%s:can't config ADPF DPF!", __func__);
      goto config_end;
    }
  }

  if (cfg->updated_mask & HAL_ISP_ADPF_FLT_MASK) {
    if (cfg->enabled[HAL_ISP_ADPF_FLT_ID] && cfg->adpf_flt) {
      mAdpfFltNeededUpdate = BOOL_TRUE;
      mAdpfFltEnabled = HAL_ISP_ACTIVE_SETTING;
      adpf_flt = *cfg->adpf_flt;
    } else if (!cfg->enabled[HAL_ISP_ADPF_FLT_ID]) {
      mAdpfFltNeededUpdate = BOOL_TRUE;
      mAdpfFltEnabled = HAL_ISP_ACTIVE_FALSE;
      adpf_flt = *cfg->adpf_flt;
    } else if (cfg->enabled[HAL_ISP_ADPF_FLT_ID]) {
      mAdpfFltNeededUpdate = BOOL_TRUE;
      mAdpfFltEnabled = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      mAdpfFltNeededUpdate = BOOL_FALSE;
      ALOGE("%s:can't config ADPF FLT!", __func__);
      goto config_end;
    }
  }

  if (cfg->updated_mask & HAL_ISP_AWB_CCM_MASK) {
    if (cfg->enabled[HAL_ISP_AWB_CCM_ID] && cfg->awb_ccm_pfl) {
      mAwbCcmNeededUpdate = BOOL_TRUE;
      mAwbCcmEnabled = HAL_ISP_ACTIVE_SETTING;
      awb_ccm_pfl = *cfg->awb_ccm_pfl;
    } else if (!cfg->enabled[HAL_ISP_AWB_CCM_ID]) {
      mAwbCcmNeededUpdate = BOOL_TRUE;
      mAwbCcmEnabled = HAL_ISP_ACTIVE_FALSE;
    } else if (cfg->enabled[HAL_ISP_AWB_CCM_ID]) {
      mAwbCcmNeededUpdate = BOOL_TRUE;
      mAwbCcmEnabled = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      mAwbCcmNeededUpdate = BOOL_FALSE;
      ALOGE("%s:can't config AWB Ccm!", __func__);
      goto config_end;
    }
  }

  if (cfg->updated_mask & HAL_ISP_AWB_GAIN_ILLU_MASK) {
    if (cfg->enabled[HAL_ISP_AWB_GAIN_ILLU_ID] && cfg->awb_gain_illu_pfl) {
      mAwbGainIlluNeededUpdate = BOOL_TRUE;
      mAwbGainIlluEnabled = HAL_ISP_ACTIVE_SETTING;
      awb_gain_illu_pfl = *cfg->awb_gain_illu_pfl;
    } else if (!cfg->enabled[HAL_ISP_AWB_GAIN_ILLU_ID]) {
      mAwbGainIlluNeededUpdate = BOOL_TRUE;
      mAwbGainIlluEnabled = HAL_ISP_ACTIVE_FALSE;
      awb_gain_illu_pfl = *cfg->awb_gain_illu_pfl;
    } else if (cfg->enabled[HAL_ISP_AWB_GAIN_ILLU_ID]) {
      mAwbGainIlluNeededUpdate = BOOL_TRUE;
      mAwbGainIlluEnabled = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      mAwbGainIlluNeededUpdate = BOOL_FALSE;
      ALOGE("%s:can't config AWB Gain Illu!", __func__);
      goto config_end;
    }
  }

  if (cfg->updated_mask & HAL_ISP_AWB_REFGAIN_MASK) {
    if (cfg->enabled[HAL_ISP_AWB_REFGAIN_ID] && cfg->awb_refgain) {
      mAwbRefGainNeededUpdate = BOOL_TRUE;
      mAwbRefGainEnabled = HAL_ISP_ACTIVE_SETTING;
      awb_refgain = *cfg->awb_refgain;
    } else if (!cfg->enabled[HAL_ISP_AWB_REFGAIN_ID]) {
      mAwbRefGainNeededUpdate = BOOL_TRUE;
      mAwbRefGainEnabled = HAL_ISP_ACTIVE_FALSE;
      awb_refgain = *cfg->awb_refgain;
    } else if (cfg->enabled[HAL_ISP_AWB_REFGAIN_ID]) {
      mAwbRefGainNeededUpdate = BOOL_TRUE;
      mAwbRefGainEnabled = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      mAwbRefGainNeededUpdate = BOOL_FALSE;
      ALOGE("%s:can't config AWB ref gain!", __func__);
      goto config_end;
    }
  }

  if (cfg->updated_mask & HAL_ISP_AWB_CURVE_MASK) {
    if (cfg->enabled[HAL_ISP_AWB_CURVE_ID] && cfg->awb_curve) {
      mAwbCurveNeededUpdate = BOOL_TRUE;
      mAwbCurveEnabled = HAL_ISP_ACTIVE_SETTING;
      awb_curve = *cfg->awb_curve;
    } else if (!cfg->enabled[HAL_ISP_AWB_CURVE_ID]) {
      mAwbCurveNeededUpdate = BOOL_TRUE;
      mAwbCurveEnabled = HAL_ISP_ACTIVE_FALSE;
      awb_curve = *cfg->awb_curve;
    } else if (cfg->enabled[HAL_ISP_AWB_CURVE_ID]) {
      mAwbCurveNeededUpdate = BOOL_TRUE;
      mAwbCurveEnabled = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      mAwbCurveNeededUpdate = BOOL_FALSE;
      ALOGE("%s:can't config AWB curve!", __func__);
      goto config_end;
    }
  }

  if (cfg->updated_mask & HAL_ISP_AWB_WP_SET_MASK) {
    if (cfg->enabled[HAL_ISP_AWB_WP_SET_ID] && cfg->awb_wp_set) {
      mAwbWpSetNeededUpdate = BOOL_TRUE;
      mAwbWpSetEnabled = HAL_ISP_ACTIVE_SETTING;
      awb_wp_set = *cfg->awb_wp_set;
    } else if (!cfg->enabled[HAL_ISP_AWB_WP_SET_ID]) {
      mAwbWpSetNeededUpdate = BOOL_TRUE;
      mAwbWpSetEnabled = HAL_ISP_ACTIVE_FALSE;
      awb_wp_set = *cfg->awb_wp_set;
    } else if (cfg->enabled[HAL_ISP_AWB_WP_SET_ID]) {
      mAwbWpSetNeededUpdate = BOOL_TRUE;
      mAwbWpSetEnabled = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      mAwbWpSetNeededUpdate = BOOL_FALSE;
      ALOGE("%s:can't config AWB write point!", __func__);
      goto config_end;
    }
  }

  if (cfg->updated_mask & HAL_ISP_CALIBDB_GOC_MASK) {
    if (cfg->enabled[HAL_ISP_CALIBDB_GOC_ID] && cfg->calibdb_goc_cfg) {
      mCalibdbGocNeededUpdate = BOOL_TRUE;
      mCalibdbGocEnabled = HAL_ISP_ACTIVE_SETTING;
      calibdb_goc = *cfg->calibdb_goc_cfg;
    } else if (!cfg->enabled[HAL_ISP_CALIBDB_GOC_ID]) {
      mCalibdbGocNeededUpdate = BOOL_TRUE;
      mCalibdbGocEnabled = HAL_ISP_ACTIVE_FALSE;
      calibdb_goc = *cfg->calibdb_goc_cfg;
    } else if (cfg->enabled[HAL_ISP_CALIBDB_GOC_ID]) {
      mCalibdbGocNeededUpdate = BOOL_TRUE;
      mCalibdbGocEnabled = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      mCalibdbGocNeededUpdate = BOOL_FALSE;
      ALOGE("%s:can't config Calibdb Goc!", __func__);
      goto config_end;
    }
  }

  if (cfg->updated_mask & HAL_ISP_CALIBDB_CPROC_MASK) {
    if (cfg->enabled[HAL_ISP_CALIBDB_CPROC_ID] && cfg->calibdb_cproc_cfg) {
      mCalibdbCprocNeededUpdate = BOOL_TRUE;
      mCalibdbCprocEnabled = HAL_ISP_ACTIVE_SETTING;
      calibdb_cproc = *cfg->calibdb_cproc_cfg;
    } else if (!cfg->enabled[HAL_ISP_CALIBDB_CPROC_ID]) {
      mCalibdbCprocNeededUpdate = BOOL_TRUE;
      mCalibdbCprocEnabled = HAL_ISP_ACTIVE_FALSE;
      calibdb_cproc = *cfg->calibdb_cproc_cfg;
    } else if (cfg->enabled[HAL_ISP_CALIBDB_CPROC_ID]) {
      mCalibdbCprocNeededUpdate = BOOL_TRUE;
      mCalibdbCprocEnabled = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      mCalibdbCprocNeededUpdate = BOOL_FALSE;
      ALOGE("%s:can't config Calibdb Cproc!", __func__);
      goto config_end;
    }
  }

  /* should reconfig 3A algorithm ?*/
config_end:
  osMutexUnlock(&mApiLock);
  return ret;
}

bool CamIsp1xCtrItf::getIspConfig(enum HAL_ISP_SUB_MODULE_ID_e mod_id,
      bool_t& enabled, CamIspCtrItf::Configuration& cfg) {
  return true;
}

bool CamIsp1xCtrItf::start(bool run_3a_thd) {
  bool ret = true;
  //should call this after camera stream on, or driver will return error
  if (mCamHwItf) {
    if (mCamHwItf->isSupportedIrCut() >= 0)
      mSupportedSubDevs |= SUBDEV_IRCUT_MASK;
  }
  osMutexLock(&mApiLock);
  if (!mInitialized)
    goto end;
  if (++mStartCnt > 1)
    goto end;

  if (!startMeasurements()) {
    ALOGE("%s failed to start measurements", __func__);
    --mStartCnt;
    ret = false;
    goto end;
  }

  mLenPos = -1;
  mAeStableCnt = 0;
  mRun3AThd = false;

  if (run_3a_thd) {
    if (RET_SUCCESS != mISP3AThread->run("ISP3ATh", OSLAYER_THREAD_PRIO_HIGH)) {
      ALOGE("%s: ISP3ATh thread start failed", __func__);
      stopMeasurements();
      --mStartCnt;
      ret = false;
    }

    if (ret)
      mRun3AThd = true;
  }
end:
  osMutexUnlock(&mApiLock);
  return ret;
}
bool CamIsp1xCtrItf::stop() {
  bool ret = true;

  osMutexLock(&mApiLock);
  if (!mInitialized)
    goto end;
  if ((!mStartCnt) || ((mStartCnt > 0) && (--mStartCnt)))
    goto end;
  osMutexUnlock(&mApiLock);
  if (mRun3AThd)
    mISP3AThread->requestExitAndWait();
  osMutexLock(&mApiLock);
  stopMeasurements();

end:
  osMutexUnlock(&mApiLock);
  return ret;
}
/*
bool CamIsp1xCtrItf::awbConfig(struct HAL_AwbCfg *cfg)
{
  return true;
}
bool CamIsp1xCtrItf::aecConfig(struct HAL_AecCfg *cfg)
{
  return true;
}
bool CamIsp1xCtrItf::afcConfig(struct HAL_AfcCfg *cfg)
{
  return true;
}
bool CamIsp1xCtrItf::ispFunLock(unsigned int fun_lock)
{
  return true;
}
bool CamIsp1xCtrItf::ispFunEnable(unsigned int fun_en)
{
  return true;
}
bool CamIsp1xCtrItf::ispFunDisable(unsigned int fun_dis)
{
  return true;
}
bool CamIsp1xCtrItf::getIspFunStats(unsigned int  *lock, unsigned int *enable)
{
  return true;
}
*/

bool CamIsp1xCtrItf::initISPStream(const char* ispDev) {
  struct v4l2_requestbuffers req;
  struct v4l2_buffer v4l2_buf;

  mIspFd = open(ispDev, O_RDWR | O_NONBLOCK);
  if (mIspFd < 0) {
    ALOGE("%s: Cannot open %s (error : %s)\n",
          __func__,
          ispDev,
          strerror(errno));
    return false;
  }

  req.count = CAM_ISP_NUM_OF_STAT_BUFS;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;
  if (ioctl(mIspFd, VIDIOC_REQBUFS, &req) < 0) {
    ALOGE("%s: VIDIOC_REQBUFS failed, strerror: %s",
          __func__,
          strerror(errno));
    return false;
  }

  v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  v4l2_buf.memory = V4L2_MEMORY_MMAP;
  for (int i = 0; i < req.count; i++) {
    v4l2_buf.index = i;
    if (ioctl(mIspFd, VIDIOC_QUERYBUF, &v4l2_buf) < 0) {
      ALOGE("%s: VIDIOC_QUERYBUF failed\n", __func__);
      return false;
    }

    mIspStatBuf[i] = mmap(0,
                          v4l2_buf.length,
                          PROT_READ | PROT_WRITE,
                          MAP_SHARED,
                          mIspFd,
                          v4l2_buf.m.offset);
    if (mIspStatBuf[i] == MAP_FAILED) {
      ALOGE("%s mmap() failed\n", __func__);
      return false;
    }

    if (ioctl(mIspFd, VIDIOC_QBUF, &v4l2_buf) < 0) {
      ALOGE("QBUF failed index %d", v4l2_buf.index);
      return false;
    }
  }

  mIspStatBufSize = v4l2_buf.length;
  return true;
}

bool CamIsp1xCtrItf::getMeasurement(struct v4l2_buffer& v4l2_buf) {
  int retrycount = 3, ret;
  struct pollfd fds[1];
  int timeout_ms = 3000;

  fds[0].fd = mIspFd;
  fds[0].events = POLLIN | POLLERR;

  while (retrycount > 0) {
    ret = poll(fds, 1, timeout_ms);
    if (ret <= 0) {
      ALOGE("%s: poll error, %s",
            __FUNCTION__,
            strerror(errno));
      return false;
    }

    if (fds[0].revents & POLLERR) {
      ALOGD("%s: POLLERR in isp node", __FUNCTION__);
      return false;
    }

    if (fds[0].revents & POLLIN) {
      v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      v4l2_buf.memory = V4L2_MEMORY_MMAP;

      if (ioctl(mIspFd, VIDIOC_DQBUF, &v4l2_buf) < 0) {
        ALOGD("%s: VIDIOC_DQBUF failed, retry count %d\n",
              __FUNCTION__,
              retrycount);
        retrycount--;
        continue;
      }
      TRACE_D(1, "%s:  VIDIOC_DQBUF v4l2_buf: %d",
              __func__,
              v4l2_buf.index);
      if (v4l2_buf.sequence == (uint32_t) - 1) {
        ALOGD("%s: sequence=-1 qbuf: %d", v4l2_buf.index);
        releaseMeasurement(&v4l2_buf);
      }

      return true;
    }
  }
  return false;
}
bool CamIsp1xCtrItf::releaseMeasurement(struct v4l2_buffer* v4l2_buf) {
  if (ioctl(mIspFd, VIDIOC_QBUF, v4l2_buf) < 0) {
    ALOGE("%s: QBUF failed", __func__);
    return false;
  }

  return true;

}
bool CamIsp1xCtrItf::stopMeasurements() {
  bool ret = false;
  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (ioctl(mIspFd, VIDIOC_STREAMOFF, &type) < 0) {
    ALOGE("%s: VIDIOC_STREAMON failed\n", __func__);
    return false;
  }

  mStreaming = false;
  return ret;
}
bool CamIsp1xCtrItf::startMeasurements() {
  int ret;
  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  //ALOGD("%s: mIspFd: %d",
  //  __func__,
  //  mIspFd);
  if (ret = ioctl(mIspFd, VIDIOC_STREAMON, &type) < 0) {
    ALOGE("%s: VIDIOC_STREAMON failed, %s\n", __func__,
          strerror(ret));
    return false;
  }

  mStreaming = true;
  return true;
}

bool CamIsp1xCtrItf::runISPManual(struct CamIA10_Results* ia_results, bool_t lock) {
  struct HAL_ISP_cfg_s  manCfg = {0};

  if (lock)
    osMutexLock(&mApiLock);

  /* TODO: following may conflict with AEC, now update always to override AEC settings*/
  if (mHstNeededUpdate) {
    manCfg.enabled[HAL_ISP_HST_ID] = mHstEnabled;
    if (mHstEnabled == HAL_ISP_ACTIVE_DEFAULT) {
      //controlled by aec default
      manCfg.updated_mask &= ~HAL_ISP_HST_MASK;
      mHstNeededUpdate = BOOL_FALSE;
    } else {
      manCfg.hst_cfg = &hst_cfg;
      manCfg.updated_mask |= HAL_ISP_HST_MASK;
    }
  }

  if (mAecNeededUpdate) {
    manCfg.enabled[HAL_ISP_AEC_ID] = mAecEnabled;
    if (mAecEnabled == HAL_ISP_ACTIVE_DEFAULT) {
      //controlled by aec default
      manCfg.updated_mask &= ~HAL_ISP_AEC_MASK;
      mAecNeededUpdate = BOOL_FALSE;
    } else {
      manCfg.aec_cfg = &aec_cfg;
      manCfg.updated_mask |= HAL_ISP_AEC_MASK;
    }
  }

  /* TODO: following may conflict with AWB, now update always to override AWB settings*/
  if (mLscNeededUpdate) {
    manCfg.enabled[HAL_ISP_LSC_ID] = mLscEnabled;
    if (mLscEnabled == HAL_ISP_ACTIVE_DEFAULT) {
      //controlled by awb default
      manCfg.updated_mask &= ~HAL_ISP_LSC_MASK;
      mLscNeededUpdate = BOOL_FALSE;
    } else {
      manCfg.lsc_cfg = &lsc_cfg;
      manCfg.updated_mask |= HAL_ISP_LSC_MASK;
    }
  }

  if (mAwbGainNeededUpdate) {
    manCfg.enabled[HAL_ISP_AWB_GAIN_ID] = mAwbEnabled;
    if (mAwbEnabled == HAL_ISP_ACTIVE_DEFAULT) {
      //controlled by awb default
      manCfg.updated_mask &= ~HAL_ISP_AWB_GAIN_MASK;
      mAwbGainNeededUpdate = BOOL_FALSE;
    } else {
      manCfg.awb_gain_cfg = &awb_gain_cfg;
      manCfg.updated_mask |= HAL_ISP_AWB_GAIN_MASK;
    }
  }

  if (mAwbMeNeededUpdate) {
    manCfg.enabled[HAL_ISP_AWB_MEAS_ID] = mAwbMeEnabled;
    if (mAwbMeEnabled == HAL_ISP_ACTIVE_DEFAULT) {
      //controlled by awb default
      manCfg.updated_mask &= ~HAL_ISP_AWB_MEAS_MASK;
      mAwbMeNeededUpdate = BOOL_FALSE;
    } else {
      manCfg.awb_cfg = &awb_cfg;
      manCfg.updated_mask |= HAL_ISP_AWB_MEAS_MASK;
      //if awb gain is set, awb measure should enable
      if ((mAwbMeEnabled == HAL_ISP_ACTIVE_FALSE) &&
          (mAwbEnabled == HAL_ISP_ACTIVE_SETTING)) {
        //controlled by awb default
        manCfg.updated_mask &= ~HAL_ISP_AWB_MEAS_ID;
        mAwbMeNeededUpdate = BOOL_FALSE;
      }
    }
  }

  if (mCtkNeededUpdate) {
    manCfg.enabled[HAL_ISP_CTK_ID] = mCtkEnabled;
    if (mCtkEnabled == HAL_ISP_ACTIVE_DEFAULT) {
      //controlled by awb default
      manCfg.updated_mask &= ~HAL_ISP_CTK_MASK;
      mCtkNeededUpdate = BOOL_FALSE;
    } else {
      manCfg.ctk_cfg = &ctk_cfg;
      manCfg.updated_mask |= HAL_ISP_CTK_MASK;
    }
  }

  /* TODO: following may conflict with AWB, now update always to override AWB settings*/
  if (mDpfNeededUpdate) {
    manCfg.enabled[HAL_ISP_DPF_ID] = mDpfEnabled;
    if (mDpfEnabled == HAL_ISP_ACTIVE_DEFAULT) {
      //controlled by adpf default
      manCfg.updated_mask &= ~HAL_ISP_DPF_MASK;
      mDpfNeededUpdate = BOOL_FALSE;
    } else {
      manCfg.dpf_cfg = &dpf_cfg;
      manCfg.updated_mask |= HAL_ISP_DPF_MASK;
    }
  }

  if (mDpfStrengthNeededUpdate) {
    manCfg.enabled[HAL_ISP_DPF_STRENGTH_ID] = mDpfStrengthEnabled;
    if (mDpfStrengthEnabled == HAL_ISP_ACTIVE_DEFAULT) {
      //controlled by adpf default
      manCfg.updated_mask &= ~HAL_ISP_DPF_STRENGTH_MASK;
      mDpfStrengthNeededUpdate = BOOL_FALSE;
    } else {
      manCfg.dpf_strength_cfg = &dpf_strength_cfg;
      manCfg.updated_mask |= HAL_ISP_DPF_STRENGTH_MASK;
    }
  }

  /* TODO: following may conflict with AFC, now update always to override AFC settings*/
  if (mAfcNeededUpdate) {
    manCfg.afc_cfg = &afc_cfg;
    manCfg.updated_mask |= HAL_ISP_AFC_MASK;
    manCfg.enabled[HAL_ISP_AFC_ID] = mAfcEnabled;
    //mAfcNeededUpdate= BOOL_FALSE;
  }

  if (mBlsNeededUpdate) {
    manCfg.bls_cfg = &bls_cfg;
    manCfg.updated_mask |= HAL_ISP_BLS_MASK;
    manCfg.enabled[HAL_ISP_BLS_ID] = mBlsEnabled;
    mBlsNeededUpdate = BOOL_FALSE;
  }

  if (mIeNeededUpdate) {
    manCfg.ie_cfg = &ie_cfg;
    manCfg.updated_mask |= HAL_ISP_IE_MASK;
    manCfg.enabled[HAL_ISP_IE_ID] = mIeEnabled;
    mIeNeededUpdate = BOOL_FALSE;
  }

  if (mDpccNeededUpdate) {
    manCfg.dpcc_cfg = &dpcc_cfg;
    manCfg.updated_mask |= HAL_ISP_BPC_MASK;
    manCfg.enabled[HAL_ISP_BPC_ID] = mDpccEnabled;
    mDpccNeededUpdate = BOOL_FALSE;
  }

  if (mSdgNeededUpdate) {
    manCfg.sdg_cfg = &sdg_cfg;
    manCfg.updated_mask |= HAL_ISP_SDG_MASK;
    manCfg.enabled[HAL_ISP_SDG_ID] = mSdgEnabled;
    mSdgNeededUpdate = BOOL_FALSE;
  }
  if (mFltNeededUpdate) {
    manCfg.flt_cfg = &flt_cfg;
    manCfg.updated_mask |= HAL_ISP_FLT_MASK;
    manCfg.enabled[HAL_ISP_FLT_ID] = mFltEnabled;
    mFltNeededUpdate = BOOL_FALSE;
  }

  if (mBdmNeededUpdate) {
    manCfg.bdm_cfg = &bdm_cfg;
    manCfg.updated_mask |= HAL_ISP_BDM_MASK;
    manCfg.enabled[HAL_ISP_BDM_ID] = mBdmEnabled;
    mBdmNeededUpdate = BOOL_FALSE;
  }

  if (mGocNeededUpdate) {
    manCfg.goc_cfg = &goc_cfg;
    manCfg.updated_mask |= HAL_ISP_GOC_MASK;
    manCfg.enabled[HAL_ISP_GOC_ID] = mGocEnabled;
    mGocNeededUpdate = BOOL_FALSE;
  }

  if (mCprocNeededUpdate) {
    manCfg.cproc_cfg = &cproc_cfg;
    manCfg.updated_mask |= HAL_ISP_CPROC_MASK;
    manCfg.enabled[HAL_ISP_CPROC_ID] = mCprocEnabled;
    mCprocNeededUpdate = BOOL_FALSE;
  }

  if (mWdrNeededUpdate) {
    manCfg.wdr_cfg = &wdr_cfg;
    manCfg.updated_mask |= HAL_ISP_WDR_MASK;
    manCfg.enabled[HAL_ISP_WDR_ID] = mWdrEnabled;
    mWdrNeededUpdate = BOOL_FALSE;
  }

  if (m3DnrNeededUpdate) {
    manCfg.dsp_3dnr_cfg = &dsp_3dnr_cfg;
    manCfg.updated_mask |= HAL_ISP_3DNR_MASK;
    manCfg.enabled[HAL_ISP_3DNR_ID] = m3DnrEnabled;
    m3DnrNeededUpdate = BOOL_FALSE;
  }

  /*new 3dnr */
  if (mNew3DnrNeededUpdate) {
    manCfg.newDsp3DNR_cfg = &new_dsp_3dnr_cfg;
    manCfg.updated_mask |= HAL_ISP_NEW_3DNR_MASK;
    manCfg.enabled[HAL_ISP_NEW_3DNR_ID] = mNew3DnrEnabled;
    mNew3DnrNeededUpdate = BOOL_FALSE;
  }

  if (mAwbLscNeededUpdate) {
    manCfg.awb_lsc_pfl = &awb_lsc_pfl;
    manCfg.updated_mask |= HAL_ISP_AWB_LSC_MASK;
    manCfg.enabled[HAL_ISP_AWB_LSC_ID] = mAwbLscEnabled;
    mAwbLscNeededUpdate = BOOL_FALSE;
  }

  if (mAdpfDpfNeededUpdate) {
    manCfg.adpf_dpf = &adpf_dpf;
    manCfg.updated_mask |= HAL_ISP_ADPF_DPF_MASK;
    manCfg.enabled[HAL_ISP_ADPF_DPF_ID] = mAdpfDpfEnabled;
    mAdpfDpfNeededUpdate = BOOL_FALSE;
  }

  if (mAdpfFltNeededUpdate) {
    manCfg.adpf_flt = &adpf_flt;
    manCfg.updated_mask |= HAL_ISP_ADPF_FLT_MASK;
    manCfg.enabled[HAL_ISP_ADPF_FLT_ID] = mAdpfFltEnabled;
    mAdpfFltNeededUpdate = BOOL_FALSE;
  }

  if (mAwbCcmNeededUpdate) {
    manCfg.awb_ccm_pfl = &awb_ccm_pfl;
    manCfg.updated_mask |= HAL_ISP_AWB_CCM_MASK;
    manCfg.enabled[HAL_ISP_AWB_CCM_ID] = mAwbCcmEnabled;
    mAwbCcmNeededUpdate = BOOL_FALSE;
  }

  if (mAwbGainIlluNeededUpdate) {
    manCfg.awb_gain_illu_pfl = &awb_gain_illu_pfl;
    manCfg.updated_mask |= HAL_ISP_AWB_GAIN_ILLU_MASK;
    manCfg.enabled[HAL_ISP_AWB_GAIN_ILLU_ID] = mAwbGainIlluEnabled;
    mAwbGainIlluNeededUpdate = BOOL_FALSE;
  }

  if (mAwbRefGainNeededUpdate) {
    manCfg.awb_refgain = &awb_refgain;
    manCfg.updated_mask |= HAL_ISP_AWB_REFGAIN_MASK;
    manCfg.enabled[HAL_ISP_AWB_REFGAIN_ID] = mAwbRefGainEnabled;
    mAwbRefGainNeededUpdate = BOOL_FALSE;
  }

  if (mAwbCurveNeededUpdate) {
    manCfg.awb_curve = &awb_curve;
    manCfg.updated_mask |= HAL_ISP_AWB_CURVE_MASK;
    manCfg.enabled[HAL_ISP_AWB_CURVE_ID] = mAwbCurveEnabled;
    mAwbCurveNeededUpdate = BOOL_FALSE;
  }

  if (mAwbWpSetNeededUpdate) {
    manCfg.awb_wp_set = &awb_wp_set;
    manCfg.updated_mask |= HAL_ISP_AWB_WP_SET_MASK;
    manCfg.enabled[HAL_ISP_AWB_WP_SET_ID] = mAwbWpSetEnabled;
    mAwbWpSetNeededUpdate = BOOL_FALSE;
  }

  if (mCalibdbGocNeededUpdate) {
    manCfg.calibdb_goc_cfg = &calibdb_goc;
    manCfg.updated_mask |= HAL_ISP_CALIBDB_GOC_MASK;
    manCfg.enabled[HAL_ISP_CALIBDB_GOC_ID] = mCalibdbGocEnabled;
    mCalibdbGocNeededUpdate = BOOL_FALSE;
  }

  if (mCalibdbCprocNeededUpdate) {
    manCfg.calibdb_cproc_cfg = &calibdb_cproc;
    manCfg.cproc_cfg = &cproc_cfg;
    manCfg.updated_mask |= HAL_ISP_CALIBDB_CPROC_MASK;
    manCfg.enabled[HAL_ISP_CALIBDB_CPROC_ID] = mCalibdbCprocEnabled;
    mCalibdbCprocNeededUpdate = BOOL_FALSE;
  }
  
  if (lock)
    osMutexUnlock(&mApiLock);
  if (mCamIAEngine.get() &&
     (RET_SUCCESS == mCamIAEngine->runManISP(&manCfg, ia_results))) {
    return BOOL_TRUE;
  } else {
    return BOOL_FALSE;
  }
}

void CamIsp1xCtrItf::transDrvMetaDataToHal
(
    const void* drvMeta,
    struct HAL_Buffer_MetaData* halMeta
) {
  return;
}

void CamIsp1xCtrItf::mapSensorExpToHal
(
    int sensorGain,
    int sensorInttime,
    float& halGain,
    float& halInttime
) {
  mCamIAEngine->mapSensorExpToHal(sensorGain, sensorInttime, halGain, halInttime);
}

int CamIsp1xCtrItf::switchSubDevIrCutMode(int mode) {
#if 1
  if (!mCamHwItf || ((mSupportedSubDevs & SUBDEV_IRCUT_MASK) == 0))
    return 0;
  int ircut_state = mCamHwItf->getIrCutState();
  if ((ircut_state < 0) ||
      (((mode > 0) && (ircut_state > 0)) ||
       ((mode == 0) && (ircut_state == 0)))) {
    return 0;
  }
#else
  static int last_mode = 1;
  if (mode != last_mode)
    last_mode = mode;
  else
    return 0;
#endif
  struct HAL_ISP_cfg_s cfg;
  struct HAL_ISP_ie_cfg_s ie_cfg;
  struct HAL_ISP_goc_cfg_s goc_cfg;
  memset(&cfg, 0, sizeof(struct HAL_ISP_cfg_s));
  cfg.updated_mask = 0;
  ie_cfg.range = HAL_ISP_COLOR_RANGE_OUT_BT601;
  cfg.ie_cfg = &ie_cfg;
  cfg.goc_cfg = &goc_cfg;
  if (mode == 0) {
    //stop awb
    //here HAL_WB_DAYLIGHT means awb stopped
	mLastWbMode = mCamIA_DyCfg.awb_cfg.mode;
    mCamIA_DyCfg.awb_cfg.mode = HAL_WB_INVAL;
    cfg.updated_mask |= HAL_ISP_CTK_MASK;
    cfg.enabled[HAL_ISP_CTK_ID] = HAL_ISP_ACTIVE_FALSE;
    cfg.updated_mask |= HAL_ISP_AWB_GAIN_MASK;
    cfg.enabled[HAL_ISP_AWB_GAIN_ID] = HAL_ISP_ACTIVE_FALSE;
    //set ie mode to mono
    osMutexLock(&mApiLock);
    ie_cfg.mode = HAL_EFFECT_MONO;
    cfg.updated_mask |= HAL_ISP_IE_MASK;
    cfg.enabled[HAL_ISP_IE_ID] = HAL_ISP_ACTIVE_SETTING;
    mCamIA_DyCfg.ie_mode = HAL_EFFECT_MONO;
    osMutexUnlock(&mApiLock);
    //disable wdr
    cfg.updated_mask |= HAL_ISP_WDR_MASK;
    cfg.enabled[HAL_ISP_WDR_ID] = HAL_ISP_ACTIVE_FALSE;
    //disable lsc
    cfg.updated_mask |= HAL_ISP_LSC_MASK;
    cfg.enabled[HAL_ISP_LSC_ID] = HAL_ISP_ACTIVE_FALSE;
	//set gamma using wdr off gammay
    cfg.updated_mask |= HAL_ISP_GOC_MASK;
    cfg.enabled[HAL_ISP_GOC_ID] = HAL_ISP_ACTIVE_DEFAULT;
	mCamIA_DyCfg.LightMode = LIGHT_MODE_NIGHT;
	goc_cfg.light_mode = mCamIA_DyCfg.LightMode;
	
    //close ircut
    mCamHwItf->setIrCutState(0);
	
  } else {
    //start awb
    mCamIA_DyCfg.awb_cfg.mode = mLastWbMode;
    cfg.updated_mask |= HAL_ISP_CTK_MASK;
    cfg.enabled[HAL_ISP_CTK_ID] = HAL_ISP_ACTIVE_DEFAULT;
    cfg.updated_mask |= HAL_ISP_AWB_GAIN_MASK;
    cfg.enabled[HAL_ISP_AWB_GAIN_ID] = HAL_ISP_ACTIVE_DEFAULT;
	cfg.updated_mask |= HAL_ISP_AWB_MEAS_MASK;
    cfg.enabled[HAL_ISP_AWB_MEAS_ID] = HAL_ISP_ACTIVE_DEFAULT;
    //set ie mode to normal
    osMutexLock(&mApiLock);
    ie_cfg.mode = HAL_EFFECT_NONE;
    cfg.updated_mask |= HAL_ISP_IE_MASK;
    cfg.enabled[HAL_ISP_IE_ID] = HAL_ISP_ACTIVE_FALSE;
    mCamIA_DyCfg.ie_mode = HAL_EFFECT_NONE;
    osMutexUnlock(&mApiLock);
    //enable wdr
    cfg.updated_mask |= HAL_ISP_WDR_MASK;
    cfg.enabled[HAL_ISP_WDR_ID] = HAL_ISP_ACTIVE_DEFAULT;
    //enable lsc,now this will cause fliker
    cfg.updated_mask |= HAL_ISP_LSC_MASK;
    cfg.enabled[HAL_ISP_LSC_ID] = HAL_ISP_ACTIVE_DEFAULT;
	//set gamma using wdron gammaY
    cfg.updated_mask |= HAL_ISP_GOC_MASK;
    cfg.enabled[HAL_ISP_GOC_ID] = HAL_ISP_ACTIVE_DEFAULT;
	mCamIA_DyCfg.LightMode = LIGHT_MODE_DAY;
	goc_cfg.light_mode = mCamIA_DyCfg.LightMode;
    //open ircut
    mCamHwItf->setIrCutState(1);	
  }

  configureISP(&cfg);
  return 0;
}

bool CamIsp1xCtrItf::runIA(struct CamIA10_DyCfg* ia_dcfg,
                           struct CamIA10_Stats* ia_stats,
                           struct CamIA10_Results* ia_results) {
  if (ia_dcfg)
    mCamIAEngine->initDynamic(ia_dcfg);

  if (ia_stats) {
    mCamIAEngine->setStatistics(ia_stats);

    if (ia_stats->meas_type & CAMIA10_AEC_MASK) {
      mCamIAEngine->runAEC();
      mCamIAEngine->runADPF();
      mCamIAEngine->runAWDR();
    }

    if (ia_stats->meas_type & CAMIA10_AWB_MEAS_MASK) {
      mCamIAEngine->runAWB();
    }

    if (ia_stats->meas_type & CAMIA10_AFC_MASK) {
      mCamIAEngine->runAF();
    }
  }

  if (ia_results) {
    ia_results->active = 0;
    if (mCamIAEngine->getAECResults(&ia_results->aec) == RET_SUCCESS) {
      ia_results->active |= CAMIA10_AEC_MASK;
      ia_results->hst.enabled = BOOL_TRUE;
      //copy aec hst result to struct hst, may be override by manual settings after
      ia_results->hst.mode = CAMERIC_ISP_HIST_MODE_RGB_COMBINED;
      ia_results->hst.Window.width =
          ia_results->aec.meas_win.h_size;
      ia_results->hst.Window.height =
          ia_results->aec.meas_win.v_size;
      ia_results->hst.Window.hOffset =
          ia_results->aec.meas_win.h_offs;
      ia_results->hst.Window.vOffset =
          ia_results->aec.meas_win.v_offs;
      ia_results->hst.StepSize =
          ia_results->aec.stepSize;

      if (ia_results->aec.aoe_enable) {
        ia_results->hst.Weights[0] = 0;
        ia_results->hst.Weights[1] = 0;
        ia_results->hst.Weights[2] = 0;
        ia_results->hst.Weights[3] = 0;
        ia_results->hst.Weights[4] = 0;

        ia_results->hst.Weights[5] = 0;
        ia_results->hst.Weights[6] = 0;
        ia_results->hst.Weights[7] = 10;
        ia_results->hst.Weights[8] = 0;
        ia_results->hst.Weights[9] = 0;

        ia_results->hst.Weights[10] = 0;
        ia_results->hst.Weights[11] = 10;
        ia_results->hst.Weights[12] = 10;
        ia_results->hst.Weights[13] = 10;
        ia_results->hst.Weights[14] = 0;

        ia_results->hst.Weights[15] = 0;
        ia_results->hst.Weights[16] = 0;
        ia_results->hst.Weights[17] = 10;
        ia_results->hst.Weights[18] = 0;
        ia_results->hst.Weights[19] = 0;

        ia_results->hst.Weights[20] = 0;
        ia_results->hst.Weights[21] = 0;
        ia_results->hst.Weights[22] = 0;
        ia_results->hst.Weights[23] = 0;
        ia_results->hst.Weights[24] = 0;
      } else {
        memcpy(ia_results->hst.Weights,
               ia_results->aec.GridWeights, sizeof(ia_results->aec.GridWeights));
      }


      ia_results->aec_enabled = BOOL_TRUE;
    }

    memset(&ia_results->awb, 0, sizeof(ia_results->awb));
    if (mCamIAEngine->getAWBResults(&ia_results->awb) == RET_SUCCESS) {
      if (ia_results->awb.actives & AWB_RECONFIG_GAINS)
        ia_results->active |= CAMIA10_AWB_GAIN_MASK;
      if ((ia_results->awb.actives & AWB_RECONFIG_CCMATRIX)
          || (ia_results->awb.actives & AWB_RECONFIG_CCOFFSET))
        ia_results->active |= CAMIA10_CTK_MASK;
      if ((ia_results->awb.actives & AWB_RECONFIG_LSCMATRIX)
          || (ia_results->awb.actives & AWB_RECONFIG_LSCSECTOR))
        ia_results->active |= CAMIA10_LSC_MASK;
      if ((ia_results->awb.actives & AWB_RECONFIG_MEASMODE)
          || (ia_results->awb.actives & AWB_RECONFIG_MEASCFG)
          || (ia_results->awb.actives & AWB_RECONFIG_AWBWIN))
        ia_results->active |= CAMIA10_AWB_MEAS_MASK;
      ia_results->awb_gains_enabled = BOOL_TRUE;
      ia_results->awb_meas_enabled = BOOL_TRUE;
      ia_results->lsc_enabled = BOOL_TRUE;
      ia_results->ctk_enabled = BOOL_TRUE;

      if (mAwbLscEnabled == HAL_ISP_ACTIVE_FALSE) {
        ia_results->active &= ~CAMIA10_LSC_MASK;
        ia_results->lsc_enabled = BOOL_FALSE;
      }

      if (mAwbCcmEnabled == HAL_ISP_ACTIVE_FALSE) {
        ia_results->active &= ~CAMIA10_CTK_MASK;
        ia_results->ctk_enabled = BOOL_FALSE;
      }
    }

    if (mCamIAEngine->getADPFResults(&ia_results->adpf) == RET_SUCCESS) {
      if (ia_results->adpf.actives & ADPF_MASK){
        ia_results->active |= CAMIA10_DPF_MASK;
		ia_results->adpf_enabled = BOOL_TRUE;
      }
      if (ia_results->adpf.actives & ADPF_STRENGTH_MASK){
        ia_results->active |= CAMIA10_DPF_STRENGTH_MASK;    
      	ia_results->adpf_strength_enabled = BOOL_TRUE;
      }

      if ((ia_results->adpf.actives & ADPF_DENOISE_SHARP_LEVEL_MASK) &&
        (ia_dcfg->flt_mode == HAL_MODE_AUTO)) {
        flt_cfg.denoise_level = ia_results->adpf.denoise_level;
        flt_cfg.sharp_level = ia_results->adpf.sharp_level;
		flt_cfg.light_mode = mCamIA_DyCfg.LightMode;
        mFltEnabled = HAL_ISP_ACTIVE_DEFAULT;
        mFltNeededUpdate = BOOL_TRUE;
        runISPManual(ia_results, BOOL_FALSE);
        ia_results->flt.enabled = ia_results->adpf.FltEnable;
        ia_results->active |= CAMIA10_FLT_MASK;
      }

	  if (ia_results->adpf.actives & ADPF_DEMOSAIC_TH_MASK) {
        bdm_cfg.demosaic_th = ia_results->adpf.demosaic_th;
        mBdmEnabled = HAL_ISP_ACTIVE_SETTING;
        mBdmNeededUpdate = BOOL_TRUE;
        runISPManual(ia_results, BOOL_FALSE);
        ia_results->bdm.enabled = BOOL_TRUE;
        ia_results->active |= CAMIA10_BDM_MASK;
      }

      if ((ia_results->adpf.actives & ADPF_DSP_3DNR_MASK) &&
        (m3DnrEnabled == HAL_ISP_ACTIVE_DEFAULT)) {
        ia_results->active |= CAMIA10_DSP_3DNR_MASK;
      }

	  if ((ia_results->adpf.actives & ADPF_NEW_DSP_3DNR_MASK) &&
        (mNew3DnrEnabled == HAL_ISP_ACTIVE_DEFAULT)) {
        ia_results->active |= CAMIA10_NEW_DSP_3DNR_MASK;
      }
    }

    if (mCamIAEngine->getAWDRResults(&ia_results->awdr) == RET_SUCCESS) {
      if ((ia_results->awdr.actives & AWDR_WDR_MAXGAIN_LEVEL_MASK) &&
        (ia_dcfg->wdr_cfg.mode == HAL_MODE_AUTO)) {
        wdr_cfg.wdr_gain_max_value = ia_results->awdr.Wdr_MaxGain_level_RegValue;
        mWdrEnabled = HAL_ISP_ACTIVE_DEFAULT;
        mWdrNeededUpdate = BOOL_TRUE;
        runISPManual(ia_results, BOOL_FALSE);
        ia_results->wdr.wdr_gain_max_value = wdr_cfg.wdr_gain_max_value;
        ia_results->wdr.enabled = BOOL_TRUE;
        ia_results->active |= CAMIA10_WDR_MASK;		
      }
    }

    if (mCamIAEngine->getAFResults(&ia_results->af) == RET_SUCCESS) {
      ia_results->active |= CAMIA10_AFC_MASK;
      ia_results->afc_meas_enabled = BOOL_TRUE;
    }
  }

}

void CamIsp1xCtrItf::mapHalExpToSensor(float hal_gain, float hal_time, int& sensor_gain, int& sensor_time) {
  if (mCamIAEngine.get())
      mCamIAEngine->mapHalExpToSensor(hal_gain, hal_time, sensor_gain, sensor_time);
  return;
}

