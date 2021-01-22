#include <ebase/types.h>
#include <ebase/trace.h>
#include <ebase/builtins.h>
#include <sys/ioctl.h>
#include <calib_xml/calibdb.h>
#include <utils/Log.h>
#include "CamHalVersion.h"
#ifdef RK_ISP10
#include "CamIsp10CtrItf.h"
#endif
#ifdef RK_ISP11
#include "CamIsp11CtrItf.h"
#endif
#include "camapi_interface.h"


#ifndef __cplusplus
#define __cplusplus
#endif

using namespace std;

void CamApiItf::initApiItf(CamIspCtrItf* ispDev) {

  //ALOGD("%s: E", __func__);
  ALOGD("CAMHALVERSION is: %s\n", CAMHALVERSION);

  mIspDev = ispDev;
  memset(&mIspCfg, 0, sizeof(mIspCfg));
  osMutexInit(&mApiLock);
  mISPBrightness = -120;
  mISPContrast = -200.0f;
  mISPHue = -200.0f;
  mISPSaturation = -200.0f;
  mAfMode = HAL_AF_MODE_NOT_SET;
  mAfTrigger = BOOL_FALSE;
  mAfSupport = BOOL_FALSE;
  memset(&mAfWin, 0, sizeof(mAfWin));

  mCurAeMode = HAL_AE_OPERATION_MODE_AUTO;
  mCurAeBias = 0;
  mCurAbsExp = -1;
  mCurAeMeterMode = HAL_AE_METERING_MODE_CENTER;
  mFlkMode = HAL_AE_FLK_AUTO;
  m_wb_mode = HAL_WB_AUTO;
  m3ALocks = HAL_3A_LOCKS_NONE;
  mSceneMode = HAL_SCENE_MODE_AUTO;
  mIsoMode = HAL_ISO_MODE_AUTO;
  m_image_effect = HAL_EFFECT_NONE;
  mZoom = 0;
  mBrightness = 0;
  mContrast = 0;
  mSaturation = 0;
  mHue = 0;
  mFlip = HAL_FLIP_NONE;
  m_fps.numerator = 0;
  m_fps.denominator = 0;
  memset(&mAeWin, 0, sizeof(mAeWin));
  memset(mAeMeterCoeff, 0, sizeof(mAeMeterCoeff));
  mAeSetPoint = 0;
  mNightMode = true;
  mDayNightSwitch = HAL_DAYNIGHT_AUTO;
  mAEMeanLuma = 0;
  mAEMaxExposureTime = 0.0f;
  mAEMaxExposureGain = 0.0f;
  mAEMaxGainRange = 0.0f;
  memset(&mWdr, 0, sizeof(mWdr));
  mWdr.mode = HAL_MODE_AUTO;
  m3DnrMode = HAL_MODE_AUTO;
  mFltMode = HAL_MODE_AUTO;
  mBlsMode = HAL_MODE_AUTO;
  memset(&mBlsCfg, 0, sizeof(mBlsCfg));
  mGocMode = HAL_MODE_AUTO;
  memset(&mGocCfg, 0, sizeof(mGocCfg));
  mCprocMode = HAL_MODE_AUTO;
  memset(&mCprocCfg, 0, sizeof(mCprocCfg));
  mAwbLscMode = HAL_MODE_AUTO;
  memset(&mAwbLscPfl, 0, sizeof(mAwbLscPfl));
  memset(&mAdpfDpf, 0, sizeof(mAdpfDpf));
  memset(&mAdpfFlt, 0, sizeof(mAdpfFlt));
  mAwbCcmMode = HAL_MODE_AUTO;
  memset(&mAwbCcmPfl, 0, sizeof(mAwbCcmPfl));
  mAwbGainIlluMode = HAL_MODE_AUTO;
  memset(&mAwbGainIlluPfl, 0, sizeof(mAwbGainIlluPfl));
  memset(&mAwbRefGain, 0, sizeof(mAwbRefGain));
  memset(&mAwbCurve, 0, sizeof(mAwbCurve));
  memset(&mAwbWpSet, 0, sizeof(mAwbWpSet));
  mAEManualExpTime = 0.0f;
  mAEManualExpGain = 0.0f;
  mAeState = HAL_AE_STATE_UNSTABLE;
  //ALOGD("%s: X", __func__);
}

