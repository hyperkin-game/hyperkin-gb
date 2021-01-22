#ifndef _CAMAPI_INTERFACE_H_
#define _CAMAPI_INTERFACE_H_

#include <ebase/types.h>
#include <ebase/trace.h>
#include <ebase/builtins.h>
#include <sys/ioctl.h>
#include "CamIspCtrItf.h"

#ifndef __cplusplus
#define __cplusplus
#endif

using namespace std;

class CamApiItf {
 public:
  CamApiItf() {};
  ~CamApiItf() {};
  void initApiItf(CamIspCtrItf* ispDev);
  int configIsp_l(struct isp_supplemental_sensor_mode_data* sensor);

  //controls
  //3A controls:AE,AWB,AF
  //AE
  virtual int getAeTime(float &time);
  virtual int getAeGain(float &gain);
  virtual int GetAeMaxExposureTime(float &time);
  virtual int GetAeMaxExposureGain(float &gain);
  virtual int SetAeMaxExposureTime(float time);
  virtual int SetAeMaxExposureGain(float gain);
  virtual int getAeState(enum HAL_AE_STATE* ae_state);

  virtual int getAeMeanLuma(float &meanLuma);
  
  virtual int setWhiteBalance(HAL_WB_MODE wbMode);
  virtual int setAeMode(enum HAL_AE_OPERATION_MODE aeMode);
  virtual int setManualGainAndTime(float hal_gain, float hal_time);
  
  virtual int setAntiBandMode(enum HAL_AE_FLK_MODE flkMode);
  
  virtual int setAeBias(int aeBias);
  
  virtual int setFps(HAL_FPS_INFO_t fps);
  virtual int getFps(HAL_FPS_INFO_t &fps);
  
  virtual int setAeWindow(int left_hoff, int top_voff, int right_width, int bottom_height);
  virtual int getAeWindow(int &left_hoff, int &top_voff, int &right_width, int &bottom_height);
  virtual int setExposureMeterMode(enum HAL_AE_METERING_MODE aeMeterMode);
  virtual int getExposureMeterMode(enum HAL_AE_METERING_MODE& aeMeterMode);
  virtual int setExposureMeterCoeff(unsigned char meter_coeff[]);
  virtual int getExposureMeterCoeff(unsigned char meter_coeff[]);
  virtual int setAeSetPoint(float set_point);
  virtual int getAeSetPoint(float &set_point);

  virtual int set3ALocks(int locks);
  virtual int get3ALocks(int& curLocks);

  virtual int setFocusMode(enum HAL_AF_MODE fcMode);
  virtual int getFocusMode(enum HAL_AF_MODE& fcMode);
  virtual int setFocusWin(HAL_Window_t afwin);
  virtual int getFocusWin(HAL_Window_t& afwin);
  virtual int trigggerAf(bool trigger);

  //filter
  virtual int setFilterLevel(enum HAL_MODE_e mode,
	  enum HAL_FLT_DENOISE_LEVEL_e denoise, enum HAL_FLT_SHARPENING_LEVEL_e sharp);
  virtual int getFilterLevel(enum HAL_MODE_e& mode,
	  enum HAL_FLT_DENOISE_LEVEL_e& denoise, enum HAL_FLT_SHARPENING_LEVEL_e& sharp);
  //brightness
  virtual int getSupportedBtRange(HAL_RANGES_t& brightRange);
  virtual int setBrightness(int brightVal);
  virtual int getBrightness(int& brightVal);
  virtual int getApiBrightness(int& brightVal);
  //contrast
  virtual int getSupportedCtRange(HAL_RANGES_t& contrastRange);
  virtual int setContrast(int contrast);
  virtual int getContrast(int& contrast);
  virtual int getApiContrast(int& contrast);
  //saturation
  virtual int getSupportedStRange(HAL_RANGES_t& saturationRange);
  virtual int setSaturation(int sat);
  virtual int getSaturation(int& sat);
  virtual int getApiSaturation(int& sat);
  //hue
  virtual int getSupportedHueRange(HAL_RANGES_t& hueRange);
  virtual int setHue(int hue);
  virtual int getHue(int& hue);
  virtual int getApiHue(int& hue);

  //night switch
  virtual void setNightMode(bool night_mode);
  virtual bool getNightMode();
  virtual void setDayNightSwitch(enum HAL_DAYNIGHT_MODE sw);
  virtual enum HAL_DAYNIGHT_MODE getDayNightSwitch();

