#ifndef _CAM_ISP_CTRL_ITF_H_
#define _CAM_ISP_CTRL_ITF_H_
#include <stdlib.h>
#include <stdio.h>
#include <memory>
#include "cam_types.h"
#include "CamThread.h"
#include "../../include/oslayer/oslayer.h"

using namespace std;

#define ISP_BPC_MASK    (1 << 0)
#define ISP_BLS_MASK    (1 << 1)
#define ISP_SDG_MASK    (1 << 2)
#define ISP_HST_MASK    (1 << 3)
#define ISP_LSC_MASK    (1 << 4)
#define ISP_AWB_GAIN_MASK (1 << 5)
#define ISP_FLT_MASK  (1 << 6)
#define ISP_BDM_MASK  (1 << 7)
#define ISP_CTK_MASK  (1 << 8)
#define ISP_GOC_MASK  (1 << 9)
#define ISP_CPROC_MASK  (1 << 10)
#define ISP_AFC_MASK  (1 << 11)
#define ISP_AWB_MEAS_MASK (1 << 12)
#define ISP_IE_MASK (1 << 13)
#define ISP_AEC_MASK  (1 << 14)
#define ISP_WDR_MASK  (1 << 15)
#define ISP_DPF_MASK  (1 << 16)
#define ISP_DPF_STRENGTH_MASK (1 << 17)
#define ISP_DSP_3DNR_MASK (1<<18)
#define ISP_DSP_AWB_LSC_MASK (1<<19)
#define ISP_ADPF_DPF_MASK (1<<20)
#define ISP_ADPF_FLT_MASK (1<<21)
#define ISP_AWB_CCM_MASK (1<<22)
#define ISP_AWB_CURVE_MASK (1<<23)
#define ISP_AWB_WP_SET_MASK (1<<24)
#define ISP_NEW_DSP_3DNR_MASK (1<<25)


#define ISP_ALL_MASK  (0xffffffff)

#define SUBDEV_IRCUT_MASK (1 << 0)
#define SUBDEV_FLASHLIGHT_MASK (1 << 1)

class CamIspCtrItf {
 public:
  struct Configuration {
    USE_CASE uc;
    struct HAL_AecCfg aec_cfg;
    struct HAL_AfcCfg afc_cfg;
    struct HAL_AwbCfg awb_cfg;
    HAL_FLASH_MODE flash_mode;
    HAL_IAMGE_EFFECT ie_mode;
    int aaa_locks;
    struct HAL_ColorProcCfg cproc;
    struct HAL_WdrCfg wdr_cfg;
    enum HAL_MODE_e dsp3dnr_mode;
    struct HAL_3DnrLevelCfg dsp3dnr_level;
    struct HAL_3DnrParamCfg dsp3dnr_param;
	enum HAL_MODE_e newDsp3dnr_mode;
	struct HAL_New3DnrCfg_s newDsp3dnr_cfg;
    enum HAL_MODE_e flt_mode;
    enum HAL_FLT_DENOISE_LEVEL_e flt_denoise;
    enum HAL_FLT_SHARPENING_LEVEL_e flt_sharp;
    enum HAL_MODE_e bls_mode;
    struct HAL_ISP_bls_cfg_s bls_cfg;
    enum HAL_MODE_e goc_mode;
    struct HAL_ISP_GOC_s api_goc_cfg;
    enum HAL_MODE_e cproc_mode;
    struct HAL_ISP_CPROC_s api_cproc_cfg;
    enum HAL_MODE_e awb_lsc_mode;
    struct HAL_ISP_Lsc_Profile_s awb_lsc_pfl;
    struct HAL_ISP_Lsc_Query_s awb_lsc_query;
    struct HAL_SensorModeData sensor_mode;
    struct HAL_ISP_ADPF_DPF_s adpf_dpf_cfg;
    struct HAL_ISP_FLT_Set_s adpf_flt_cfg;
    enum HAL_MODE_e awb_ccm_mode;
    struct HAL_ISP_AWB_CCM_SET_s awb_ccm_pfl;
    struct HAL_ISP_AWB_CCM_GET_s awb_ccm_get;
    enum HAL_MODE_e awb_gain_illu_mode;
    struct HAL_ISP_AWB_s awb_gain_illu_pfl;
    struct HAL_ISP_AWB_RefGain_s awb_refgain;
    struct HAL_ISP_AWB_Curve_s awb_curve;
    struct HAL_ISP_AWB_White_Point_Set_s awb_wp_set;
    struct HAL_ISP_AWB_White_Point_Get_s awb_wp_get;
  };

  CamIspCtrItf(): mISP3AThread(new ISP3AThread(this)) {};
  virtual ~CamIspCtrItf() {};
  virtual bool init(const char* tuningFile,
                    const char* ispDev,
                    enum CAMISP_CTRL_MODE ctrl_mode = CAMISP_CTRL_MASTER) = 0;
  virtual bool deInit() = 0;
  virtual bool configure(const Configuration& config) = 0;
  /* control ISP module directly*/
  virtual bool configureISP(const void* config) = 0;
  virtual bool start(bool run_3a_thd) = 0;
  virtual bool stop() = 0;
  virtual void mapHalExpToSensor(float hal_gain, float hal_time, int& sensor_gain, int& sensor_time) = 0;
  
  virtual bool getTimeAndGain(float* time, float* gain) = 0;
  virtual bool getMaxMinTime(float* max_time, float* min_time) = 0;
  virtual bool getMaxMinGain(float* max_gain, float* min_gain) = 0;
  virtual bool getMeanLuma(float* MeanLuma) = 0;
  virtual void setNightMode(bool night_mode) = 0;
  virtual void setDayNightSwitch(enum HAL_DAYNIGHT_MODE sw) = 0;
  virtual void getAeState(enum HAL_AE_STATE* ae_state) = 0;
  virtual bool getIspConfig(enum HAL_ISP_SUB_MODULE_ID_e mod_id,
      bool_t& enabled, CamIspCtrItf::Configuration& cfg) = 0;
  virtual int setExposure(unsigned int vts, unsigned int exposure, unsigned int gain, unsigned int gain_percent) = 0;
  virtual int setAutoAdjustFps(bool auto_adjust_fps) = 0;
  virtual void mapSensorExpToHal(int sensorGain, int sensorInttime, float& halGain, float& halInttime) = 0;
  /*
    virtual bool awbConfig(struct HAL_AwbCfg *cfg) = 0;
    virtual bool aecConfig(struct HAL_AecCfg *cfg) = 0;
    virtual bool afcConfig(struct HAL_AfcCfg *cfg) = 0;

    virtual bool ispFunLock(unsigned int fun_lock) = 0;
    virtual bool ispFunEnable(unsigned int fun_en) = 0;
    virtual bool ispFunDisable(unsigned int fun_dis) = 0;
    virtual bool getIspFunStats(unsigned int  *lock, unsigned int *enable) = 0;
    */
 protected:

  virtual bool threadLoop(void) = 0;
  class ISP3AThread : public CamThread {
   public:
    ISP3AThread(CamIspCtrItf* owener): mOwener(owener) {};
    virtual bool threadLoop(void) {return mOwener->threadLoop();};
   private:
    CamIspCtrItf* mOwener;
  };
  shared_ptr<ISP3AThread> mISP3AThread;
};

#endif