int CamApiItf::configIsp_l(struct isp_supplemental_sensor_mode_data* sensor) {
  CamIspCtrItf::Configuration cfg;
  cfg = mIspCfg;
  /*config sensor mode data*/
  if (sensor && (
      (sensor->isp_input_width != mIspCfg.sensor_mode.isp_input_width) ||
      (sensor->isp_input_height != mIspCfg.sensor_mode.isp_input_height) ||
      (sensor->vt_pix_clk_freq_hz / 1000000.0f != mIspCfg.sensor_mode.pixel_clock_freq_mhz) ||
      (sensor->crop_horizontal_start != mIspCfg.sensor_mode.horizontal_crop_offset) ||
      (sensor->crop_vertical_start != mIspCfg.sensor_mode.vertical_crop_offset) ||
      (sensor->crop_horizontal_end - sensor->crop_horizontal_start + 1 != mIspCfg.sensor_mode.cropped_image_width) ||
      (sensor->crop_vertical_end - sensor->crop_vertical_start + 1 != mIspCfg.sensor_mode.cropped_image_height) ||
      (sensor->line_length_pck != mIspCfg.sensor_mode.pixel_periods_per_line) ||
      (sensor->frame_length_lines != mIspCfg.sensor_mode.line_periods_per_field) ||
      (sensor->sensor_output_height != mIspCfg.sensor_mode.sensor_output_height) ||
      (sensor->fine_integration_time_min != mIspCfg.sensor_mode.fine_integration_time_min)  ||
      (sensor->line_length_pck - sensor->fine_integration_time_max_margin != mIspCfg.sensor_mode.fine_integration_time_max_margin) ||
      (sensor->coarse_integration_time_min != mIspCfg.sensor_mode.coarse_integration_time_min)  ||
      (sensor->coarse_integration_time_max_margin != mIspCfg.sensor_mode.coarse_integration_time_max_margin) ||
      (sensor->gain != mIspCfg.sensor_mode.gain) ||
      (sensor->exp_time != mIspCfg.sensor_mode.exp_time) ||
      (sensor->exposure_valid_frame[0] != mIspCfg.sensor_mode.exposure_valid_frame))) {


    cfg.sensor_mode.isp_input_width = sensor->isp_input_width;
    cfg.sensor_mode.isp_input_height = sensor->isp_input_height;
    cfg.sensor_mode.isp_output_width = sensor->isp_output_width;
    cfg.sensor_mode.isp_output_height = sensor->isp_output_height;
    cfg.sensor_mode.pixel_clock_freq_mhz = sensor->vt_pix_clk_freq_hz / 1000000.0f;
    cfg.sensor_mode.horizontal_crop_offset = sensor->crop_horizontal_start;
    cfg.sensor_mode.vertical_crop_offset = sensor->crop_vertical_start;
    cfg.sensor_mode.cropped_image_width = sensor->crop_horizontal_end - sensor->crop_horizontal_start + 1;
    cfg.sensor_mode.cropped_image_height = sensor->crop_vertical_end - sensor->crop_vertical_start + 1;
    cfg.sensor_mode.pixel_periods_per_line =  sensor->line_length_pck;
    cfg.sensor_mode.line_periods_per_field = sensor->frame_length_lines;
    cfg.sensor_mode.sensor_output_height = sensor->sensor_output_height;
    cfg.sensor_mode.fine_integration_time_min = sensor->fine_integration_time_min;
    cfg.sensor_mode.fine_integration_time_max_margin = sensor->line_length_pck - sensor->fine_integration_time_max_margin;
    cfg.sensor_mode.coarse_integration_time_min = sensor->coarse_integration_time_min;
    cfg.sensor_mode.coarse_integration_time_max_margin = sensor->coarse_integration_time_max_margin;
    cfg.sensor_mode.gain = sensor->gain;
    cfg.sensor_mode.exp_time = sensor->exp_time;
    cfg.sensor_mode.exposure_valid_frame = sensor->exposure_valid_frame[0];
  }

  /*config controls*/
  cfg.uc = UC_PREVIEW;
  if (sensor && mAfSupport) {
    mAfMode = HAL_AF_MODE_CONTINUOUS_VIDEO;
  }
  cfg.aaa_locks = m3ALocks;
  cfg.aec_cfg.flk = mFlkMode;
  cfg.aec_cfg.mode = mCurAeMode;
  cfg.aec_cfg.meter_mode = mCurAeMeterMode;
  cfg.aec_cfg.ae_bias = mCurAeBias;
  if (m_fps.numerator != 0)
    cfg.aec_cfg.api_set_fps = m_fps.denominator / m_fps.numerator;
  else
    cfg.aec_cfg.api_set_fps = 0;
  cfg.aec_cfg.win = mAeWin;
  memcpy(cfg.aec_cfg.meter_coeff, mAeMeterCoeff, sizeof(mAeMeterCoeff));
  cfg.aec_cfg.set_point = mAeSetPoint;
  cfg.aec_cfg.aeMaxExpTime = mAEMaxExposureTime;
  cfg.aec_cfg.aeMaxExpGain = mAEMaxExposureGain;
  //cfg.aec_cfg.win = ;
  cfg.afc_cfg.win_a = mAfWin;
  cfg.afc_cfg.mode = mAfMode;
  cfg.afc_cfg.oneshot_trigger = mAfTrigger;
  cfg.afc_cfg.win_num = 1;
  cfg.afc_cfg.type.contrast_af = 1;
  mAfTrigger = BOOL_FALSE;
  //cfg.afc_cfg.win = ;
  cfg.awb_cfg.mode = m_wb_mode;
  //cfg.awb_cfg.win = ;
  cfg.cproc.brightness = mISPBrightness;
  cfg.cproc.contrast = mISPContrast;
  cfg.cproc.hue = mISPHue;
  cfg.cproc.saturation = mISPSaturation;
  //cfg.cproc.sharpness = ;
  cfg.flash_mode = mFlMode;
  cfg.ie_mode = m_image_effect;
  cfg.wdr_cfg = mWdr;
  cfg.dsp3dnr_mode = m3DnrMode;
  cfg.dsp3dnr_level = m3DnrLevel;
  cfg.dsp3dnr_param = m3DnrParam;
  cfg.flt_mode = mFltMode;
  cfg.flt_denoise = mDenoise;
  cfg.flt_sharp = mSharp;

  //TODO: ae bias,zoom,rotation,3a areas
  cfg.bls_mode = mBlsMode;
  cfg.bls_cfg = mBlsCfg;
  cfg.goc_mode = mGocMode;
  cfg.api_goc_cfg = mGocCfg;
  cfg.cproc_mode = mCprocMode;
  cfg.api_cproc_cfg = mCprocCfg;
  cfg.awb_lsc_mode = mAwbLscMode;
  cfg.awb_lsc_pfl = mAwbLscPfl;
  cfg.adpf_dpf_cfg = mAdpfDpf;
  cfg.adpf_flt_cfg = mAdpfFlt;
  cfg.awb_ccm_mode = mAwbCcmMode;
  cfg.awb_ccm_pfl = mAwbCcmPfl;
  cfg.awb_gain_illu_mode = mAwbGainIlluMode;
  cfg.awb_gain_illu_pfl = mAwbGainIlluPfl;
  cfg.awb_refgain = mAwbRefGain;
  cfg.awb_curve = mAwbCurve;
  cfg.awb_wp_set = mAwbWpSet;
  if (!mIspDev->configure(cfg)) {
    ALOGE("%s: mISPDev->configure failed!",
          __func__);
  }

  mIspCfg = cfg;
}