  //wdr
  virtual int setWdrMode(enum HAL_MODE_e mode);
  virtual int getWdrMode(enum HAL_MODE_e& mode);
  virtual int setWdrLevel(int level);
  virtual int getWdrLevel(int& level);
  virtual int setWdrCurve(HAL_ISP_WDR_MODE_e mode,
      unsigned short (&wdr_dy)[HAL_ISP_WDR_SECTION_MAX + 1]);
  virtual int getWdrCurve(HAL_ISP_WDR_MODE_e& mode,
      unsigned short (&wdr_dy)[HAL_ISP_WDR_SECTION_MAX + 1]);
  //3dnr
  virtual int set3DnrMode(enum HAL_MODE_e mode);
  virtual int get3DnrMode(enum HAL_MODE_e& mode);
  virtual int set3DnrLevel(struct HAL_3DnrLevelCfg& cfg);
  virtual int get3DnrLevel(struct HAL_3DnrLevelCfg& cfg);
  virtual int set3DnrParam(struct HAL_3DnrParamCfg& cfg);
  virtual int get3DnrParam(struct HAL_3DnrParamCfg& cfg);

  //bls
  virtual int setBls(enum HAL_MODE_e mode, struct HAL_ISP_bls_cfg_s& bls_cfg);
  virtual int getBls(enum HAL_MODE_e& mode, struct HAL_ISP_bls_cfg_s& bls_cfg);
  virtual int getIspBls(bool& onoff, struct HAL_ISP_bls_cfg_s& bls_cfg);
  //lsc
  virtual int setAwbLsc(enum HAL_MODE_e mode, struct HAL_ISP_Lsc_Profile_s& lsc_cfg);
  virtual int getAwbLsc(enum HAL_MODE_e& mode, struct HAL_ISP_Lsc_Profile_s& lsc_cfg);
  virtual int getIspAwbLsc(bool& onoff, struct HAL_ISP_Lsc_Profile_s& lsc_cfg);
  virtual int getIspAwbLsc(bool& onoff, struct HAL_ISP_Lsc_Query_s& query, struct HAL_ISP_Lsc_Profile_s& lsc_cfg);
  //ccm
  virtual int setAwbCcm(enum HAL_MODE_e mode, struct HAL_ISP_AWB_CCM_SET_s& ccm_cfg);
  virtual int getAwbCcm(enum HAL_MODE_e& mode, struct HAL_ISP_AWB_CCM_SET_s& ccm_cfg);
  virtual int getIspAwbCcm(bool& onoff, struct HAL_ISP_AWB_CCM_GET_s& ccm_cfg);
  //awb
  virtual int setAwb(enum HAL_MODE_e mode, struct HAL_ISP_AWB_s& awb);
  virtual int getAwb(enum HAL_MODE_e& mode, struct HAL_ISP_AWB_s& awb);
  virtual int getIspAwb(bool& onoff, struct HAL_ISP_AWB_s& awb);
  //awb white point
  virtual int setAwbWP(struct HAL_ISP_AWB_White_Point_Set_s& wp);
  virtual int getAwbWP(struct HAL_ISP_AWB_White_Point_Get_s& wp);
  //awb Curve
  virtual int setAwbCurve(struct HAL_ISP_AWB_Curve_s& curve);
  virtual int getAwbCurve(struct HAL_ISP_AWB_Curve_s& curve);
  //awb RefGain
  virtual int setAwbRefGain(struct HAL_ISP_AWB_RefGain_s& refgain);
  virtual int getAwbRefGain(struct HAL_ISP_AWB_RefGain_s& refgain);
  //goc
  virtual int setGoc(enum HAL_MODE_e mode, struct HAL_ISP_GOC_s& goc);
  virtual int getGoc(enum HAL_MODE_e& mode, struct HAL_ISP_GOC_s& goc);
  virtual int getIspGoc(bool& onoff, struct HAL_ISP_GOC_s& goc);
  //cproc
  virtual int setCproc(enum HAL_MODE_e mode, struct HAL_ISP_CPROC_s& cproc);
  virtual int getCproc(enum HAL_MODE_e& mode, struct HAL_ISP_CPROC_s& cproc);
  virtual int getIspCproc(bool& onoff, struct HAL_ISP_CPROC_s& cproc);
  //AdpfDpf
  virtual int setAdpfDpf(struct HAL_ISP_ADPF_DPF_s& dpf);
  virtual int getAdpfDpf(struct HAL_ISP_ADPF_DPF_s& dpf);
  //AdpfFlt
  virtual int setAdpfFlt(struct HAL_ISP_FLT_Set_s& flt);
  virtual int getAdpfFlt(uint8_t scene_mode, uint8_t level, struct HAL_ISP_FLT_Get_s& flt);

 protected:
  int setCprocDefault();

  //device ,path
  bool m_flag_init;
  //effect
  enum HAL_IAMGE_EFFECT m_image_effect;
  //awb
  enum HAL_WB_MODE m_wb_mode;
  //ae
  enum HAL_AE_OPERATION_MODE mCurAeMode;
  int mCurAeBias;
  int mCurAbsExp;
  int mAEMeanLuma;
  float mAEMaxExposureTime;
  float mAEMaxExposureGain;
  float mAEMaxGainRange;
  float mAEManualExpTime;
  float mAEManualExpGain;
  enum HAL_AE_METERING_MODE mCurAeMeterMode;
  enum HAL_AE_FLK_MODE mFlkMode;
  HAL_Window mAeWin;
  unsigned char mAeMeterCoeff[5 * 5];
  float mAeSetPoint;
  enum HAL_AE_STATE mAeState;
  //af
  int mLastLensPosition;
  enum HAL_AF_MODE mAfMode;
  HAL_Window_t mAfWin;
  bool_t mAfTrigger;
  bool_t mAfSupport;
  //3A lock
  int m3ALocks;
  //flash
  enum HAL_FLASH_MODE mFlMode;
  //scene
  enum HAL_SCENE_MODE mSceneMode;
  //ISO
  enum HAL_ISO_MODE mIsoMode;
  //zoom
  int mZoom;
  //brightness
  int mBrightness;
  //contrast
  int mContrast;
  //saturation
  int mSaturation;
  //hue
  int mHue;
  //flip
  int mFlip;
  //jpeg
  int m_jpeg_quality;
  //framerate
  HAL_FPS_INFO_t m_fps;
  //night mode
  bool mNightMode;
  enum HAL_DAYNIGHT_MODE mDayNightSwitch;
  //wdr
  struct HAL_WdrCfg mWdr;
  //3dnr
  enum HAL_MODE_e m3DnrMode;
  struct HAL_3DnrLevelCfg m3DnrLevel;
  struct HAL_3DnrParamCfg m3DnrParam;
  //filter
  enum HAL_MODE_e mFltMode;
  enum HAL_FLT_DENOISE_LEVEL_e mDenoise;
  enum HAL_FLT_SHARPENING_LEVEL_e mSharp;
  // bls
  enum HAL_MODE_e mBlsMode;
  struct HAL_ISP_bls_cfg_s mBlsCfg;
  // goc
  enum HAL_MODE_e mGocMode;
  struct HAL_ISP_GOC_s mGocCfg;
  // cproc
  enum HAL_MODE_e mCprocMode;
  struct HAL_ISP_CPROC_s mCprocCfg;
  // lsc
  enum HAL_MODE_e mAwbLscMode;
  struct HAL_ISP_Lsc_Profile_s mAwbLscPfl;
  //capture
  struct HAL_ISP_Cap_Req_s mCapReq;
  struct HAL_ISP_Cap_Result_s mCapResult;
  //Adpfdpf
  struct HAL_ISP_ADPF_DPF_s mAdpfDpf;
  //AdpfFlt
  struct HAL_ISP_FLT_Set_s mAdpfFlt;
  // ccm
  enum HAL_MODE_e mAwbCcmMode;
  struct HAL_ISP_AWB_CCM_SET_s mAwbCcmPfl;
  // awb
  enum HAL_MODE_e mAwbGainIlluMode;
  struct HAL_ISP_AWB_s mAwbGainIlluPfl;
  // awb refgain
  struct HAL_ISP_AWB_RefGain_s mAwbRefGain;
  // awb curve
  struct HAL_ISP_AWB_Curve_s mAwbCurve;
  // awb wp
  struct HAL_ISP_AWB_White_Point_Set_s mAwbWpSet;

  unsigned int mExposureSequence;

  osMutex mApiLock;
  signed char mISPBrightness;
  float mISPContrast;
  float mISPSaturation;
  float mISPHue;
  char mIqPath[64];

  CamIspCtrItf* mIspDev;
  int mDevFd;
  CamIspCtrItf::Configuration mIspCfg;
};


#endif