int CamApiItf::getAeTime(float &time)
{
  float ae_time;
  float ae_gain;
  bool ret;

  if (mCurAeMode != HAL_AE_OPERATION_MODE_MANUAL) {
    ret = mIspDev->getTimeAndGain(&ae_time, &ae_gain);
    if (!ret) {
      ALOGE("%s: getTimeAndGain failed!",
          __func__);
      return -1;
    }
  } else {
    ae_time = mAEManualExpTime;
  }

  time = ae_time;
  return 0;
}

int CamApiItf::getAeGain(float &gain)
{
  float ae_time;
  float ae_gain;
  bool ret;

  if (mCurAeMode != HAL_AE_OPERATION_MODE_MANUAL) {
    ret = mIspDev->getTimeAndGain(&ae_time, &ae_gain);
    if (!ret) {
      ALOGE("%s: getTimeAndGain failed!",
            __func__);
      return -1;
    }
  } else {
    ae_gain = mAEManualExpGain;
  }

  gain = ae_gain;
  return 0;
}

int CamApiItf::GetAeMaxExposureTime(float &time)
{
  float max_time;
  float min_time;
  bool ret;

  ret = mIspDev->getMaxMinTime(&max_time, &min_time);
  if (!ret) {
    ALOGE("%s: getTimeAndGain failed!",
          __func__);
    return -1;
  }

  time = max_time;
  return 0;
}

int CamApiItf::GetAeMaxExposureGain(float &gain)
{
  float max_gain;
  float min_gain;
  bool ret;

  ret = mIspDev->getMaxMinGain(&max_gain, &min_gain);
  if (!ret) {
    ALOGE("%s: getTimeAndGain failed!",
          __func__);
    return -1;
  }

  gain = max_gain;
  return 0;
}

int CamApiItf::SetAeMaxExposureTime(float time)
{
  if(mAEMaxExposureTime != time){
    mAEMaxExposureTime = time;
    configIsp_l(NULL);
  }

  return 0;
}

int CamApiItf::SetAeMaxExposureGain(float gain)
{
  if(mAEMaxExposureGain != gain){
    mAEMaxExposureGain = gain;
    configIsp_l(NULL);
  }

  return 0;
}

int CamApiItf::getAeState(enum HAL_AE_STATE* ae_state) {
  mIspDev->getAeState(ae_state);
  return 0;
}

int CamApiItf::getAeMeanLuma(float &meanLuma)
{
  float mean;
  bool ret;

  ret = mIspDev->getMeanLuma(&mean);
  if (!ret) {
    ALOGE("%s: getMeanLuma failed!",
          __func__);
    return -1;
  }

  meanLuma = mean;
  return 0;
}

int CamApiItf::setWhiteBalance(HAL_WB_MODE wbMode) {
  if (m_wb_mode == wbMode)
    return 0;

  m_wb_mode = wbMode;
  configIsp_l(NULL);
  return 0;
}

int CamApiItf::setAeMode(enum HAL_AE_OPERATION_MODE aeMode) {
  if (mCurAeMode == aeMode)
    return 0;

  mCurAeMode = aeMode;
  configIsp_l(NULL);
  return 0;
}

int CamApiItf::setManualGainAndTime(float hal_gain, float hal_time) {
  int sensor_gain = 0;
  int sensor_time = 0;
  unsigned int gain_percent = 100;
  int sensor_vts;
  float init_fps;

  sensor_vts = mIspCfg.sensor_mode.line_periods_per_field;
  if (mIspCfg.aec_cfg.api_set_fps) {
    init_fps = mIspCfg.sensor_mode.pixel_clock_freq_mhz * 1000000 /
      mIspCfg.sensor_mode.line_periods_per_field /
      mIspCfg.sensor_mode.pixel_periods_per_line;

    sensor_vts = sensor_vts * init_fps /
      mIspCfg.aec_cfg.api_set_fps;
  }
  mAEManualExpTime = hal_time;
  mAEManualExpGain = hal_gain;

  mIspDev->mapHalExpToSensor(hal_gain, hal_time, sensor_gain, sensor_time);
  if (sensor_gain >= 0 && sensor_time >= 0)
    mIspDev->setExposure(sensor_vts, sensor_time, sensor_gain, gain_percent);
  else
    ALOGE("%s: Manual gain or time is invalid! hal_gain(%f), hal_time(%f), sensor_time(%d), sensor_gain(%d)",
      __func__, hal_gain, hal_time, sensor_time, sensor_gain);
}

int CamApiItf::setAntiBandMode(enum HAL_AE_FLK_MODE flkMode) {
  if (mFlkMode == flkMode)
    return 0;
  mFlkMode = flkMode;
  configIsp_l(NULL);
  return 0;
}

int CamApiItf::setAeBias(int aeBias) {
  if (mCurAeBias == aeBias)
    return 0;
  mCurAeBias = aeBias;
  configIsp_l(NULL);
  return 0;
}

int CamApiItf::setFps(HAL_FPS_INFO_t fps) {
  if ((fps.numerator != m_fps.numerator) ||
     (fps.denominator != m_fps.denominator)) {

    m_fps = fps;
    mIspDev->setAutoAdjustFps(false);
    configIsp_l(NULL);
  }

  return 0;
}

int CamApiItf::getFps(HAL_FPS_INFO_t &fps) {
  if (m_fps.numerator && m_fps.denominator) {
    fps = m_fps;
  } else {
    fps.numerator = 0;
    fps.denominator = 0;
  }

  return 0;
}

int CamApiItf::setAeWindow(int left_hoff, int top_voff, int right_width, int bottom_height) {
  mAeWin.left_hoff = left_hoff;
  mAeWin.top_voff = top_voff;
  mAeWin.right_width = right_width;
  mAeWin.bottom_height = bottom_height;

  configIsp_l(NULL);
  return 0;
}

int CamApiItf::getAeWindow(int &left_hoff, int &top_voff, int &right_width, int &bottom_height) {
  left_hoff = mAeWin.left_hoff;
  top_voff = mAeWin.top_voff;
  right_width = mAeWin.right_width;
  bottom_height = mAeWin.bottom_height;
  return 0;
}

int CamApiItf::setExposureMeterMode(enum HAL_AE_METERING_MODE aeMeterMode) {
  mCurAeMeterMode = aeMeterMode;
  configIsp_l(NULL);
  return 0;
}
int CamApiItf::getExposureMeterMode(enum HAL_AE_METERING_MODE& aeMeterMode) {
  aeMeterMode = mCurAeMeterMode;
  return 0;
}

int CamApiItf::setExposureMeterCoeff(unsigned char meter_coeff[]) {
  memcpy(mAeMeterCoeff, meter_coeff, sizeof(mAeMeterCoeff));
  configIsp_l(NULL);
  return 0;
}
int CamApiItf::getExposureMeterCoeff(unsigned char meter_coeff[]) {
  memcpy(meter_coeff, mAeMeterCoeff, sizeof(mAeMeterCoeff));
  return 0;
}

int CamApiItf::setAeSetPoint(float set_point) {
  mAeSetPoint = set_point;
  configIsp_l(NULL);
  return 0;
}
int CamApiItf::getAeSetPoint(float &set_point) {
  set_point = mAeSetPoint;
  return 0;
}

int CamApiItf::set3ALocks(int locks) {
  m3ALocks = locks & HAL_3A_LOCKS_ALL;
  configIsp_l(NULL);
  return 0;
}

int CamApiItf::get3ALocks(int& curLocks) {
  curLocks = m3ALocks;
  return 0;
}

int CamApiItf::setCprocDefault() {
  if (mISPBrightness == -120) {
    mISPBrightness = 0;
    mBrightness = 0;
  }
  if (mISPContrast == -200.0f) {
    mISPContrast = 1.0f;
    mContrast = 0;
  }
  if (mISPHue == -200.0f) {
    mISPHue = 0;
    mHue = 0;
  }
  if (mISPSaturation == -200.0f) {
    mISPSaturation = 1.0f;
    mSaturation = 0;
  }
}

//brightness
int CamApiItf::getSupportedBtRange(HAL_RANGES_t& brightRange)
{
  brightRange.max = 100;
  brightRange.min = -100;
  brightRange.step = 1;
  return 0;
}

int CamApiItf::setBrightness(int brightVal)
{
  int tmp;
  if (mBrightness == brightVal )
    return 0;
  //ISP  brightness range is -128 to 127
  //reported brightness range is -100 to 100
  tmp = (brightVal / 100.0) * 128.0;
  mISPBrightness =  (tmp >= 127) ? 127 : tmp;
  mBrightness = brightVal;
  setCprocDefault();
  configIsp_l(NULL);
  return 0;
}

int CamApiItf::getApiBrightness(int& brightVal)
{
  brightVal = mBrightness;
  return 0;
}

int CamApiItf::getBrightness(int& brightVal)
{
  bool ret;
  int tmp;
  bool_t enabled;
  CamIspCtrItf::Configuration cfg;

  ret = mIspDev->getIspConfig(HAL_ISP_CPROC_ID, enabled, cfg);
  if (ret) {
    tmp = (cfg.cproc.brightness / 128.0) * 100.0;
    brightVal =  (tmp >= 100) ? 100 : tmp;
    return 0;
  }

  return -1;
}

//contrast
int CamApiItf::getSupportedCtRange(HAL_RANGES_t& contrastRange)
{
  contrastRange.max = 100;
  contrastRange.min = -100;
  contrastRange.step = 1;
  return 0;
}

int CamApiItf::setContrast(int contrast)
{
  if (mContrast == contrast)
    return 0;
  // 1.992 means the  max ISP contrast value
  // 100 means the max reported contrast value
  // map (-100,100) to (0, 1.992)
  mISPContrast = ((contrast+100) / 200.0) * 2.0;
  mISPContrast =  (mISPContrast > 1.992) ? 1.992 : mISPContrast;
  mContrast = contrast;
  setCprocDefault();
  configIsp_l(NULL);
  return 0;
}

int CamApiItf::getApiContrast(int& contrast)
{
  contrast = mContrast;
  return 0;
}

int CamApiItf::getContrast(int& contrast)
{
  bool ret;
  int tmp;
  bool_t enabled;
  CamIspCtrItf::Configuration cfg;

  ret = mIspDev->getIspConfig(HAL_ISP_CPROC_ID, enabled, cfg);
  if (ret) {
    tmp = (cfg.cproc.contrast * 100.0) - 100.0;
    contrast =  (tmp >= 100) ? 100 : tmp;
    return 0;
  }
  return -1;
}

//saturation
int CamApiItf::getSupportedStRange(HAL_RANGES_t& saturationRange)
{
  saturationRange.max = 100;
  saturationRange.min = -100;
  saturationRange.step = 1;
}

int CamApiItf::setSaturation(int sat)
{
  if (mSaturation== sat)
    return 0;
  // 1.992 means the max ISP Saturation value
  // 100 means the max reported Saturation value
  // map (-100,100) to (0, 1.992)
  mISPSaturation = ((sat+100) / 200.0) * 2.0;
  mISPSaturation =  (mISPSaturation > 1.992) ? 1.992 : mISPSaturation;
  mSaturation = sat;
  setCprocDefault();
  configIsp_l(NULL);
  return 0;
}

int CamApiItf::getApiSaturation(int& sat)
{
  sat = mSaturation;
  return 0;
}

int CamApiItf::getSaturation(int& sat)
{
  bool ret;
  int tmp;
  bool_t enabled;
  CamIspCtrItf::Configuration cfg;

  ret = mIspDev->getIspConfig(HAL_ISP_CPROC_ID, enabled, cfg);
  if (ret) {
    tmp = (cfg.cproc.saturation * 100.0) - 100.0;
    sat =  (tmp >= 100) ? 100 : tmp;
    return 0;
  }
  return -1;
}

//hue
int CamApiItf::getSupportedHueRange(HAL_RANGES_t& hueRange)
{
  hueRange.max = 90;
  hueRange.min = -90;
  hueRange.step = 1;
}

int CamApiItf::setHue(int hue)
{
  if (mHue == hue)
    return 0;
  // ISP hue range is -90 to 87.188
  // reported hue range is -90 to 90
  mISPHue= (hue / 90.0) * 90.0;
  mISPHue =  (mISPHue > 87.188) ? 87.188: mISPHue;
  mHue = hue;
  setCprocDefault();
  configIsp_l(NULL);
  return 0;
}

int CamApiItf::getApiHue(int& hue)
{
  hue = mHue;
  return 0;
}

int CamApiItf::getHue(int& hue)
{
  bool ret;
  int tmp;
  bool_t enabled;
  CamIspCtrItf::Configuration cfg;

  ret = mIspDev->getIspConfig(HAL_ISP_CPROC_ID, enabled, cfg);
  if (ret) {
    hue = cfg.cproc.hue;
    return 0;
  }
  return -1;
}

int CamApiItf::setFocusMode(enum HAL_AF_MODE fcMode) {
  if (mAfSupport == BOOL_TRUE) {
    mAfMode = fcMode;
    configIsp_l(NULL);
  }
  return 0;
}
int CamApiItf::getFocusMode(enum HAL_AF_MODE& fcMode) {
  fcMode = mAfMode;
  return 0;
}
int CamApiItf::setFocusWin(HAL_Window_t afwin) {
  mAfWin = afwin;
  configIsp_l(NULL);
  return 0;
}
int CamApiItf::getFocusWin(HAL_Window_t& afwin) {
  afwin = mAfWin;
  return 0;
}

int CamApiItf::trigggerAf(bool trigger) {
  int ret = 0;
  ALOGD("CamIsp10DevHwItf::trigggerAf");
  if (mAfMode == HAL_AF_MODE_AUTO && trigger == true) {
    mAfTrigger = BOOL_TRUE;
	ALOGD("trigggerAf %d", mAfTrigger);
	configIsp_l(NULL);
  }
  return ret;
}

// wdr
int CamApiItf::setWdrMode(enum HAL_MODE_e mode)
{
  mWdr.mode = mode;
  configIsp_l(NULL);
  return 0;
}

int CamApiItf::getWdrMode(enum HAL_MODE_e& mode)
{
  return mWdr.mode;
}

int CamApiItf::setWdrLevel(int level)
{
  if (level < 0 || level > 15) {
    mWdr.gain_max_clip_enable = BOOL_FALSE;
  } else {
    mWdr.gain_max_clip_enable = BOOL_TRUE;
    mWdr.gain_max_value = level;
  }
  configIsp_l(NULL);
  return 0;
}

int CamApiItf::getWdrLevel(int& level)
{
  if (!mWdr.gain_max_clip_enable)
    level = -1;
  else
    level = mWdr.gain_max_value;

  return 0;
}

int CamApiItf::setWdrCurve(HAL_ISP_WDR_MODE_e mode,
    unsigned short (&wdr_dy)[HAL_ISP_WDR_SECTION_MAX + 1])
{
  if (mode == HAL_ISP_WDR_MODE_INVALID) {
    mWdr.curve_enable = BOOL_FALSE;
  } else {
    mWdr.curve_enable = BOOL_TRUE;
    mWdr.curve_mode = mode;
    memcpy(mWdr.curve_dy, wdr_dy, sizeof(mWdr.curve_dy));
  }
  configIsp_l(NULL);
  return 0;
}

int CamApiItf::getWdrCurve(HAL_ISP_WDR_MODE_e& mode,
    unsigned short (&wdr_dy)[HAL_ISP_WDR_SECTION_MAX + 1])
{
  mode = mWdr.curve_mode;
  memcpy(wdr_dy, mWdr.curve_dy, sizeof(wdr_dy));
  return 0;
}

int CamApiItf::set3DnrMode(enum HAL_MODE_e mode)
{
  m3DnrMode = mode;
  configIsp_l(NULL);
  return 0;
}

int CamApiItf::get3DnrMode(enum HAL_MODE_e& mode)
{
  mode = m3DnrMode;
  return 0;
}

int CamApiItf::set3DnrLevel(struct HAL_3DnrLevelCfg& cfg)
{
  m3DnrLevel = cfg;
  configIsp_l(NULL);
  return 0;
}

int CamApiItf::get3DnrLevel(struct HAL_3DnrLevelCfg& cfg)
{
  cfg = m3DnrLevel;
  return 0;
}

int CamApiItf::set3DnrParam(struct HAL_3DnrParamCfg& cfg)
{
  m3DnrParam = cfg;
  configIsp_l(NULL);
  return 0;
}

int CamApiItf::get3DnrParam(struct HAL_3DnrParamCfg& cfg)
{
  cfg = m3DnrParam;
  return 0;
}

//filter
int CamApiItf::setFilterLevel(enum HAL_MODE_e mode,
	enum HAL_FLT_DENOISE_LEVEL_e denoise, enum HAL_FLT_SHARPENING_LEVEL_e sharp)
{
  mFltMode = mode;
  mDenoise = denoise;
  mSharp = sharp;
  configIsp_l(NULL);
  return 0;
}

int CamApiItf::getFilterLevel(enum HAL_MODE_e& mode,
	enum HAL_FLT_DENOISE_LEVEL_e& denoise, enum HAL_FLT_SHARPENING_LEVEL_e& sharp)
{
  mode = mFltMode;
  denoise = mDenoise;
  sharp = mSharp;
  return 0;
}

//bls
int CamApiItf::setBls(enum HAL_MODE_e mode, struct HAL_ISP_bls_cfg_s& bls_cfg) {
  mBlsMode = mode;
  mBlsCfg = bls_cfg;
  configIsp_l(NULL);
}

int CamApiItf::getBls(enum HAL_MODE_e& mode, struct HAL_ISP_bls_cfg_s& bls_cfg) {
  mode = mBlsMode;
  bls_cfg = mBlsCfg;
}

int CamApiItf::getIspBls(bool& onoff, struct HAL_ISP_bls_cfg_s& bls_cfg) {
  bool_t enabled;
  CamIspCtrItf::Configuration cfg;

  if (!mIspDev->getIspConfig(HAL_ISP_BLS_ID, enabled, cfg)) {
    ALOGE("%s: mISPDev->configure failed!",
          __func__);
  }

  bls_cfg = cfg.bls_cfg;
  onoff = (enabled == BOOL_TRUE) ? true: false;
}

//goc
int CamApiItf::setGoc(enum HAL_MODE_e mode, struct HAL_ISP_GOC_s& goc) {
  for(int i = 0; i < sizeof(goc.scene_name); i++)
    goc.scene_name[i] = toupper(goc.scene_name[i]);
  mGocMode = mode;
  mGocCfg = goc;
  configIsp_l(NULL);
}
int CamApiItf::getGoc(enum HAL_MODE_e& mode, struct HAL_ISP_GOC_s& goc) {
  mode = mGocMode;
  goc = mGocCfg;
}
int CamApiItf::getIspGoc(bool& onoff, struct HAL_ISP_GOC_s& goc) {
  bool_t enabled;
  CamIspCtrItf::Configuration cfg;

  if (!mIspDev->getIspConfig(HAL_ISP_CALIBDB_GOC_ID, enabled, cfg)) {
    ALOGE("%s: mISPDev->configure failed!",
          __func__);
  }

  goc = cfg.api_goc_cfg;
  onoff = (enabled == BOOL_TRUE) ? true: false;
}

//cproc
int CamApiItf::setCproc(enum HAL_MODE_e mode, struct HAL_ISP_CPROC_s& cproc) {
  mCprocMode = mode;
  mCprocCfg = cproc;
  configIsp_l(NULL);
}
int CamApiItf::getCproc(enum HAL_MODE_e& mode, struct HAL_ISP_CPROC_s& cproc) {
  mode = mCprocMode;
  cproc = mCprocCfg;
}
int CamApiItf::getIspCproc(bool& onoff, struct HAL_ISP_CPROC_s& cproc) {
  bool_t enabled;
  CamIspCtrItf::Configuration cfg;

  if (!mIspDev->getIspConfig(HAL_ISP_CALIBDB_CPROC_ID, enabled, cfg)) {
    ALOGE("%s: mISPDev->configure failed!",
          __func__);
  }

  cproc = cfg.api_cproc_cfg;
  onoff = (enabled == BOOL_TRUE) ? true: false;
}

//lsc
int CamApiItf::setAwbLsc(enum HAL_MODE_e mode, struct HAL_ISP_Lsc_Profile_s& lsc_cfg) {
  mAwbLscMode = mode;
  mAwbLscPfl = lsc_cfg;

  for(int i = 0; i < sizeof(mAwbLscPfl.LscName); i++)
    mAwbLscPfl.LscName[i] = toupper(mAwbLscPfl.LscName[i]);
  configIsp_l(NULL);
}

int CamApiItf::getAwbLsc(enum HAL_MODE_e& mode, struct HAL_ISP_Lsc_Profile_s& lsc_cfg) {
  mode = mAwbLscMode;
  lsc_cfg = mAwbLscPfl;
}

int CamApiItf::getIspAwbLsc(bool& onoff, struct HAL_ISP_Lsc_Profile_s& lsc_cfg) {
  bool_t enabled;
  CamIspCtrItf::Configuration cfg;

  memcpy(cfg.awb_lsc_pfl.LscName, lsc_cfg.LscName, sizeof(lsc_cfg.LscName));
  for(int i = 0; i < sizeof(cfg.awb_lsc_pfl.LscName); i++)
    cfg.awb_lsc_pfl.LscName[i] = toupper(cfg.awb_lsc_pfl.LscName[i]);
  if (!mIspDev->getIspConfig(HAL_ISP_AWB_LSC_ID, enabled, cfg)) {
    ALOGE("%s: mISPDev->getIspConfig failed!",__func__);
  }

  lsc_cfg = cfg.awb_lsc_pfl;
  onoff = (enabled == BOOL_TRUE) ? true: false;
}

int CamApiItf::getIspAwbLsc(bool& onoff, struct HAL_ISP_Lsc_Query_s& query,
	struct HAL_ISP_Lsc_Profile_s& lsc_cfg) {
  bool_t enabled;
  CamIspCtrItf::Configuration cfg;

  memset(&cfg, 0, sizeof(cfg));
  memcpy(cfg.awb_lsc_pfl.LscName, "CURRENT", sizeof("CURRENT"));
  if (!mIspDev->getIspConfig(HAL_ISP_AWB_LSC_ID, enabled, cfg)) {
    ALOGE("%s: mISPDev->getIspConfig failed!", __func__);
  }

  lsc_cfg = cfg.awb_lsc_pfl;
  onoff = (enabled == BOOL_TRUE) ? true: false;
  query = cfg.awb_lsc_query;
}

//ccm
int CamApiItf::setAwbCcm(enum HAL_MODE_e mode, struct HAL_ISP_AWB_CCM_SET_s& ccm_cfg) {
  mAwbCcmMode = mode;
  mAwbCcmPfl = ccm_cfg;

  for(int i = 0; i < sizeof(mAwbCcmPfl.ill_name); i++)
    mAwbCcmPfl.ill_name[i] = toupper(mAwbCcmPfl.ill_name[i]);
  configIsp_l(NULL);
}
int CamApiItf::getAwbCcm(enum HAL_MODE_e& mode, struct HAL_ISP_AWB_CCM_SET_s& ccm_cfg) {
  mode = mAwbCcmMode;
  ccm_cfg = mAwbCcmPfl;
}
int CamApiItf::getIspAwbCcm(bool& onoff, struct HAL_ISP_AWB_CCM_GET_s& ccm_cfg) {
  bool_t enabled;
  CamIspCtrItf::Configuration cfg;

  memset(&cfg, 0, sizeof(cfg));
  if (!mIspDev->getIspConfig(HAL_ISP_AWB_CCM_ID, enabled, cfg)) {
    ALOGE("%s: mISPDev->getIspConfig failed!", __func__);
  }

  onoff = (enabled == BOOL_TRUE) ? true: false;
  ccm_cfg = cfg.awb_ccm_get;
}

//awb
int CamApiItf::setAwb(enum HAL_MODE_e mode, struct HAL_ISP_AWB_s& awb) {
  if (mode == HAL_MODE_OFF)
    mAwbGainIlluMode = HAL_MODE_MANUAL;
  else
    mAwbGainIlluMode = HAL_MODE_OFF;
  mAwbGainIlluPfl = awb;

  for(int i = 0; i < sizeof(mAwbGainIlluPfl.ill_name); i++)
    mAwbGainIlluPfl.ill_name[i] = toupper(mAwbGainIlluPfl.ill_name[i]);

  configIsp_l(NULL);
}
int CamApiItf::getAwb(enum HAL_MODE_e& mode, struct HAL_ISP_AWB_s& awb) {
  mode = mAwbGainIlluMode;
  awb = mAwbGainIlluPfl;
}
int CamApiItf::getIspAwb(bool& onoff, struct HAL_ISP_AWB_s& awb) {
  bool_t enabled;
  CamIspCtrItf::Configuration cfg;

  memset(&cfg, 0, sizeof(cfg));
  if (!mIspDev->getIspConfig(HAL_ISP_AWB_GAIN_ILLU_ID, enabled, cfg)) {
    ALOGE("%s: mISPDev->getIspConfig failed!", __func__);
  }

  onoff = (enabled == BOOL_TRUE) ? false: true;
  awb = cfg.awb_gain_illu_pfl;
  for(int i = 0; i < sizeof(cfg.awb_gain_illu_pfl.ill_name); i++)
    awb.ill_name[i] = toupper(cfg.awb_gain_illu_pfl.ill_name[i]);
}
//awb white point
int CamApiItf::setAwbWP(struct HAL_ISP_AWB_White_Point_Set_s& wp) {
  mAwbWpSet = wp;
  configIsp_l(NULL);
}
int CamApiItf::getAwbWP(struct HAL_ISP_AWB_White_Point_Get_s& wp) {
  bool_t enabled;
  CamIspCtrItf::Configuration cfg;

  if (!mIspDev->getIspConfig(HAL_ISP_AWB_WP_GET_ID, enabled, cfg)) {
    ALOGE("%s: mISPDev->getIspConfig failed!", __func__);
  }

  wp = cfg.awb_wp_get;
}

//awb Curve
int CamApiItf::setAwbCurve(struct HAL_ISP_AWB_Curve_s& curve) {
  mAwbCurve = curve;
  configIsp_l(NULL);
}
int CamApiItf::getAwbCurve(struct HAL_ISP_AWB_Curve_s& curve) {
  bool_t enabled;
  CamIspCtrItf::Configuration cfg;

  if (!mIspDev->getIspConfig(HAL_ISP_AWB_CURVE_ID, enabled, cfg)) {
    ALOGE("%s: mISPDev->getIspConfig failed!", __func__);
  }

  curve = cfg.awb_curve;
}

//awb RefGain
int CamApiItf::setAwbRefGain(struct HAL_ISP_AWB_RefGain_s& refgain) {
  mAwbRefGain = refgain;
  configIsp_l(NULL);
}

int CamApiItf::getAwbRefGain(struct HAL_ISP_AWB_RefGain_s& refgain) {
  bool_t enabled;
  CamIspCtrItf::Configuration cfg;

  memset(&cfg, 0, sizeof(cfg));
  if (!mIspDev->getIspConfig(HAL_ISP_AWB_REFGAIN_ID, enabled, cfg)) {
    ALOGE("%s: mISPDev->getIspConfig failed!", __func__);
  }

  refgain = cfg.awb_refgain;
}

void CamApiItf::setNightMode(bool night_mode) {
  mNightMode = night_mode;
  mIspDev->setNightMode(night_mode);
}

bool CamApiItf::getNightMode() {
  return mNightMode;
}

void CamApiItf::setDayNightSwitch(enum HAL_DAYNIGHT_MODE sw) {
  mDayNightSwitch = sw;
  mIspDev->setDayNightSwitch(sw);
}

enum HAL_DAYNIGHT_MODE CamApiItf::getDayNightSwitch() {
  return mDayNightSwitch;
}


//AdpfDpf
int CamApiItf::setAdpfDpf(struct HAL_ISP_ADPF_DPF_s& dpf) {
  mAdpfDpf = dpf;

  for(int i = 0; i < sizeof(mAdpfDpf.dpf_name); i++)
    mAdpfDpf.dpf_name[i] = toupper(mAdpfDpf.dpf_name[i]);
  configIsp_l(NULL);
}

int CamApiItf::getAdpfDpf(struct HAL_ISP_ADPF_DPF_s& dpf) {
  bool_t enabled;
  CamIspCtrItf::Configuration cfg;

  memset(&cfg, 0, sizeof(cfg));
  if (!mIspDev->getIspConfig(HAL_ISP_ADPF_DPF_ID, enabled, cfg)) {
    ALOGE("%s: mISPDev->getIspConfig failed!", __func__);
  }

  dpf = cfg.adpf_dpf_cfg;
}

//AdpfFlt
int CamApiItf::setAdpfFlt(struct HAL_ISP_FLT_Set_s& flt) {
  mAdpfFlt = flt;

  for(int i = 0; i < sizeof(mAdpfFlt.filter_name); i++)
    mAdpfFlt.filter_name[i] = toupper(mAdpfFlt.filter_name[i]);
  configIsp_l(NULL);
}
int CamApiItf::getAdpfFlt(uint8_t scene_mode, uint8_t level, struct HAL_ISP_FLT_Get_s& flt) {
  bool_t enabled;
  CamIspCtrItf::Configuration cfg;

  cfg.adpf_flt_cfg.scene_mode = scene_mode;
  cfg.adpf_flt_cfg.level = level;
  if (!mIspDev->getIspConfig(HAL_ISP_ADPF_FLT_ID, enabled, cfg)) {
    ALOGE("%s: mISPDev->getIspConfig failed!", __func__);
  }

  memset(&flt, 0, sizeof(flt));
  strcpy((char *)flt.filter_name, (char *)cfg.adpf_flt_cfg.filter_name);
  flt.filter_enable = cfg.adpf_flt_cfg.filter_enable;
  flt.denoise = cfg.adpf_flt_cfg.denoise;
  flt.sharp = cfg.adpf_flt_cfg.sharp;
  flt.level_conf_enable = cfg.adpf_flt_cfg.level_conf_enable & 0x01;
  flt.level_conf = cfg.adpf_flt_cfg.level_conf;
  if (cfg.adpf_flt_cfg.level_conf_enable & 0x02)
    flt.is_level_exit = 0;
  else
    flt.is_level_exit = 1;

}

