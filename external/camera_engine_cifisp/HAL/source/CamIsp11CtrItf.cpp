#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/mman.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "CamIsp11CtrItf.h"
#include "cam_ia_api/cam_ia10_trace.h"
#include <ebase/utl_fixfloat.h>
#include "CamIsp11DevHwItf.h"
#include "vl6180/vl6180x_def.h"


#define AE_STABLE_COUNT_NUM   30
using namespace std;
static bool AecMeasuringMode_to_cifisp_exp_meas_mode(
    AecMeasuringMode_t in, enum cifisp_exp_meas_mode* out) {
  switch (in) {
    case AEC_MEASURING_MODE_1:
      *out = CIFISP_EXP_MEASURING_MODE_0;
      return true;
    case AEC_MEASURING_MODE_2:
      *out = CIFISP_EXP_MEASURING_MODE_1;
      return true;
    default:
      return false;
  }
}
CamIsp11CtrItf::CamIsp11CtrItf(CamHwItf* camHwItf, int devFd):
  CamIsp1xCtrItf(camHwItf, devFd) {
  //ALOGD("%s: E", __func__);

  memset(&mIspCfg, 0, sizeof(mIspCfg));
  mBlsNeededUpdate = BOOL_TRUE;
  mBlsEnabled = HAL_ISP_ACTIVE_DEFAULT;
  mSdgNeededUpdate = BOOL_TRUE;
  mSdgEnabled = HAL_ISP_ACTIVE_FALSE;
  mFltNeededUpdate = BOOL_TRUE;
  mFltEnabled = HAL_ISP_ACTIVE_DEFAULT;
  mGocNeededUpdate = BOOL_TRUE;
  mGocEnabled = HAL_ISP_ACTIVE_DEFAULT;
  mCprocNeededUpdate = BOOL_TRUE;
  mCprocEnabled = HAL_ISP_ACTIVE_DEFAULT;
  cproc_cfg.use_case = UC_PREVIEW;
  mIeNeededUpdate = BOOL_FALSE;
  mIeEnabled = HAL_ISP_ACTIVE_FALSE;
  mDpccNeededUpdate = BOOL_TRUE;
  mDpccEnabled = HAL_ISP_ACTIVE_DEFAULT;
  mBdmNeededUpdate = BOOL_TRUE;
  mBdmEnabled = HAL_ISP_ACTIVE_DEFAULT;

  /*following modules will be initialized by AWB algorithm*/
  mLscNeededUpdate = BOOL_FALSE;
  mLscEnabled = HAL_ISP_ACTIVE_FALSE;
  mAwbGainNeededUpdate = BOOL_FALSE;
  mAwbEnabled = HAL_ISP_ACTIVE_FALSE;
  mCtkNeededUpdate = BOOL_FALSE;
  mCtkEnabled = HAL_ISP_ACTIVE_FALSE;
  mAwbMeNeededUpdate = BOOL_FALSE;
  mAwbMeEnabled = HAL_ISP_ACTIVE_FALSE;

  /*following modules will be initialized by AEC algorithm*/
  mAecNeededUpdate = BOOL_FALSE;
  mAecEnabled = HAL_ISP_ACTIVE_FALSE;
  mHstNeededUpdate = BOOL_FALSE;
  mHstEnabled = HAL_ISP_ACTIVE_FALSE;

  /*following modules will be initialized by ADPF algorithm*/
  mDpfNeededUpdate = BOOL_FALSE;
  mDpfEnabled = HAL_ISP_ACTIVE_FALSE;
  mDpfStrengthNeededUpdate = BOOL_FALSE;
  mDpfStrengthEnabled = HAL_ISP_ACTIVE_FALSE;

  /*following modules will be initialized by AFC algorithm*/
  mAfcNeededUpdate = BOOL_FALSE;
  mAfcEnabled = HAL_ISP_ACTIVE_FALSE;

  mWdrNeededUpdate = BOOL_TRUE;
  mWdrEnabled = HAL_ISP_ACTIVE_DEFAULT;

  m3DnrNeededUpdate = BOOL_TRUE;
  m3DnrEnabled = HAL_ISP_ACTIVE_DEFAULT;

  mNew3DnrNeededUpdate = BOOL_TRUE;
  mNew3DnrEnabled = HAL_ISP_ACTIVE_DEFAULT;
  
  mAwbLscNeededUpdate = BOOL_TRUE;
  mAwbLscEnabled = HAL_ISP_ACTIVE_DEFAULT;

  mAdpfDpfNeededUpdate = BOOL_TRUE;
  mAdpfDpfEnabled = HAL_ISP_ACTIVE_DEFAULT;

  mAdpfFltNeededUpdate = BOOL_TRUE;
  mAdpfFltEnabled = HAL_ISP_ACTIVE_DEFAULT;

  mAwbCcmNeededUpdate = BOOL_TRUE;
  mAwbCcmEnabled = HAL_ISP_ACTIVE_DEFAULT;

  mAwbGainIlluNeededUpdate = BOOL_TRUE;
  mAwbGainIlluEnabled = HAL_ISP_ACTIVE_DEFAULT;

  mCalibdbGocNeededUpdate = BOOL_TRUE;
  mCalibdbGocEnabled = HAL_ISP_ACTIVE_DEFAULT;

  mSupportedSubDevs = 0;
  mRefWidth = 0;
  mRefHeight = 0;

  mNightMode = true;
  mDayNightSwitch = HAL_DAYNIGHT_AUTO;

  //ALOGD("%s: x", __func__);
}
CamIsp11CtrItf::~CamIsp11CtrItf() {
  //ALOGD("%s: E", __func__);
  //ALOGD("%s: x", __func__);

}

bool CamIsp11CtrItf::init(const char* tuningFile,
                          const char* ispDev, enum CAMISP_CTRL_MODE ctrl_mode) {
  int i;
  bool ret = false;
  struct CamIA10_Results ia_results = {0};
  struct CamIsp11ConfigSet isp_cfg = {0};


  osMutexLock(&mApiLock);
  if (mInitialized == 0) {
    mCamIAEngine = getCamIA10EngineItf();
    if (mCamIAEngine == NULL) {
      ALOGE("%s: getCamIA10EngineItf failed!",
            __func__);
      goto init_exit;
    }

    ALOGD("%s:tuningFile %s", __func__, tuningFile);
    if (mCamIAEngine->initStatic((char*)tuningFile) != RET_SUCCESS) {
      ALOGE("%s: initstatic failed", __func__);
      osMutexUnlock(&mApiLock);
      deInit();
      osMutexLock(&mApiLock);
      ret = false;
      goto init_exit;
    }

    if (!initISPStream(ispDev)) {
      ALOGE("%s: initISPStream failed", __func__);
      osMutexUnlock(&mApiLock);
      deInit();
      osMutexLock(&mApiLock);
      goto init_exit;
    }

    if ((mCamIA_DyCfg.aec_cfg.win.right_width == 0) ||
        (mCamIA_DyCfg.aec_cfg.win.bottom_height == 0)) {
      mCamIA_DyCfg.aec_cfg.win.left_hoff = 512;
      mCamIA_DyCfg.aec_cfg.win.top_voff = 512;
      mCamIA_DyCfg.aec_cfg.win.right_width = 1024;
      mCamIA_DyCfg.aec_cfg.win.bottom_height = 1024;
    }

    for (i = 0; i < CAM_ISP_NUM_OF_STAT_BUFS; i++) {
      mIspStats[i] = (struct cifisp_stat_buffer*)mIspStatBuf[i];
    }
    mIspIoctl = new V4l2Isp11Ioctl(mIspFd);
    mCtrlMode = ctrl_mode;

    runIA(&mCamIA_DyCfg, NULL, &ia_results);
    runISPManual(&ia_results, BOOL_FALSE);
    convertIAResults(&isp_cfg, &ia_results);
    applyIspConfig(&isp_cfg);
    applySubDevConfig(&ia_results);

    mLaserFd = open("/dev/stmvl6180_ranging", O_RDWR | O_SYNC);
    if (mLaserFd > 0) {
      //make sure it's not started
  	  if (ioctl(mLaserFd, VL6180_IOCTL_STOP , NULL) < 0) {
  	    ALOGE("%s: Could not perform VL6180_IOCTL_STOP : %s\n", __func__, strerror(errno));
          close(mLaserFd);
  	    mLaserFd = 0;
  	  } else {
           // to init 
  	    if (ioctl(mLaserFd, VL6180_IOCTL_INIT , NULL) < 0) {
  	      ALOGE("%s: Could not perform VL6180_IOCTL_INIT : %s\n", __func__, strerror(errno));
          close(mLaserFd);
  	      mLaserFd = 0;
        } else {
          ALOGD("%s: VL6180 init success!", __func__);
        }
  	  }
    }
  }
  mInitialized++;
  ret = true;

init_exit:
  osMutexUnlock(&mApiLock);
  return ret;

}

bool CamIsp11CtrItf::deInit() {
  osMutexLock(&mApiLock);
  if ((mInitialized > 0) && (--mInitialized == 0)) {
    struct CamIsp11ConfigSet isp_cfg;
    isp_cfg.active_configs = 0xffffffff;
    memset(isp_cfg.enabled, 0, sizeof(isp_cfg.enabled));
    applyIspConfig(&isp_cfg);
    if (mIspIoctl) {
      delete mIspIoctl;
      mIspIoctl = NULL;
    }
  }
  osMutexUnlock(&mApiLock);
  if (mInitialized == 0)
    return CamIsp1xCtrItf::deInit();
}

bool CamIsp11CtrItf::getTimeAndGain(float* time, float* gain) {
  *time = mIA_results.aec.coarse_integration_time;
  *gain = mIA_results.aec.analog_gain_code_global;
  return true;
}

bool CamIsp11CtrItf::getMaxMinTime(float* max_time, float* min_time) {
  *max_time = mIA_results.aec.MaxIntegrationTime;
  *min_time = mIA_results.aec.MinIntegrationTime;
  return true;
}

bool CamIsp11CtrItf::getMaxMinGain(float* max_gain, float* min_gain) {
  *max_gain = mIA_results.aec.MaxGain;
  *min_gain = mIA_results.aec.MinGain;
  return true;
}

bool CamIsp11CtrItf::getMeanLuma(float* MeanLuma) {
  *MeanLuma = mIA_results.aec.MeanLuma;
  return true;
}

void CamIsp11CtrItf::setNightMode(bool night_mode) {
  mNightMode = night_mode;
}

void CamIsp11CtrItf::setDayNightSwitch(enum HAL_DAYNIGHT_MODE sw) {
  mDayNightSwitch = sw;
}

void CamIsp11CtrItf::getAeState(enum HAL_AE_STATE* ae_state) {
  *ae_state = mAeState;
}

bool CamIsp11CtrItf::getIspConfig(enum HAL_ISP_SUB_MODULE_ID_e mod_id,
    bool_t& enabled, CamIspCtrItf::Configuration& cfg) {
  bool ret = true;
  bool mod_en;

  if (mInitialized == 0)
    return false;

  osMutexLock(&mApiLock);

  if (mod_id == HAL_ISP_BPC_ID) {
    struct cifisp_dpcc_config dpcc_config;
    if (mIspIoctl->getDpccCfg(dpcc_config, mod_en) < 0) {
      ALOGE("%s: getDpccCfg failed", __func__);
      ret = false;
    }
  }

  if (mod_id == HAL_ISP_BLS_ID) {
    struct cifisp_bls_config bls_config;
    if (mIspIoctl->getBlsCfg(bls_config, mod_en) < 0) {
      ALOGE("%s: getBlsCfg failed", __func__);
      ret = false;
    }

    enabled = (mod_en == true) ? BOOL_TRUE : BOOL_FALSE;
    cfg.bls_cfg.mode = (enum HAL_BLS_MODE)bls_config.enable_auto;
    cfg.bls_cfg.win_cfg = (enum HAL_BLS_WINCFG)bls_config.en_windows;
    cfg.bls_cfg.win1.h_offs = bls_config.bls_window1.h_offs;
    cfg.bls_cfg.win1.v_offs = bls_config.bls_window1.v_offs;
    cfg.bls_cfg.win1.width = bls_config.bls_window1.h_size;
    cfg.bls_cfg.win1.height = bls_config.bls_window1.v_size;
    cfg.bls_cfg.win2.h_offs = bls_config.bls_window2.h_offs;
    cfg.bls_cfg.win2.v_offs = bls_config.bls_window2.v_offs;
    cfg.bls_cfg.win2.width = bls_config.bls_window2.h_size;
    cfg.bls_cfg.win2.height = bls_config.bls_window2.v_size;
    cfg.bls_cfg.samples = bls_config.bls_samples;
    cfg.bls_cfg.fixed_red = bls_config.fixed_val.r;
    cfg.bls_cfg.fixed_greenR = bls_config.fixed_val.gr;
    cfg.bls_cfg.fixed_greenB = bls_config.fixed_val.gb;
    cfg.bls_cfg.fixed_blue = bls_config.fixed_val.b;
  }

  if (mod_id == HAL_ISP_SDG_ID) {
    struct cifisp_sdg_config sdg_config;
    if (!mIspIoctl->getSdgCfg(sdg_config, mod_en)) {
      ALOGE("%s: getSdgCfg failed", __func__);
      ret = false;
    }
  }

  if (mod_id == HAL_ISP_HST_ID) {
    struct cifisp_hst_config hst_config;
    if (!mIspIoctl->getHstCfg(hst_config, mod_en)) {
      ALOGE("%s: getHstCfg failed", __func__);
      ret = false;
    }
  }

  if (mod_id == HAL_ISP_LSC_ID) {
    struct cifisp_lsc_config lsc_config;
    if (!mIspIoctl->getLscCfg(lsc_config, mod_en)) {
      ALOGE("%s: getLscCfg failed", __func__);
      ret = false;
    }
  }

  if (mod_id == HAL_ISP_AWB_MEAS_ID) {
    struct cifisp_awb_meas_config awb_meas_config;
    if (!mIspIoctl->getAwbMeasCfg(awb_meas_config, mod_en)) {
      ALOGE("%s: getAwbMeasCfg failed", __func__);
      ret = false;
    }
  }

  if (mod_id == HAL_ISP_AWB_GAIN_ID) {
    struct cifisp_awb_gain_config awb_gain_config;
    if (!mIspIoctl->getAwbGainCfg(awb_gain_config, mod_en)) {
      ALOGE("%s: getAwbGainCfg failed", __func__);
      ret = false;
    }
  }
  if (mod_id == HAL_ISP_FLT_ID) {
    struct cifisp_flt_config flt_config;
    if (!mIspIoctl->getFltCfg(flt_config, mod_en)) {
      ALOGE("%s: getFltCfg failed", __func__);
      ret = false;
    }
  }
  if (mod_id == HAL_ISP_BDM_ID) {
    struct cifisp_bdm_config bdm_config;
    if (!mIspIoctl->getBdmCfg(bdm_config, mod_en)) {
      ALOGE("%s: getBdmCfg failed", __func__);
      ret = false;
    }
  }

  if (mod_id == HAL_ISP_CTK_ID) {
    struct cifisp_ctk_config ctk_config;
    if (!mIspIoctl->getCtkCfg(ctk_config, mod_en)) {
      ALOGE("%s: getCtkCfg failed", __func__);
      ret = false;
    }
  }

  if (mod_id == HAL_ISP_GOC_ID) {
    struct cifisp_goc_config goc_config;
    if (!mIspIoctl->getGocCfg(goc_config, mod_en)) {
      ALOGE("%s: getGocCfg failed", __func__);
      ret = false;
    }
  }

  if (mod_id == HAL_ISP_CPROC_ID) {
    struct cifisp_cproc_config cproc_config;
    if (!mIspIoctl->getCprocCfg(cproc_config, mod_en)) {
      ALOGE("%s: getCprocCfg failed", __func__);
      ret = false;
    }
  }

  if (mod_id == HAL_ISP_AEC_ID) {
    struct cifisp_aec_config aec_config;
    if (!mIspIoctl->getAecCfg(aec_config, mod_en)) {
      ALOGE("%s: getAecCfg failed", __func__);
      ret = false;
    }
  }

  if (mod_id == HAL_ISP_AFC_ID) {
    struct cifisp_afc_config afc_config;
    if (!mIspIoctl->getAfcCfg(afc_config, mod_en)) {
      ALOGE("%s: getAfcCfg failed", __func__);
      ret = false;
    }
  }

  if (mod_id == HAL_ISP_IE_ID) {
    struct cifisp_ie_config ie_config;
    if (!mIspIoctl->getIeCfg(ie_config, mod_en)) {
      ALOGE("%s: getIeCfg failed", __func__);
      ret = false;
    }
  }

  if (mod_id == HAL_ISP_DPF_ID) {
    struct cifisp_dpf_config dpf_config;
    if (!mIspIoctl->getDpfCfg(dpf_config, mod_en)) {
      ALOGE("%s: getDpfCfg failed", __func__);
      ret = false;
    }
  }

  if (mod_id == HAL_ISP_DPF_STRENGTH_ID) {
    struct cifisp_dpf_strength_config dpf_strength_config;
    if (!mIspIoctl->getDpfStrengthCfg(dpf_strength_config, mod_en)) {
      ALOGE("%s: getDpfStrengthCfg failed", __func__);
      ret = false;
    }
  }

  if (mod_id == HAL_ISP_WDR_ID) {
    struct cifisp_wdr_config wdr_config;
    if (!mIspIoctl->getWdrCfg(wdr_config, mod_en)) {
      ALOGE("%s: getWdrCfg failed", __func__);
      ret = false;
    }
  }

  if (mod_id == HAL_ISP_AWB_LSC_ID) {
    struct cifisp_lsc_config lsc_config;
    if (!mIspIoctl->getLscCfg(lsc_config, mod_en)) {
      ALOGE("%s: getLscCfg failed", __func__);
      ret = false;
    }
    enabled = (mod_en == true) ? BOOL_TRUE : BOOL_FALSE;

    if (strcmp((const char*)&cfg.awb_lsc_pfl.LscName, "CURRENT") == 0) {
      memcpy(&cfg.awb_lsc_query.LscNameUp, &mIA_results.awb.aLscpflName1, sizeof(cfg.awb_lsc_query.LscNameUp));
      memcpy(&cfg.awb_lsc_query.LscNameDn, &mIA_results.awb.aLscpflName2, sizeof(cfg.awb_lsc_query.LscNameDn));
      memcpy(&cfg.awb_lsc_pfl.LscName, &mIA_results.awb.aLscpflName1, sizeof(cfg.awb_lsc_pfl.LscName));

      if (mCamIAEngine.get())
        mCamIAEngine->getAwbLscPfl(&cfg.awb_lsc_pfl);

      for (int i = 0; i < HAL_ISP_LSC_SIZE_TBL_LEN; i++) {
        cfg.awb_lsc_pfl.LscXSizeTbl[i] = lsc_config.x_size_tbl[i];
        cfg.awb_lsc_pfl.LscYSizeTbl[i] = lsc_config.y_size_tbl[i];
      }
      for (int i = 0; i < HAL_ISP_LSC_MATRIX_TBL_LEN; i++) {
        cfg.awb_lsc_pfl.LscMatrix[0][i] = lsc_config.r_data_tbl[i];
        cfg.awb_lsc_pfl.LscMatrix[1][i] = lsc_config.gr_data_tbl[i];
        cfg.awb_lsc_pfl.LscMatrix[2][i] = lsc_config.gb_data_tbl[i];
        cfg.awb_lsc_pfl.LscMatrix[3][i] = lsc_config.b_data_tbl[i];
      }
    } else if (mCamIAEngine.get())
      mCamIAEngine->getAwbLscPfl(&cfg.awb_lsc_pfl);

  }

  if (mod_id == HAL_ISP_AWB_CCM_ID) {
    struct cifisp_ctk_config ctk_config;
    if (!mIspIoctl->getCtkCfg(ctk_config, mod_en)) {
      ALOGE("%s: getCtkCfg failed", __func__);
      ret = false;
    }
    enabled = (mod_en == true) ? BOOL_TRUE : BOOL_FALSE;

    if (mCamIAEngine.get())
      mCamIAEngine->getAwbCcmPfl(&cfg.awb_ccm_get);
  }

  if (mod_id == HAL_ISP_ADPF_DPF_ID) {
    if (mCamIAEngine.get())
      mCamIAEngine->getDbAdpfDpf(&cfg.adpf_dpf_cfg);
  }

  if (mod_id == HAL_ISP_ADPF_FLT_ID) {
    if (mCamIAEngine.get())
      mCamIAEngine->getDbAdpfFlt(&cfg.adpf_flt_cfg);
  }

  if (mod_id == HAL_ISP_AWB_GAIN_ILLU_ID) {
    if (mCamIAEngine.get()) {
      mCamIAEngine->getAwbGainIllu(&enabled, &cfg.awb_gain_illu_pfl);
    }
  }

  if (mod_id == HAL_ISP_AWB_REFGAIN_ID) {
    if (mCamIAEngine.get()) {
      mCamIAEngine->getAwbRefGain(&cfg.awb_refgain);
    }
  }

  if (mod_id == HAL_ISP_AWB_CURVE_ID) {
    if (mCamIAEngine.get()) {
      mCamIAEngine->getAwbCurve(&cfg.awb_curve);
    }
  }

  if (mod_id == HAL_ISP_AWB_WP_GET_ID) {
    if (mCamIAEngine.get()) {
      mCamIAEngine->getAwbWP(&cfg.awb_wp_get);
    }
  }

  if (mod_id == HAL_ISP_CALIBDB_GOC_ID) {
    struct cifisp_goc_config goc_config;
    if (!mIspIoctl->getGocCfg(goc_config, mod_en)) {
      ALOGE("%s: getGocCfg failed", __func__);
      ret = false;
    }

    enabled = (mod_en == true) ? BOOL_TRUE : BOOL_FALSE;
    if (mCamIAEngine.get()) {
      mCamIAEngine->getApiGocCfg(&cfg.api_goc_cfg);
    }
  }

  if (mod_id == HAL_ISP_CALIBDB_CPROC_ID) {
    struct cifisp_cproc_config cproc_config;
    if (!mIspIoctl->getCprocCfg(cproc_config, mod_en)) {
      ALOGE("%s: getCprocCfg failed", __func__);
      ret = false;
    }

    enabled = (mod_en == true) ? BOOL_TRUE : BOOL_FALSE;
    if (mCamIAEngine.get()) {
      mCamIAEngine->getApiCprocCfg(&cproc_cfg, &cfg.api_cproc_cfg);
    }
  }

  osMutexUnlock(&mApiLock);
  return ret;
}

int CamIsp11CtrItf::setExposure(unsigned int vts, unsigned int exposure, unsigned int gain, unsigned int gain_percent) {
  int ret;

  ret = mCamHwItf->setExposure(vts, exposure, gain, gain_percent);
  return ret;
}

int CamIsp11CtrItf::setAutoAdjustFps(bool auto_adjust_fps) {
  int ret;

  ret = mCamHwItf->setAutoAdjustFps(auto_adjust_fps);
  return ret;
}

bool CamIsp11CtrItf::applySubDevConfig(struct CamIA10_Results* results) {

  if ((results->active & CAMIA10_AFC_MASK) &&
    (mIA_results.af.LensePos != results->af.LensePos)) {
    if (mCamHwItf->setFocusPos(results->af.LensePos) < 0) {
      ALOGE("%s: setFocusPos failed", __func__);
    }
    mIA_results.af.LensePos = results->af.LensePos;
  }
}

bool CamIsp11CtrItf::applyIspConfig(struct CamIsp11ConfigSet* isp_cfg) {

  if (isp_cfg->active_configs & ISP_BPC_MASK) {
    if (!mIspIoctl->setDpccCfg(
            isp_cfg->configs.dpcc_config,
            isp_cfg->enabled[HAL_ISP_BPC_ID])) {
      ALOGE("%s: setDpccCfg failed", __func__);
    }
    mIspCfg.dpcc_config = isp_cfg->configs.dpcc_config;
    mIspCfg.enabled[HAL_ISP_BPC_ID] =
        isp_cfg->enabled[HAL_ISP_BPC_ID];
  }
  if (isp_cfg->active_configs & ISP_BLS_MASK) {
    if (!mIspIoctl->setBlsCfg(
            isp_cfg->configs.bls_config,
            isp_cfg->enabled[HAL_ISP_BLS_ID])) {
      ALOGE("%s: setBlsCfg failed", __func__);
    }
    mIspCfg.bls_config = isp_cfg->configs.bls_config;
    mIspCfg.enabled[HAL_ISP_BLS_ID] =
        isp_cfg->enabled[HAL_ISP_BLS_ID];
  }

  if (isp_cfg->active_configs & ISP_SDG_MASK) {
    if (!mIspIoctl->setSdgCfg(
            isp_cfg->configs.sdg_config,
            isp_cfg->enabled[HAL_ISP_SDG_ID])) {
      ALOGE("%s: setSdgCfg failed", __func__);
    }
    mIspCfg.sdg_config = isp_cfg->configs.sdg_config;
    mIspCfg.enabled[HAL_ISP_SDG_ID] =
        isp_cfg->enabled[HAL_ISP_SDG_ID];
  }

  if (isp_cfg->active_configs & ISP_HST_MASK) {
    if (!mIspIoctl->setHstCfg(
            isp_cfg->configs.hst_config,
            isp_cfg->enabled[HAL_ISP_HST_ID])) {
      ALOGE("%s: setHstCfg failed", __func__);
    }
    mIspCfg.hst_config = isp_cfg->configs.hst_config;
    mIspCfg.enabled[HAL_ISP_HST_ID] =
        isp_cfg->enabled[HAL_ISP_HST_ID];
  }

  if (isp_cfg->active_configs & ISP_LSC_MASK) {
    if (!mIspIoctl->setLscCfg(
            isp_cfg->configs.lsc_config,
            isp_cfg->enabled[HAL_ISP_LSC_ID])) {
      ALOGE("%s: setLscCfg failed", __func__);
    }
    mIspCfg.lsc_config = isp_cfg->configs.lsc_config;
    mIspCfg.enabled[HAL_ISP_LSC_ID] =
        isp_cfg->enabled[HAL_ISP_LSC_ID];
  }

  if (isp_cfg->active_configs & ISP_AWB_MEAS_MASK) {
    if (!mIspIoctl->setAwbMeasCfg(
            isp_cfg->configs.awb_meas_config,
            isp_cfg->enabled[HAL_ISP_AWB_MEAS_ID])) {
      ALOGE("%s: setAwbMeasCfg failed", __func__);
    }
    mIspCfg.awb_meas_config = isp_cfg->configs.awb_meas_config;
    mIspCfg.enabled[HAL_ISP_AWB_MEAS_ID] =
        isp_cfg->enabled[HAL_ISP_AWB_MEAS_ID];
  }

  if (isp_cfg->active_configs & ISP_AWB_GAIN_MASK) {
    if (!mIspIoctl->setAwbGainCfg(
            isp_cfg->configs.awb_gain_config,
            isp_cfg->enabled[HAL_ISP_AWB_GAIN_ID])) {
      ALOGE("%s: setAwbGainCfg failed", __func__);
    }
    mIspCfg.awb_gain_config = isp_cfg->configs.awb_gain_config;
    mIspCfg.enabled[HAL_ISP_AWB_GAIN_ID] =
        isp_cfg->enabled[HAL_ISP_AWB_GAIN_ID];
  }
  if (isp_cfg->active_configs & ISP_FLT_MASK) {
    if (!mIspIoctl->setFltCfg(
            isp_cfg->configs.flt_config,
            isp_cfg->enabled[HAL_ISP_FLT_ID])) {
      ALOGE("%s: setFltCfg failed", __func__);
    }
    mIspCfg.flt_config = isp_cfg->configs.flt_config;
    mIspCfg.flt_denoise_level =
        isp_cfg->configs.flt_denoise_level;
    mIspCfg.flt_sharp_level =
        isp_cfg->configs.flt_sharp_level;
    mIspCfg.enabled[HAL_ISP_FLT_ID] =
        isp_cfg->enabled[HAL_ISP_FLT_ID];
  }
  if (isp_cfg->active_configs & ISP_BDM_MASK) {
    if (!mIspIoctl->setBdmCfg(
            isp_cfg->configs.bdm_config,
            isp_cfg->enabled[HAL_ISP_BDM_ID])) {
      ALOGE("%s: setBdmCfg failed", __func__);
    }
    mIspCfg.bdm_config = isp_cfg->configs.bdm_config;
    mIspCfg.enabled[HAL_ISP_BDM_ID] =
        isp_cfg->enabled[HAL_ISP_BDM_ID];
  }

  if (isp_cfg->active_configs & ISP_CTK_MASK) {
    if (!mIspIoctl->setCtkCfg(
            isp_cfg->configs.ctk_config,
            isp_cfg->enabled[HAL_ISP_CTK_ID])) {
      ALOGE("%s: setCtkCfg failed", __func__);
    }
    mIspCfg.ctk_config = isp_cfg->configs.ctk_config;
    mIspCfg.enabled[HAL_ISP_CTK_ID] =
        isp_cfg->enabled[HAL_ISP_CTK_ID];
  }

  if (isp_cfg->active_configs & ISP_GOC_MASK) {
    if (!mIspIoctl->setGocCfg(
            isp_cfg->configs.goc_config,
            isp_cfg->enabled[HAL_ISP_GOC_ID])) {
      ALOGE("%s: setGocCfg failed", __func__);
    }
    mIspCfg.goc_config = isp_cfg->configs.goc_config;
    mIspCfg.enabled[HAL_ISP_GOC_ID] =
        isp_cfg->enabled[HAL_ISP_GOC_ID];
  }

  if (isp_cfg->active_configs & ISP_CPROC_MASK) {
    if (!mIspIoctl->setCprocCfg(
            isp_cfg->configs.cproc_config,
            isp_cfg->enabled[HAL_ISP_CPROC_ID])) {
      ALOGE("%s: setCprocCfg failed", __func__);
    }
    TRACE_D(1, "%s:apply cproc config!enabled %d",
            __func__, isp_cfg->enabled[HAL_ISP_CPROC_ID]
           );
    mIspCfg.cproc_config = isp_cfg->configs.cproc_config;
    mIspCfg.enabled[HAL_ISP_CPROC_ID] =
        isp_cfg->enabled[HAL_ISP_CPROC_ID];
  }


  if (isp_cfg->active_configs & ISP_AEC_MASK) {
    if (!mIspIoctl->setAecCfg(
            isp_cfg->configs.aec_config,
            isp_cfg->enabled[HAL_ISP_AEC_ID])) {
      ALOGE("%s: setAecCfg failed", __func__);
    }
    mIspCfg.aec_config = isp_cfg->configs.aec_config;
    mIspCfg.enabled[HAL_ISP_AEC_ID] =
        isp_cfg->enabled[HAL_ISP_AEC_ID];
  }

  if (isp_cfg->active_configs & ISP_AFC_MASK) {
    if (!mIspIoctl->setAfcCfg(
            isp_cfg->configs.afc_config,
            isp_cfg->enabled[HAL_ISP_AFC_ID])) {
      ALOGE("%s: setAfcCfg failed", __func__);
    }
    mIspCfg.afc_config = isp_cfg->configs.afc_config;
    mIspCfg.enabled[HAL_ISP_AFC_ID] =
        isp_cfg->enabled[HAL_ISP_AFC_ID];
  }


  if (isp_cfg->active_configs & ISP_IE_MASK) {
    if (!mIspIoctl->setIeCfg(
            isp_cfg->configs.ie_config,
            isp_cfg->enabled[HAL_ISP_IE_ID])) {
      ALOGE("%s: setIeCfg failed", __func__);
    }
    TRACE_D(1, "%s:apply ie config,enabled %d!", __func__,
            isp_cfg->enabled[HAL_ISP_IE_ID]);
    mIspCfg.ie_config = isp_cfg->configs.ie_config;
    mIspCfg.enabled[HAL_ISP_IE_ID] =
        isp_cfg->enabled[HAL_ISP_IE_ID];
  }

  if (isp_cfg->active_configs & ISP_DPF_MASK) {
    if (!mIspIoctl->setDpfCfg(
            isp_cfg->configs.dpf_config,
            isp_cfg->enabled[HAL_ISP_DPF_ID])) {
      ALOGE("%s: setDpfCfg failed, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x",
            __func__,
            isp_cfg->configs.dpf_config.gain.mode,
            isp_cfg->configs.dpf_config.gain.nf_b_gain,
            isp_cfg->configs.dpf_config.gain.nf_gb_gain,
            isp_cfg->configs.dpf_config.gain.nf_gr_gain,
            isp_cfg->configs.dpf_config.gain.nf_r_gain);
    }
    mIspCfg.dpf_config = isp_cfg->configs.dpf_config;
    mIspCfg.enabled[HAL_ISP_DPF_ID] =
        isp_cfg->enabled[HAL_ISP_DPF_ID];
  }

  if (isp_cfg->active_configs & ISP_DPF_STRENGTH_MASK) {
    if (!mIspIoctl->setDpfStrengthCfg(
            isp_cfg->configs.dpf_strength_config,
            isp_cfg->enabled[HAL_ISP_DPF_STRENGTH_ID])) {
      ALOGE("%s: setDpfStrengthCfg failed", __func__);
    }
    mIspCfg.dpf_strength_config = isp_cfg->configs.dpf_strength_config;
    mIspCfg.enabled[HAL_ISP_DPF_STRENGTH_ID] =
        isp_cfg->enabled[HAL_ISP_DPF_STRENGTH_ID];
  }

  if (isp_cfg->active_configs & ISP_WDR_MASK) {
    if (!mIspIoctl->setWdrCfg(
            isp_cfg->configs.wdr_config,
            isp_cfg->enabled[HAL_ISP_WDR_ID])) {
      ALOGE("%s: setWdrCfg failed", __func__);
    }
    mIspCfg.wdr_config = isp_cfg->configs.wdr_config;
    mIspCfg.enabled[HAL_ISP_WDR_ID] =
        isp_cfg->enabled[HAL_ISP_WDR_ID];
    //TRACE_D(1,"WDR enabled %d",isp_cfg->enabled[HAL_ISP_WDR_ID]);
    //for (int i = 0;i < 48; i++)
    //  TRACE_D(1,"WDR[%d] = 0x%x",i,isp_cfg->configs.wdr_config.c_wdr[i]);
  }

  if (isp_cfg->active_configs & ISP_DSP_3DNR_MASK) {
    mIspCfg.Dsp3DnrSetConfig = isp_cfg->configs.Dsp3DnrSetConfig;
  }
  
  if (isp_cfg->active_configs & ISP_NEW_DSP_3DNR_MASK) {
    mIspCfg.NewDsp3DnrSetConfig = isp_cfg->configs.NewDsp3DnrSetConfig;
  }

  if (isp_cfg->active_configs & ISP_DSP_AWB_LSC_MASK) {
    if (isp_cfg->enabled[HAL_ISP_AWB_LSC_ID] == BOOL_FALSE) {
      if (!mIspIoctl->setLscCfg(
          isp_cfg->configs.lsc_config,
          isp_cfg->enabled[HAL_ISP_AWB_LSC_ID])) {
        ALOGE("%s: setLscCfg failed", __func__);
      }
    } else {
      mCamIAEngine->closeAWB();
    }
  }

  if (isp_cfg->active_configs & ISP_AWB_CCM_MASK) {
    if (isp_cfg->enabled[HAL_ISP_AWB_CCM_ID] == BOOL_FALSE) {
      if (!mIspIoctl->setCtkCfg(
          isp_cfg->configs.ctk_config,
          isp_cfg->enabled[HAL_ISP_AWB_CCM_ID])) {
        ALOGE("%s: setCtkCfg failed", __func__);
      }
    } else {
      mCamIAEngine->closeAWB();
    }
  }

  if (isp_cfg->active_configs & ISP_AWB_CURVE_MASK) {
    if (isp_cfg->enabled[HAL_ISP_AWB_CURVE_ID] == BOOL_TRUE) {
      mCamIAEngine->closeAWB();
    }
  }

  if (isp_cfg->active_configs & ISP_AWB_WP_SET_MASK) {
    if (isp_cfg->enabled[HAL_ISP_AWB_WP_SET_ID] == BOOL_TRUE) {
      mCamIAEngine->closeAWB();
    }
  }

  if (isp_cfg->active_configs & ISP_ADPF_DPF_MASK) {
    if (isp_cfg->enabled[HAL_ISP_ADPF_DPF_ID] == BOOL_FALSE) {
      if (!mIspIoctl->setDpfCfg(
          isp_cfg->configs.dpf_config,
          isp_cfg->enabled[HAL_ISP_ADPF_DPF_ID])) {
        ALOGE("%s: setDpfCfg failed", __func__);
      }

      if (!mIspIoctl->setDpfStrengthCfg(
          isp_cfg->configs.dpf_strength_config,
          isp_cfg->enabled[HAL_ISP_ADPF_DPF_ID])) {
        ALOGE("%s: setDpfStrengthCfg failed", __func__);
      }
    } else {
      mCamIAEngine->closeADPF();
    }
  }

  if (isp_cfg->active_configs & ISP_ADPF_FLT_MASK) {
    if (isp_cfg->enabled[HAL_ISP_ADPF_FLT_ID] == BOOL_FALSE) {
      if (!mIspIoctl->setFltCfg(
          isp_cfg->configs.flt_config,
          isp_cfg->enabled[HAL_ISP_ADPF_FLT_ID])) {
        ALOGE("%s: setFltCfg failed", __func__);
      }
    } else {
      mCamIAEngine->closeADPF();
    }
  }
  return true;
}


bool CamIsp11CtrItf::convertIspStats(
    struct cifisp_stat_buffer* isp_stats,
    struct CamIA10_Stats* ia_stats) {
  unsigned int i;

  m3AMeasType = HAL_3A_MEAS_INVALID;
  if (isp_stats->meas_type & (CIFISP_STAT_AUTOEXP |
    CIFISP_STAT_HIST |
    CIFISP_STAT_AWB |
    CIFISP_STAT_AFM_FIN)) {
    mFrmStartTime = ((int64_t)isp_stats->vs_t.tv_sec * 1000) + isp_stats->vs_t.tv_usec / 1000;    
    mFrmEndTime = ((int64_t)isp_stats->fi_t.tv_sec * 1000) + isp_stats->fi_t.tv_usec / 1000;
  }

  if (isp_stats->meas_type & CIFISP_STAT_AUTOEXP) {
    m3AMeasType = HAL_3A_MEAS_AEAWB;
    ia_stats->meas_type |= CAMIA10_AEC_MASK;
    memcpy(ia_stats->aec.exp_mean,
           isp_stats->params.ae.exp_mean,
           sizeof(ia_stats->aec.exp_mean));
    
    /*ALOGD("> AE Measurement:\n");
    for (i = 0; i < CIFISP_AE_MEAN_MAX; i += 5) {
      ALOGD(">     Exposure means %d-%d: %d, %d, %d, %d, %d\n",i, i+4,
        isp_stats->params.ae.exp_mean[i],
        isp_stats->params.ae.exp_mean[i + 1],
        isp_stats->params.ae.exp_mean[i + 2],
        isp_stats->params.ae.exp_mean[i + 3],
        isp_stats->params.ae.exp_mean[i + 4]);

    }*/
  }

  if (isp_stats->meas_type & CIFISP_STAT_HIST) {
    ia_stats->meas_type |= CAMIA10_HST_MASK;
    memcpy(ia_stats->aec.hist_bins,
           isp_stats->params.hist.hist_bins,
           sizeof(ia_stats->aec.hist_bins));
    /*
    for (int i=0; i<CIFISP_HIST_BIN_N_MAX; i+=4) {
      ALOGD("histogram > %d-%d-%d-%d: %d-%d-%d-%d \n",
          i, i+1, i+2, i+3,
          isp_stats->params.hist.hist_bins[i],
          isp_stats->params.hist.hist_bins[i+1],
          isp_stats->params.hist.hist_bins[i+2],
          isp_stats->params.hist.hist_bins[i+3]);
    }
    */
  }

  if (isp_stats->meas_type & CIFISP_STAT_AWB) {
    ia_stats->meas_type |= CAMIA10_AWB_MEAS_MASK;
    if (mIspCfg.awb_meas_config.awb_mode == CIFISP_AWB_MODE_YCBCR) {
      ia_stats->awb.NoWhitePixel = isp_stats->params.awb.awb_mean[0].cnt;
      ia_stats->awb.MeanY__G     = isp_stats->params.awb.awb_mean[0].mean_y;
      ia_stats->awb.MeanCb__B    = isp_stats->params.awb.awb_mean[0].mean_cb;
      ia_stats->awb.MeanCr__R    = isp_stats->params.awb.awb_mean[0].mean_cr;
    } else if (mIspCfg.awb_meas_config.awb_mode == CIFISP_AWB_MODE_RGB) {
      ia_stats->awb.NoWhitePixel = isp_stats->params.awb.awb_mean[0].cnt;
      ia_stats->awb.MeanY__G     = isp_stats->params.awb.awb_mean[0].mean_g;
      ia_stats->awb.MeanCb__B    = isp_stats->params.awb.awb_mean[0].mean_b;
      ia_stats->awb.MeanCr__R    = isp_stats->params.awb.awb_mean[0].mean_r;
    } else {
      memset(&ia_stats->awb, 0, sizeof(ia_stats->awb));
    }
  }

  if (isp_stats->meas_type & CIFISP_STAT_AFM_FIN) {
    int64_t vcm_start_t;
    int64_t vcm_end_t;
    uint32_t tshift = 0U;
    uint32_t lshift = 0U;

    vcm_start_t = ((int64_t)isp_stats->subdev_stat.vcm.vcm_start_t.tv_sec * 1000) +
        isp_stats->subdev_stat.vcm.vcm_start_t.tv_usec / 1000;
    vcm_end_t = ((int64_t)isp_stats->subdev_stat.vcm.vcm_end_t.tv_sec * 1000) +
        isp_stats->subdev_stat.vcm.vcm_end_t.tv_usec / 1000;

    if (vcm_end_t <= mFrmStartTime) {
      ia_stats->af.cameric.MoveRes = CAMERIC_ISP_VCM_MOVE_END;
    } else {
      ia_stats->af.cameric.MoveRes = CAMERIC_ISP_VCM_MOVE_RUNNING;
    }
    /*ALOGD("mFrmStartTime: %lldms  vcm start: %lldms end: %lldms, %lds move: %d\n",
		  mFrmStartTime,
		  vcm_start_t,
		  vcm_end_t, isp_stats->subdev_stat.vcm.vcm_end_t.tv_sec,
		  (int)ia_stats->af.cameric.MoveRes);*/
    if (m3AMeasType == HAL_3A_MEAS_AEAWB) {
      m3AMeasType = HAL_3A_MEAS_AEAWBAF;
    } else {
      m3AMeasType = HAL_3A_MEAS_AF;
    }

    ia_stats->meas_type |= CAMIA10_AFC_MASK;
    ia_stats->af.meas_type |= AF_TYPE_CONTRAST;
    lshift = ((mIspCfg.afc_config.var_shift & 0xff0000) >> 16);
    ia_stats->af.cameric.SharpnessA = 
      isp_stats->params.af.window[0].sum;
    ia_stats->af.cameric.LuminanceA =
      (isp_stats->params.af.window[0].lum << lshift);
    ia_stats->af.cameric.SharpnessB =
      isp_stats->params.af.window[1].sum;
    ia_stats->af.cameric.LuminanceB =
      (isp_stats->params.af.window[1].lum << lshift);
    ia_stats->af.cameric.SharpnessC =
      isp_stats->params.af.window[2].sum;
    ia_stats->af.cameric.LuminanceC =
      (isp_stats->params.af.window[2].lum << lshift);

    /*for (int i=0; i<CIFISP_AFM_MAX_WINDOWS; i++) {
      ALOGD("Af window[%d]: lum=0x%x, sum=0x%x\n", i,
          isp_stats->params.af.window[i].lum,
          isp_stats->params.af.window[i].sum);
    }*/
  }

  //ALOGD("%s: meas packet type %d, %d", __func__, m3AMeasType, ia_stats->af.MoveRes);
  return true;
}

bool CamIsp11CtrItf::configureISP(const void* config) {
  bool ret = CamIsp1xCtrItf::configureISP(config);
  if (ret) {
    if (mCamIA_DyCfg.uc == UC_RAW) {
      struct CamIA10_Results ia_results = {0};
      struct CamIsp11ConfigSet isp_cfg = {0};
      //run isp manual config?will override the 3A results
      if (!runISPManual(&ia_results, BOOL_TRUE))
        ALOGE("%s:run ISP manual failed!", __func__);
      convertIAResults(&isp_cfg, &ia_results);
      applyIspConfig(&isp_cfg);
    }
  }

  return ret;
}

bool CamIsp11CtrItf::convertIAResults(
    struct CamIsp11ConfigSet* isp_cfg,
    struct CamIA10_Results* ia_results) {
  unsigned int i;
  unsigned int isp_ref_width;
  unsigned int isp_ref_height;
  int newTime;
  int newGain;
  int newVts;

  if (isp_cfg == NULL)
    return false;

  if (mCamIA_DyCfg.sensor_mode.isp_output_width != 0) {
    isp_ref_width = mCamIA_DyCfg.sensor_mode.isp_output_width;
    isp_ref_height = mCamIA_DyCfg.sensor_mode.isp_output_height;
  } else {
    isp_ref_width = mCamIA_DyCfg.sensor_mode.isp_input_width;
    isp_ref_height = mCamIA_DyCfg.sensor_mode.isp_input_height;
  }

  isp_cfg->active_configs = 0;
  if (ia_results) {
    if (mRefWidth != isp_ref_width ||
        mRefHeight != isp_ref_height) {
        ia_results->aec.actives |= CAMIA10_AEC_MASK;
        mRefWidth = isp_ref_width;
        mRefHeight = isp_ref_height;
    }
    if ((ia_results->active & CAMIA10_AEC_MASK)
        || (ia_results->active & CAMIA10_HST_MASK)
        || (ia_results->active & CAMIA10_AEC_AFPS_MASK)) {
      if (ia_results->aec.actives & CAMIA10_AEC_MASK) {
        /*ae enable or manual exposure*/
        if (ia_results->aec_enabled) {
          for (i = 0; i < ia_results->aec.exp_set_cnt; i++) {
            newTime = ia_results->aec.exp_set[i].regTime;
            newGain = ia_results->aec.exp_set[i].regGain;
            newVts = ia_results->aec.exp_set[i].vts;

            //ALOGD("set auto exposure regtime: %d, reggain: %d, time:%f gain:%f pcf: %f, pppl: %d",
            //      newTime, newGain, ia_results->aec.coarse_integration_time,
            //      ia_results->aec.analog_gain_code_global,
            //      mCamIA_DyCfg.sensor_mode.pixel_clock_freq_mhz,
            //      mCamIA_DyCfg.sensor_mode.pixel_periods_per_line);
            mCamHwItf->setExposure(newVts, newTime, newGain, 100);
          }
        } else if (((ia_results->aec.regIntegrationTime > 0) ||
             (ia_results->aec.regGain > 0))) {
            newTime = ia_results->aec.regIntegrationTime;
            newGain = ia_results->aec.regGain;
            newVts = ia_results->aec.LinePeriodsPerField;

            //ALOGD("set manual exposure regtime: %d, reggain: %d, time:%f gain:%f pcf: %f, pppl: %d",
            //      newTime, newGain, ia_results->aec.coarse_integration_time,
            //      ia_results->aec.analog_gain_code_global,
            //      mCamIA_DyCfg.sensor_mode.pixel_clock_freq_mhz,
            //      mCamIA_DyCfg.sensor_mode.pixel_periods_per_line);
            mCamHwItf->setExposure(newVts, newTime, newGain, 100);
            ia_results->aec_enabled = BOOL_TRUE;
        }

        AecMeasuringMode_to_cifisp_exp_meas_mode(
            ia_results->aec.meas_mode,
            &isp_cfg->configs.aec_config.mode);
        isp_cfg->configs.aec_config.autostop =
            CIFISP_EXP_CTRL_AUTOSTOP_0;

        mCamIAEngine->mapHalWinToIsp(ia_results->aec.meas_win.h_size,
                       ia_results->aec.meas_win.v_size,
                       ia_results->aec.meas_win.h_offs,
                       ia_results->aec.meas_win.v_offs,
                       isp_ref_width,
                       isp_ref_height,
                       isp_cfg->configs.aec_config.meas_window.h_size,
                       isp_cfg->configs.aec_config.meas_window.v_size,
                       isp_cfg->configs.aec_config.meas_window.h_offs,
                       isp_cfg->configs.aec_config.meas_window.v_offs);

        isp_cfg->active_configs |= ISP_AEC_MASK;
        isp_cfg->enabled[HAL_ISP_AEC_ID] = ia_results->aec_enabled;
        TRACE_D(1, "%s:aec mode : %d", __func__,
                ia_results->aec.meas_mode);

        mAeStableCnt = 0;
		mAeState = HAL_AE_STATE_UNSTABLE;
      } else {
        if (mAeStableCnt < AE_STABLE_COUNT_NUM) {
          mAeStableCnt++;
        } else {
		  mAeState = HAL_AE_STATE_STABLE;
        }
      }
      if (
          ia_results->aec.actives & CAMIA10_HST_MASK) {
        isp_cfg->active_configs |= ISP_HST_MASK;
        isp_cfg->enabled[HAL_ISP_HST_ID] = ia_results->hst.enabled;
        isp_cfg->configs.hst_config.mode = (cifisp_histogram_mode)
                                           (ia_results->hst.mode);

        mCamIAEngine->mapHalWinToIsp(ia_results->hst.Window.width,
                       ia_results->hst.Window.height,
                       ia_results->hst.Window.hOffset,
                       ia_results->hst.Window.vOffset,
                       isp_ref_width,
                       isp_ref_height,
                       isp_cfg->configs.hst_config.meas_window.h_size,
                       isp_cfg->configs.hst_config.meas_window.v_size,
                       isp_cfg->configs.hst_config.meas_window.h_offs,
                       isp_cfg->configs.hst_config.meas_window.v_offs);
        memcpy(isp_cfg->configs.hst_config.hist_weight,
               ia_results->hst.Weights, sizeof(ia_results->hst.Weights));
        TRACE_D(1, "step size: %d, w-h: %d-%d\n",
                ia_results->aec.stepSize, isp_cfg->configs.hst_config.meas_window.h_size,
                isp_cfg->configs.hst_config.meas_window.v_size);
        isp_cfg->configs.hst_config.histogram_predivider = ia_results->hst.StepSize;

      }

	  if(ia_results->aec.actives & CAMIA10_AEC_AFPS_MASK)
	  {
		//ioctl auto adjust afps to kernel to sensor driver
		CamIsp11DevHwItf *CamIsp11HwItf = dynamic_cast<CamIsp11DevHwItf *>(mCamHwItf);
		CamIsp11HwItf->setAutoAdjustFps(ia_results->aec.auto_adjust_fps);
	  }
    }

    if (ia_results->active & CAMIA10_AWB_GAIN_MASK) {
      isp_cfg->configs.awb_gain_config.gain_blue =
          ia_results->awb.awbGains.Blue;
      isp_cfg->configs.awb_gain_config.gain_green_b =
          ia_results->awb.awbGains.GreenB;
      isp_cfg->configs.awb_gain_config.gain_green_r =
          ia_results->awb.awbGains.GreenR;
      isp_cfg->configs.awb_gain_config.gain_red =
          ia_results->awb.awbGains.Red;
      isp_cfg->active_configs |= ISP_AWB_GAIN_MASK;
      isp_cfg->enabled[HAL_ISP_AWB_GAIN_ID] =
          ia_results->awb_gains_enabled;
      TRACE_D(1, "AWB GAIN : enabled %d,BGbGrR(%d,%d,%d,%d)",
              isp_cfg->enabled[HAL_ISP_AWB_GAIN_ID],
              isp_cfg->configs.awb_gain_config.gain_blue,
              isp_cfg->configs.awb_gain_config.gain_green_b,
              isp_cfg->configs.awb_gain_config.gain_green_r,
              isp_cfg->configs.awb_gain_config.gain_red);
    }

    if (ia_results->active & CAMIA10_CTK_MASK) {
      //if (ia_results->awb.actives & AWB_RECONFIG_CCMATRIX)
      {
        isp_cfg->configs.ctk_config.coeff0 =
            ia_results->awb.CcMatrix.Coeff[0];
        isp_cfg->configs.ctk_config.coeff1 =
            ia_results->awb.CcMatrix.Coeff[1];
        isp_cfg->configs.ctk_config.coeff2 =
            ia_results->awb.CcMatrix.Coeff[2];
        isp_cfg->configs.ctk_config.coeff3 =
            ia_results->awb.CcMatrix.Coeff[3];
        isp_cfg->configs.ctk_config.coeff4 =
            ia_results->awb.CcMatrix.Coeff[4];
        isp_cfg->configs.ctk_config.coeff5 =
            ia_results->awb.CcMatrix.Coeff[5];
        isp_cfg->configs.ctk_config.coeff6 =
            ia_results->awb.CcMatrix.Coeff[6];
        isp_cfg->configs.ctk_config.coeff7 =
            ia_results->awb.CcMatrix.Coeff[7];
        isp_cfg->configs.ctk_config.coeff8 =
            ia_results->awb.CcMatrix.Coeff[8];
        isp_cfg->active_configs |= ISP_CTK_MASK;
        isp_cfg->enabled[HAL_ISP_CTK_ID] =
            ia_results->ctk_enabled;
        TRACE_D(1, "AWB CTK COEFF: enabled %d",
                isp_cfg->enabled[HAL_ISP_CTK_ID]);
        for (int i = 0; i < 9; i++)
          TRACE_D(1, "-->COEFF[%d]:%d", i,
                  ia_results->awb.CcMatrix.Coeff[i]);
      }

      //if (ia_results->awb.actives & AWB_RECONFIG_CCOFFSET)
      {
        isp_cfg->configs.ctk_config.ct_offset_b =
            ia_results->awb.CcOffset.Blue;
        isp_cfg->configs.ctk_config.ct_offset_g =
            ia_results->awb.CcOffset.Green;
        isp_cfg->configs.ctk_config.ct_offset_r =
            ia_results->awb.CcOffset.Red;
        isp_cfg->active_configs |= ISP_CTK_MASK;
        isp_cfg->enabled[HAL_ISP_CTK_ID] =
            ia_results->ctk_enabled;
        TRACE_D(1, "AWB CTK OFFSET: BGR(%d,%d,%d)",
                isp_cfg->configs.ctk_config.ct_offset_b,
                isp_cfg->configs.ctk_config.ct_offset_g,
                isp_cfg->configs.ctk_config.ct_offset_r);
      }
    }

    if (ia_results->active & CAMIA10_LSC_MASK) {
      //if (ia_results->awb.actives & AWB_RECONFIG_LSCMATRIX)
      {
        for (i = 0; i < CIFISP_LSC_DATA_TBL_SIZE; i++)
          isp_cfg->configs.lsc_config.r_data_tbl[i] =
              ia_results->awb.LscMatrixTable.LscMatrix[0].uCoeff[i];
        for (i = 0; i < CIFISP_LSC_DATA_TBL_SIZE; i++)
          isp_cfg->configs.lsc_config.gr_data_tbl[i] =
              ia_results->awb.LscMatrixTable.LscMatrix[1].uCoeff[i];
        for (i = 0; i < CIFISP_LSC_DATA_TBL_SIZE; i++)
          isp_cfg->configs.lsc_config.gb_data_tbl[i] =
              ia_results->awb.LscMatrixTable.LscMatrix[2].uCoeff[i];
        for (i = 0; i < CIFISP_LSC_DATA_TBL_SIZE; i++)
          isp_cfg->configs.lsc_config.b_data_tbl[i] =
              ia_results->awb.LscMatrixTable.LscMatrix[3].uCoeff[i];
        isp_cfg->active_configs |= ISP_LSC_MASK;
        isp_cfg->enabled[HAL_ISP_LSC_ID] = ia_results->lsc_enabled;
      }
      //if (ia_results->awb.actives & AWB_RECONFIG_LSCSECTOR)
      {
        for (i = 0; i < CIFISP_LSC_GRAD_TBL_SIZE; i++)
          isp_cfg->configs.lsc_config.x_grad_tbl[i] =
              ia_results->awb.SectorConfig.LscXGradTbl[i];
        for (i = 0; i < CIFISP_LSC_SIZE_TBL_SIZE; i++)
          isp_cfg->configs.lsc_config.x_size_tbl[i] =
              ia_results->awb.SectorConfig.LscXSizeTbl[i];
        for (i = 0; i < CIFISP_LSC_GRAD_TBL_SIZE; i++)
          isp_cfg->configs.lsc_config.y_grad_tbl[i] =
              ia_results->awb.SectorConfig.LscYGradTbl[i];
        for (i = 0; i < CIFISP_LSC_SIZE_TBL_SIZE; i++)
          isp_cfg->configs.lsc_config.y_size_tbl[i] =
              ia_results->awb.SectorConfig.LscYSizeTbl[i];
        isp_cfg->active_configs |= ISP_LSC_MASK;
        isp_cfg->enabled[HAL_ISP_LSC_ID] = ia_results->lsc_enabled;
      }
      //TODO: set size
      isp_cfg->configs.lsc_config.config_width =
          mCamIA_DyCfg.sensor_mode.isp_input_width;
      isp_cfg->configs.lsc_config.config_height =
          mCamIA_DyCfg.sensor_mode.isp_input_height;

      TRACE_D(1, "AWB LSC: size(%dx%d),enabled %d",
              isp_cfg->configs.lsc_config.config_width,
              isp_cfg->configs.lsc_config.config_height,
              isp_cfg->enabled[HAL_ISP_LSC_ID]);
    }

    if (ia_results->active & CAMIA10_AWB_MEAS_MASK) {

      //if (ia_results->awb.actives & AWB_RECONFIG_MEASMODE)
      {
        if (ia_results->awb.MeasMode == CAMERIC_ISP_AWB_MEASURING_MODE_INVALID)
          isp_cfg->configs.awb_meas_config.awb_mode = CIFISP_AWB_MODE_MANUAL;
        else if (ia_results->awb.MeasMode == CAMERIC_ISP_AWB_MEASURING_MODE_YCBCR)
          isp_cfg->configs.awb_meas_config.awb_mode = CIFISP_AWB_MODE_YCBCR;
        else if (ia_results->awb.MeasMode == CAMERIC_ISP_AWB_MEASURING_MODE_RGB)
          isp_cfg->configs.awb_meas_config.awb_mode = CIFISP_AWB_MODE_RGB;
        else
          ALOGE("%s:%d,erro awb measure mode %d", __func__, __LINE__, ia_results->awb.MeasMode);
        isp_cfg->active_configs |= ISP_AWB_MEAS_MASK;
        isp_cfg->enabled[HAL_ISP_AWB_MEAS_ID] =
            ia_results->awb_meas_enabled;
        TRACE_D(1, "AWB MeasMode : %d,enabled: %d ",
                isp_cfg->configs.awb_meas_config.awb_mode,
                isp_cfg->enabled[HAL_ISP_AWB_MEAS_ID]);
      }
      //if (ia_results->awb.actives & AWB_RECONFIG_MEASCFG)
      {
        isp_cfg->configs.awb_meas_config.max_csum =
            ia_results->awb.MeasConfig.MaxCSum;
        isp_cfg->configs.awb_meas_config.max_y =
            ia_results->awb.MeasConfig.MaxY;
        isp_cfg->configs.awb_meas_config.min_y =
            ia_results->awb.MeasConfig.MinY_MaxG;
        isp_cfg->configs.awb_meas_config.min_c =
            ia_results->awb.MeasConfig.MinC;
        isp_cfg->configs.awb_meas_config.awb_ref_cr =
            ia_results->awb.MeasConfig.RefCr_MaxR;
        isp_cfg->configs.awb_meas_config.awb_ref_cb =
            ia_results->awb.MeasConfig.RefCb_MaxB;
        isp_cfg->active_configs |= ISP_AWB_MEAS_MASK;
        isp_cfg->enabled[HAL_ISP_AWB_MEAS_ID] =
            ia_results->awb_meas_enabled;
        TRACE_D(1, "AWB MEASCFG :");
        TRACE_D(1, "-->max_csum:%d,max_y:%d,min_y:%d,MinC:%d,awb_ref_cr:%d,awb_ref_cb:%d",
                isp_cfg->configs.awb_meas_config.max_csum,
                isp_cfg->configs.awb_meas_config.max_y,
                isp_cfg->configs.awb_meas_config.min_y,
                isp_cfg->configs.awb_meas_config.min_c,
                isp_cfg->configs.awb_meas_config.awb_ref_cr,
                isp_cfg->configs.awb_meas_config.awb_ref_cb
               );
      }
      //if (ia_results->awb.actives & AWB_RECONFIG_AWBWIN)
      {
        mCamIAEngine->mapHalWinToIsp(ia_results->awb.awbWin.h_size,
                       ia_results->awb.awbWin.v_size,
                       ia_results->awb.awbWin.h_offs,
                       ia_results->awb.awbWin.v_offs,
                       isp_ref_width,
                       isp_ref_height,
                       isp_cfg->configs.awb_meas_config.awb_wnd.h_size,
                       isp_cfg->configs.awb_meas_config.awb_wnd.v_size,
                       isp_cfg->configs.awb_meas_config.awb_wnd.h_offs,
                       isp_cfg->configs.awb_meas_config.awb_wnd.v_offs);

        isp_cfg->active_configs |= ISP_AWB_MEAS_MASK;
        isp_cfg->enabled[HAL_ISP_AWB_MEAS_ID] =
            ia_results->awb_meas_enabled;
        TRACE_D(1, "AWB WINDOW:");
        TRACE_D(1, "-->awb win:size:%dx%d(off:%dx%d)",
                isp_cfg->configs.awb_meas_config.awb_wnd.h_size,
                isp_cfg->configs.awb_meas_config.awb_wnd.v_size,
                isp_cfg->configs.awb_meas_config.awb_wnd.h_offs,
                isp_cfg->configs.awb_meas_config.awb_wnd.v_offs);
      }
      //TODO:

      isp_cfg->configs.awb_meas_config.frames = /*CIFISP_AWB_MAX_FRAMES*/0;
      TRACE_D(1, "AWB FRAMES:%d", isp_cfg->configs.awb_meas_config.frames);
      //isp_cfg->configs.awb_meas_config.enable_ymax_cmp
    }
    if (ia_results->active &
        (CAMIA10_DPF_MASK | CAMIA10_DPF_STRENGTH_MASK)) {

      if (ia_results->active & CAMIA10_DPF_MASK) {
        isp_cfg->configs.dpf_config.gain.mode =
            (enum cifisp_dpf_gain_usage)ia_results->adpf.DpfMode.GainUsage;
        isp_cfg->configs.dpf_config.gain.nf_b_gain =
            ia_results->adpf.NfGains.Blue;
        isp_cfg->configs.dpf_config.gain.nf_gr_gain =
            ia_results->adpf.NfGains.GreenR;
        isp_cfg->configs.dpf_config.gain.nf_gb_gain =
            ia_results->adpf.NfGains.GreenB;
        isp_cfg->configs.dpf_config.gain.nf_r_gain =
            ia_results->adpf.NfGains.Red;

        for (i = 0; i < CIFISP_DPF_MAX_NLF_COEFFS; i++) {
          isp_cfg->configs.dpf_config.nll.coeff[i] =
              ia_results->adpf.Nll.NllCoeff[i];
        }
        isp_cfg->configs.dpf_config.nll.scale_mode =
            (enum cifisp_dpf_nll_scale_mode)ia_results->adpf.Nll.xScale;

        isp_cfg->configs.dpf_config.g_flt.gb_enable =
            ia_results->adpf.DpfMode.ProcessGreenBPixel;
        isp_cfg->configs.dpf_config.g_flt.gr_enable =
            ia_results->adpf.DpfMode.ProcessGreenRPixel;
        isp_cfg->configs.dpf_config.rb_flt.r_enable =
            ia_results->adpf.DpfMode.ProcessRedPixel;
        isp_cfg->configs.dpf_config.rb_flt.b_enable =
            ia_results->adpf.DpfMode.ProcessBluePixel;
        isp_cfg->configs.dpf_config.rb_flt.fltsize =
            (enum cifisp_dpf_rb_filtersize)ia_results->adpf.DpfMode.RBFilterSize;
        for (i = 0; i < CIFISP_DPF_MAX_SPATIAL_COEFFS; i++) {
          isp_cfg->configs.dpf_config.g_flt.spatial_coeff[i] =
              ia_results->adpf.DpfMode.SpatialG.WeightCoeff[i];
          isp_cfg->configs.dpf_config.rb_flt.spatial_coeff[i] =
              ia_results->adpf.DpfMode.SpatialRB.WeightCoeff[i];
        }

        TRACE_D(1, "%s: Gain: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x",
                __func__,
                ia_results->adpf.DpfMode.GainUsage,
                ia_results->adpf.NfGains.Blue,
                ia_results->adpf.NfGains.GreenB,
                ia_results->adpf.NfGains.GreenR,
                ia_results->adpf.NfGains.Red);
        isp_cfg->active_configs |= ISP_DPF_MASK;
        isp_cfg->enabled[HAL_ISP_DPF_ID] =
            ia_results->adpf_enabled;
      }

      if (ia_results->active & CAMIA10_DPF_STRENGTH_MASK) {
        isp_cfg->configs.dpf_strength_config.b =
            ia_results->adpf.DynInvStrength.WeightB;
        isp_cfg->configs.dpf_strength_config.g =
            ia_results->adpf.DynInvStrength.WeightG;
        isp_cfg->configs.dpf_strength_config.r =
            ia_results->adpf.DynInvStrength.WeightR;
        isp_cfg->active_configs |= ISP_DPF_STRENGTH_MASK;
        isp_cfg->enabled[HAL_ISP_DPF_STRENGTH_ID] =
            ia_results->adpf_strength_enabled;
      }
    }

    if (ia_results->active & CAMIA10_BPC_MASK) {
      isp_cfg->configs.dpcc_config.mode  = ia_results->dpcc.isp_dpcc_mode;
      isp_cfg->configs.dpcc_config.output_mode = ia_results->dpcc.isp_dpcc_output_mode;
      isp_cfg->configs.dpcc_config.set_use = ia_results->dpcc.isp_dpcc_set_use;
      isp_cfg->configs.dpcc_config.ro_limits = ia_results->dpcc.isp_dpcc_ro_limits;
      isp_cfg->configs.dpcc_config.rnd_offs = ia_results->dpcc.isp_dpcc_rnd_offs;
      isp_cfg->configs.dpcc_config.methods[0].line_mad_fac =
          ia_results->dpcc.isp_dpcc_line_mad_fac_1;
      isp_cfg->configs.dpcc_config.methods[0].line_thresh =
          ia_results->dpcc.isp_dpcc_line_thresh_1;
      isp_cfg->configs.dpcc_config.methods[0].method  =
          ia_results->dpcc.isp_dpcc_methods_set_1;
      isp_cfg->configs.dpcc_config.methods[0].pg_fac =
          ia_results->dpcc.isp_dpcc_pg_fac_1;
      isp_cfg->configs.dpcc_config.methods[0].rnd_thresh =
          ia_results->dpcc.isp_dpcc_rnd_thresh_1;
      isp_cfg->configs.dpcc_config.methods[0].rg_fac =
          ia_results->dpcc.isp_dpcc_rg_fac_1;

      isp_cfg->configs.dpcc_config.methods[1].line_mad_fac =
          ia_results->dpcc.isp_dpcc_line_mad_fac_2;
      isp_cfg->configs.dpcc_config.methods[1].line_thresh =
          ia_results->dpcc.isp_dpcc_line_thresh_2;
      isp_cfg->configs.dpcc_config.methods[1].method  =
          ia_results->dpcc.isp_dpcc_methods_set_2;
      isp_cfg->configs.dpcc_config.methods[1].pg_fac =
          ia_results->dpcc.isp_dpcc_pg_fac_2;
      isp_cfg->configs.dpcc_config.methods[1].rnd_thresh =
          ia_results->dpcc.isp_dpcc_rnd_thresh_2;
      isp_cfg->configs.dpcc_config.methods[1].rg_fac =
          ia_results->dpcc.isp_dpcc_rg_fac_2;

      isp_cfg->configs.dpcc_config.methods[2].line_mad_fac =
          ia_results->dpcc.isp_dpcc_line_mad_fac_3;
      isp_cfg->configs.dpcc_config.methods[2].line_thresh =
          ia_results->dpcc.isp_dpcc_line_thresh_3;
      isp_cfg->configs.dpcc_config.methods[2].method  =
          ia_results->dpcc.isp_dpcc_methods_set_3;
      isp_cfg->configs.dpcc_config.methods[2].pg_fac =
          ia_results->dpcc.isp_dpcc_pg_fac_3;
      isp_cfg->configs.dpcc_config.methods[2].rnd_thresh =
          ia_results->dpcc.isp_dpcc_rnd_thresh_3;
      isp_cfg->configs.dpcc_config.methods[2].rg_fac =
          ia_results->dpcc.isp_dpcc_rg_fac_3;

      isp_cfg->enabled[HAL_ISP_BPC_ID] = ia_results->dpcc.enabled;
      isp_cfg->active_configs |=  ISP_BPC_MASK;
    }

    if (ia_results->active & CAMIA10_BLS_MASK) {
      isp_cfg->configs.bls_config.enable_auto =
          ia_results->bls.mode == 0 ? false : true;
      isp_cfg->configs.bls_config.bls_samples =
          ia_results->bls.samples;
      isp_cfg->configs.bls_config.bls_window1.h_offs =
          ia_results->bls.Window1.hOffset;
      isp_cfg->configs.bls_config.bls_window1.v_offs =
          ia_results->bls.Window1.vOffset;
      isp_cfg->configs.bls_config.bls_window1.h_size =
          ia_results->bls.Window1.width;
      isp_cfg->configs.bls_config.bls_window1.v_size =
          ia_results->bls.Window1.height;
      isp_cfg->configs.bls_config.bls_window2.h_offs =
          ia_results->bls.Window2.hOffset;
      isp_cfg->configs.bls_config.bls_window2.v_offs =
          ia_results->bls.Window2.vOffset;
      isp_cfg->configs.bls_config.bls_window2.h_size =
          ia_results->bls.Window2.width;
      isp_cfg->configs.bls_config.bls_window2.v_size =
          ia_results->bls.Window2.height;

      isp_cfg->configs.bls_config.en_windows =
          ia_results->bls.num_win;

      /* red */
      isp_cfg->configs.bls_config.fixed_val.r =
          ia_results->bls.isp_bls_a_fixed;
      /* greenR*/
      isp_cfg->configs.bls_config.fixed_val.gr =
          ia_results->bls.isp_bls_b_fixed;
      /* greenB*/
      isp_cfg->configs.bls_config.fixed_val.gb =
          ia_results->bls.isp_bls_c_fixed;
      /* blue*/
      isp_cfg->configs.bls_config.fixed_val.b =
          ia_results->bls.isp_bls_d_fixed;

      TRACE_D(1, "BLS: RGrGbB:%d,%d,%d,%d",
              isp_cfg->configs.bls_config.fixed_val.r,
              isp_cfg->configs.bls_config.fixed_val.gr,
              isp_cfg->configs.bls_config.fixed_val.gb,
              isp_cfg->configs.bls_config.fixed_val.b);

      isp_cfg->enabled[HAL_ISP_BLS_ID] = ia_results->bls.enabled;
      isp_cfg->active_configs |=  ISP_BLS_MASK;
    }

    if (ia_results->active & CAMIA10_SDG_MASK) {
      int i = 0;
      for (i = 0; i < CIFISP_DEGAMMA_CURVE_SIZE; i++)
        isp_cfg->configs.sdg_config.curve_r.gamma_y[i] =
            ia_results->sdg.red[i];

      for (i = 0; i < CIFISP_DEGAMMA_CURVE_SIZE; i++)
        isp_cfg->configs.sdg_config.curve_g.gamma_y[i] =
            ia_results->sdg.green[i];

      for (i = 0; i < CIFISP_DEGAMMA_CURVE_SIZE; i++)
        isp_cfg->configs.sdg_config.curve_b.gamma_y[i] =
            ia_results->sdg.blue[i];

      isp_cfg->configs.sdg_config.xa_pnts.gamma_dx0 = 0;
      isp_cfg->configs.sdg_config.xa_pnts.gamma_dx1 = 0;

      for (i = 0; i < (CIFISP_DEGAMMA_CURVE_SIZE - 1) ; i++) {
        if (i < (CIFISP_DEGAMMA_CURVE_SIZE - 1) / 2)
          isp_cfg->configs.sdg_config.xa_pnts.gamma_dx0 |=
              (uint32_t)(ia_results->sdg.segment[i]) << (i * 4);
        else {
          int index = i - (CIFISP_DEGAMMA_CURVE_SIZE - 1) / 2;
          isp_cfg->configs.sdg_config.xa_pnts.gamma_dx1 |=
              (uint32_t)(ia_results->sdg.segment[i]) << (index * 4);
        }
      }

      isp_cfg->enabled[HAL_ISP_SDG_ID] = ia_results->sdg.enabled;
      isp_cfg->active_configs |=  ISP_SDG_MASK;
    }

    if (ia_results->active & CAMIA10_FLT_MASK) {
      isp_cfg->configs.flt_config.chr_h_mode =
          ia_results->flt.chr_h_mode;
      isp_cfg->configs.flt_config.mode =
          (cifisp_flt_mode)(ia_results->flt.mode);
      isp_cfg->configs.flt_config.grn_stage1 =
          ia_results->flt.grn_stage1;
      isp_cfg->configs.flt_config.chr_v_mode =
          ia_results->flt.chr_v_mode;
      isp_cfg->configs.flt_config.thresh_bl0 =
          ia_results->flt.thresh_bl0;
      isp_cfg->configs.flt_config.thresh_bl1 =
          ia_results->flt.thresh_bl1;
      isp_cfg->configs.flt_config.thresh_sh0 =
          ia_results->flt.thresh_sh0;
      isp_cfg->configs.flt_config.thresh_sh1 =
          ia_results->flt.thresh_sh1;
      isp_cfg->configs.flt_config.lum_weight =
          ia_results->flt.lum_weight;
      isp_cfg->configs.flt_config.fac_sh1 =
          ia_results->flt.fac_sh1;
      isp_cfg->configs.flt_config.fac_sh0 =
          ia_results->flt.fac_sh0;
      isp_cfg->configs.flt_config.fac_mid =
          ia_results->flt.fac_mid;
      isp_cfg->configs.flt_config.fac_bl0 =
          ia_results->flt.fac_bl0;
      isp_cfg->configs.flt_config.fac_bl1 =
          ia_results->flt.fac_bl1;
      isp_cfg->configs.flt_denoise_level =
          ia_results->flt.denoise_level;
      isp_cfg->configs.flt_sharp_level =
          ia_results->flt.sharp_level;
      TRACE_D(1, "FLT--> \n,\
				chr_h_mode:0x%x\n \
				mode	  :0x%x\n \
				grn_stage1:0x%x\n \
				chr_v_mode:0x%x\n \
				thresh_bl0:0x%x\n \
				thresh_bl1:0x%x\n \
				thresh_sh0:0x%x\n \
				thresh_sh1:0x%x\n \
				lum_weight:0x%x\n \
				fac_sh1	  :0x%x\n \
				fac_sh0	  :0x%x\n \
				fac_mid	  :0x%x\n \
				fac_bl0	  :0x%x\n \
				fac_bl1	  :0x%x\n",
              isp_cfg->configs.flt_config.chr_h_mode,
              isp_cfg->configs.flt_config.mode,
              isp_cfg->configs.flt_config.grn_stage1,
              isp_cfg->configs.flt_config.chr_v_mode,
              isp_cfg->configs.flt_config.thresh_bl0,
              isp_cfg->configs.flt_config.thresh_bl1,
              isp_cfg->configs.flt_config.thresh_sh0,
              isp_cfg->configs.flt_config.thresh_sh1,
              isp_cfg->configs.flt_config.lum_weight,
              isp_cfg->configs.flt_config.fac_sh1,
              isp_cfg->configs.flt_config.fac_sh0,
              isp_cfg->configs.flt_config.fac_mid,
              isp_cfg->configs.flt_config.fac_bl0,
              isp_cfg->configs.flt_config.fac_bl1);
      isp_cfg->enabled[HAL_ISP_FLT_ID] = ia_results->flt.enabled;
      isp_cfg->active_configs |=  ISP_FLT_MASK;
    }

    if (ia_results->active & CAMIA10_BDM_MASK) {
      isp_cfg->configs.bdm_config.demosaic_th =
          ia_results->bdm.demosaic_th;

      isp_cfg->enabled[HAL_ISP_BDM_ID] = ia_results->bdm.enabled;
      isp_cfg->active_configs |=  ISP_BDM_MASK;
    }

    if (ia_results->active & CAMIA10_GOC_MASK) {
      if (ia_results->goc.mode  == CAMERIC_ISP_SEGMENTATION_MODE_LOGARITHMIC)
        isp_cfg->configs.goc_config.mode =
            CIFISP_GOC_MODE_LOGARITHMIC;
      else if (ia_results->goc.mode  == CAMERIC_ISP_SEGMENTATION_MODE_EQUIDISTANT)
        isp_cfg->configs.goc_config.mode =
            CIFISP_GOC_MODE_EQUIDISTANT;
      else
        ALOGE("%s: not support %d goc mode.",
              __func__, ia_results->goc.mode);
      for (int i = 0; i < CIFISP_GAMMA_OUT_MAX_SAMPLES; i++) {
        isp_cfg->configs.goc_config.gamma_y[i] =
            ia_results->goc.gamma_y.GammaY[i];
      }

      isp_cfg->enabled[HAL_ISP_GOC_ID] = ia_results->goc.enabled;
      isp_cfg->active_configs |=  ISP_GOC_MASK;
    }

    if (ia_results->active & CAMIA10_CPROC_MASK) {
      isp_cfg->configs.cproc_config.brightness =
          ia_results->cproc.brightness;
      isp_cfg->configs.cproc_config.contrast =
          ia_results->cproc.contrast;
      isp_cfg->configs.cproc_config.sat =
          ia_results->cproc.saturation;
      isp_cfg->configs.cproc_config.hue =
          ia_results->cproc.hue;
      if (ia_results->cproc.ChromaOut == CAMERIC_CPROC_CHROM_RANGE_OUT_BT601)
        isp_cfg->configs.cproc_config.c_out_range = 0;
      else if (ia_results->cproc.ChromaOut == CAMERIC_CPROC_CHROM_RANGE_OUT_FULL_RANGE)
        isp_cfg->configs.cproc_config.c_out_range = 1;
      else
       isp_cfg->configs.cproc_config.c_out_range = 0;

      if (ia_results->cproc.LumaIn == CAMERIC_CPROC_LUM_RANGE_IN_BT601)
        isp_cfg->configs.cproc_config.y_in_range = 0;
      else if (ia_results->cproc.LumaIn == CAMERIC_CPROC_LUM_RANGE_IN_FULL_RANGE)
        isp_cfg->configs.cproc_config.y_in_range = 1;
      else
        isp_cfg->configs.cproc_config.y_in_range = 0;

      if (ia_results->cproc.LumaOut== CAMERIC_CPROC_LUM_RANGE_OUT_BT601)
        isp_cfg->configs.cproc_config.y_out_range = 0;
      else if (ia_results->cproc.LumaOut == CAMERIC_CPROC_LUM_RANGE_OUT_FULL_RANGE)
        isp_cfg->configs.cproc_config.y_out_range = 1;
      else
        isp_cfg->configs.cproc_config.y_out_range = 0;

      isp_cfg->enabled[HAL_ISP_CPROC_ID] = ia_results->cproc.enabled;
      isp_cfg->active_configs |=  ISP_CPROC_MASK;
    }

    if (ia_results->active & CAMIA10_IE_MASK) {
      isp_cfg->enabled[HAL_ISP_IE_ID] = ia_results->ie.enabled;
      isp_cfg->active_configs |=  ISP_IE_MASK;
      if (ia_results->ie.enabled) {
        switch (ia_results->ie.mode) {
          case CAMERIC_IE_MODE_GRAYSCALE:
            isp_cfg->configs.ie_config.effect =
                V4L2_COLORFX_BW;
            break;
          case CAMERIC_IE_MODE_NEGATIVE:
            isp_cfg->configs.ie_config.effect =
                V4L2_COLORFX_NEGATIVE;
            break;
          case CAMERIC_IE_MODE_SEPIA: {
            isp_cfg->configs.ie_config.effect =
                V4L2_COLORFX_SEPIA;
            /*
            isp_cfg->configs.ie_config.color_sel =
              ia_results->ie.;
            isp_cfg->configs.ie_config.eff_mat_1 =
              ;
            isp_cfg->configs.ie_config.eff_mat_2 =
              ;
            isp_cfg->configs.ie_config.eff_mat_3 =
              ;
            isp_cfg->configs.ie_config.eff_mat_4 =
              ;
            isp_cfg->configs.ie_config.eff_mat_5 =
              ;
            isp_cfg->configs.ie_config.eff_tint =
              ;
            */
          }
          break;
          case CAMERIC_IE_MODE_EMBOSS: {
            isp_cfg->configs.ie_config.effect =
                V4L2_COLORFX_EMBOSS;
            isp_cfg->configs.ie_config.eff_mat_1 =
                (uint16_t)(ia_results->ie.ModeConfig.Emboss.coeff[0])
                | ((uint16_t)(ia_results->ie.ModeConfig.Emboss.coeff[1]) << 0x4)
                | ((uint16_t)(ia_results->ie.ModeConfig.Emboss.coeff[2]) << 0x8)
                | ((uint16_t)(ia_results->ie.ModeConfig.Emboss.coeff[3]) << 0xc);
            isp_cfg->configs.ie_config.eff_mat_2 =
                (uint16_t)(ia_results->ie.ModeConfig.Emboss.coeff[4])
                | ((uint16_t)(ia_results->ie.ModeConfig.Emboss.coeff[5]) << 0x4)
                | ((uint16_t)(ia_results->ie.ModeConfig.Emboss.coeff[6]) << 0x8)
                | ((uint16_t)(ia_results->ie.ModeConfig.Emboss.coeff[7]) << 0xc);
            isp_cfg->configs.ie_config.eff_mat_3 =
                (ia_results->ie.ModeConfig.Emboss.coeff[8]);
            /*not used for this effect*/
            isp_cfg->configs.ie_config.eff_mat_4 =
                0;
            isp_cfg->configs.ie_config.eff_mat_5 =
                0;
            isp_cfg->configs.ie_config.color_sel =
                0;
            isp_cfg->configs.ie_config.eff_tint =
                0;
          }
          break;
          case CAMERIC_IE_MODE_SKETCH: {
            isp_cfg->configs.ie_config.effect =
                V4L2_COLORFX_SKETCH;
            isp_cfg->configs.ie_config.eff_mat_3 =
                ((uint16_t)(ia_results->ie.ModeConfig.Sketch.coeff[0]) << 0x4)
                | ((uint16_t)(ia_results->ie.ModeConfig.Sketch.coeff[1]) << 0x8)
                | ((uint16_t)(ia_results->ie.ModeConfig.Sketch.coeff[2]) << 0xc);
            /*not used for this effect*/
            isp_cfg->configs.ie_config.eff_mat_4 =
                (uint16_t)(ia_results->ie.ModeConfig.Sketch.coeff[3])
                | ((uint16_t)(ia_results->ie.ModeConfig.Sketch.coeff[4]) << 0x4)
                | ((uint16_t)(ia_results->ie.ModeConfig.Sketch.coeff[5]) << 0x8)
                | ((uint16_t)(ia_results->ie.ModeConfig.Sketch.coeff[6]) << 0xc);
            isp_cfg->configs.ie_config.eff_mat_5 =
                (uint16_t)(ia_results->ie.ModeConfig.Sketch.coeff[7])
                | ((uint16_t)(ia_results->ie.ModeConfig.Sketch.coeff[8]) << 0x4);

            /*not used for this effect*/
            isp_cfg->configs.ie_config.eff_mat_1 = 0;
            isp_cfg->configs.ie_config.eff_mat_2 = 0;
            isp_cfg->configs.ie_config.color_sel =
                0;
            isp_cfg->configs.ie_config.eff_tint =
                0;
          }
          break;
          case CAMERIC_IE_MODE_SHARPEN: {
            /* TODO: can't find related mode in v4l2_colorfx*/
            //isp_cfg->configs.ie_config.effect =
            //  V4L2_COLORFX_EMBOSS;
            isp_cfg->configs.ie_config.eff_mat_1 =
                (uint16_t)(ia_results->ie.ModeConfig.Sharpen.coeff[0])
                | ((uint16_t)(ia_results->ie.ModeConfig.Sharpen.coeff[1]) << 0x4)
                | ((uint16_t)(ia_results->ie.ModeConfig.Sharpen.coeff[2]) << 0x8)
                | ((uint16_t)(ia_results->ie.ModeConfig.Sharpen.coeff[3]) << 0xc);
            isp_cfg->configs.ie_config.eff_mat_2 =
                (uint16_t)(ia_results->ie.ModeConfig.Sharpen.coeff[4])
                | ((uint16_t)(ia_results->ie.ModeConfig.Sharpen.coeff[5]) << 0x4)
                | ((uint16_t)(ia_results->ie.ModeConfig.Sharpen.coeff[6]) << 0x8)
                | ((uint16_t)(ia_results->ie.ModeConfig.Sharpen.coeff[7]) << 0xc);
            isp_cfg->configs.ie_config.eff_mat_3 =
                (ia_results->ie.ModeConfig.Sharpen.coeff[8]);
            /*not used for this effect*/
            isp_cfg->configs.ie_config.eff_mat_4 =
                0;
            isp_cfg->configs.ie_config.eff_mat_5 =
                0;
            isp_cfg->configs.ie_config.color_sel =
                0;
            isp_cfg->configs.ie_config.eff_tint =
                0;
          }
          break;
          default: {
            ALOGE("%s: set ie mode failed %d", __FUNCTION__,
                  ia_results->ie.mode);
            if (ia_results->ie.enabled == BOOL_TRUE)
              isp_cfg->active_configs &=  ~ISP_IE_MASK;
          }
        }
      }

    }

    if (ia_results->active & CAMIA10_WDR_MASK) {
      //CameraIcWdrConfig_t to struct cifisp_wdr_config
      int regi = 0, i = 0;
      isp_cfg->enabled[HAL_ISP_WDR_ID] = ia_results->wdr.enabled;
      isp_cfg->active_configs |=  ISP_WDR_MASK;
      if (ia_results->wdr.mode == CAMERIC_WDR_MODE_BLOCK)
        isp_cfg->configs.wdr_config.mode =
            CIFISP_WDR_MODE_BLOCK;
      else
        isp_cfg->configs.wdr_config.mode =
            CIFISP_WDR_MODE_GLOBAL;
      //TODO
      /*offset 0x2a00*/
      isp_cfg->configs.wdr_config.c_wdr[0] = 0x00000812;
      /* offset 0x2a04 - 0x2a10*/

      for (regi = 1; regi < 5; regi++) {
        isp_cfg->configs.wdr_config.c_wdr[regi] = 0;
        for (int i = 0; i < 8; i++)
          isp_cfg->configs.wdr_config.c_wdr[regi] |=
              (uint32_t)(ia_results->wdr.segment \
                         [i + (regi - 1) * 8 ]) << (4 * i);
      }

      /*offset 0x2a14 - 0x2a94*/
      for (regi = 5; regi < 38; regi++)
        isp_cfg->configs.wdr_config.c_wdr[regi] =
            ((uint32_t)(ia_results->wdr.wdr_block_y[regi - 5]) << 16) |
            (uint32_t)(ia_results->wdr.wdr_global_y[regi - 5]);
      /*offset 0x2a98 - 0x2a9c*/
      isp_cfg->configs.wdr_config.c_wdr[38] = 0x0;
      isp_cfg->configs.wdr_config.c_wdr[39] = 0x0;
#if 0
      /*offset 0x2b50 - 0x2b6c*/
      isp_cfg->configs.wdr_config.c_wdr[40] = 0x00030cf0;
      isp_cfg->configs.wdr_config.c_wdr[41] = 0x000140d3;
      isp_cfg->configs.wdr_config.c_wdr[42] = 0x000000cd;
      isp_cfg->configs.wdr_config.c_wdr[43] = 0x0ccc00ee;
      isp_cfg->configs.wdr_config.c_wdr[44] = 0x00000036;
      isp_cfg->configs.wdr_config.c_wdr[45] = 0x000000b7;
      isp_cfg->configs.wdr_config.c_wdr[46] = 0x00000012;
      isp_cfg->configs.wdr_config.c_wdr[47] = 0x0;
#else
      isp_cfg->configs.wdr_config.c_wdr[40] =
          ((uint32_t)(ia_results->wdr.wdr_pym_cc) << 16) |
          ((uint32_t)(ia_results->wdr.wdr_epsilon) << 8) |
          ((uint32_t)(ia_results->wdr.wdr_lvl_en) << 4) ;

      isp_cfg->configs.wdr_config.c_wdr[41] =
          ((uint32_t)(ia_results->wdr.wdr_gain_max_clip_enable) << 16) |
          ((uint32_t)(ia_results->wdr.wdr_gain_max_value) << 8) |
          ((uint32_t)(ia_results->wdr.wdr_bavg_clip) << 6) |
          ((uint32_t)(ia_results->wdr.wdr_nonl_segm) << 5) |
          ((uint32_t)(ia_results->wdr.wdr_nonl_open) << 4) |
          ((uint32_t)(ia_results->wdr.wdr_nonl_mode1) << 3) |
          ((uint32_t)(ia_results->wdr.wdr_flt_sel) << 1) |
          ia_results->wdr.mode ;

      isp_cfg->configs.wdr_config.c_wdr[42] = ia_results->wdr.wdr_gain_off1;
      isp_cfg->configs.wdr_config.c_wdr[43] =
          ((uint32_t)(ia_results->wdr.wdr_bestlight) << 16) |
          (uint32_t)(ia_results->wdr.wdr_noiseratio);
      isp_cfg->configs.wdr_config.c_wdr[44] = ia_results->wdr.wdr_coe0;
      isp_cfg->configs.wdr_config.c_wdr[45] =  ia_results->wdr.wdr_coe1;
      isp_cfg->configs.wdr_config.c_wdr[46] =  ia_results->wdr.wdr_coe2;
      isp_cfg->configs.wdr_config.c_wdr[47] =  ia_results->wdr.wdr_coe_off;
#endif
      for (i = 0; i < 48; i++)
        TRACE_D(1, "WDR[%d] = 0x%x", i, isp_cfg->configs.wdr_config.c_wdr[i]);

      TRACE_D(1, "WDR = 0x%x", ia_results->wdr.wdr_gain_max_value);
    }

    if (ia_results->active & CAMIA10_DSP_3DNR_MASK) {
      isp_cfg->active_configs |= ISP_DSP_3DNR_MASK;
      isp_cfg->configs.Dsp3DnrSetConfig = ia_results->adpf.Dsp3DnrResult;

      TRACE_D(1, "ConvertIA luma_sp:%d %d luma_te:%d %d chrm_sp:%d %d chrm_te:%d %d shp:%d %d noise:(%d/%d)",
              isp_cfg->configs.Dsp3DnrSetConfig.luma_sp_nr_en,
              isp_cfg->configs.Dsp3DnrSetConfig.luma_sp_nr_level,
              isp_cfg->configs.Dsp3DnrSetConfig.luma_te_nr_en,
              isp_cfg->configs.Dsp3DnrSetConfig.luma_te_nr_level,
              isp_cfg->configs.Dsp3DnrSetConfig.chrm_sp_nr_en,
              isp_cfg->configs.Dsp3DnrSetConfig.chrm_sp_nr_level,
              isp_cfg->configs.Dsp3DnrSetConfig.chrm_te_nr_en,
              isp_cfg->configs.Dsp3DnrSetConfig.chrm_te_nr_level,
              isp_cfg->configs.Dsp3DnrSetConfig.shp_en,
              isp_cfg->configs.Dsp3DnrSetConfig.shp_level,
              isp_cfg->configs.Dsp3DnrSetConfig.noise_coef_num,
              isp_cfg->configs.Dsp3DnrSetConfig.noise_coef_den);

	  TRACE_D(1, "ConvertIA setting luma:%d 0x%x  chrm:%d 0x%x shp:%d 0x%x",
              isp_cfg->configs.Dsp3DnrSetConfig.luma_default,
              isp_cfg->configs.Dsp3DnrSetConfig.luma_w2,
              isp_cfg->configs.Dsp3DnrSetConfig.chrm_default,
              isp_cfg->configs.Dsp3DnrSetConfig.chrm_w2,
              isp_cfg->configs.Dsp3DnrSetConfig.shp_default,
              isp_cfg->configs.Dsp3DnrSetConfig.src_shp_w2);
    }

	if (ia_results->active & CAMIA10_NEW_DSP_3DNR_MASK) {
	  isp_cfg->active_configs |= ISP_NEW_DSP_3DNR_MASK;
	  isp_cfg->configs.NewDsp3DnrSetConfig = ia_results->adpf.NewDsp3DnrResult;

	  TRACE_D(1, "ConvertIA setting 3dnr_en:%d dpc_en:%d ynr_en:%d tnr_en:%d iir_en:%d uvnr_en:%d shrp_en:%d",
			  isp_cfg->configs.NewDsp3DnrSetConfig.enable_3dnr,
			  isp_cfg->configs.NewDsp3DnrSetConfig.enable_dpc,
			  isp_cfg->configs.NewDsp3DnrSetConfig.enable_ynr,
			  isp_cfg->configs.NewDsp3DnrSetConfig.enable_tnr,
			  isp_cfg->configs.NewDsp3DnrSetConfig.enable_iir,
			  isp_cfg->configs.NewDsp3DnrSetConfig.enable_uvnr,
			  isp_cfg->configs.NewDsp3DnrSetConfig.enable_sharp);
		  
	  TRACE_D(1, "ConvertIA setting ynr_time_weight:%d ynr_spat_weight:%d uvnr:%d sharp:%d ",
			  isp_cfg->configs.NewDsp3DnrSetConfig.ynr_time_weight,
			  isp_cfg->configs.NewDsp3DnrSetConfig.ynr_spat_weight,
			  isp_cfg->configs.NewDsp3DnrSetConfig.uvnr_weight,
			  isp_cfg->configs.NewDsp3DnrSetConfig.sharp_weight,
			  isp_cfg->configs.NewDsp3DnrSetConfig.enable_dpc);
	}

    /*  TODOS */
    if (ia_results->active & CAMIA10_AFC_MASK) {
      isp_cfg->active_configs |= ISP_AFC_MASK;
      isp_cfg->enabled[HAL_ISP_AFC_ID] = ia_results->afc_meas_enabled;
      isp_cfg->configs.afc_config.afm_win[0].h_offs = ia_results->af.WindowA.h_offs;
      isp_cfg->configs.afc_config.afm_win[0].v_offs = ia_results->af.WindowA.v_offs;
      isp_cfg->configs.afc_config.afm_win[0].h_size = ia_results->af.WindowA.h_size;
      isp_cfg->configs.afc_config.afm_win[0].v_size = ia_results->af.WindowA.v_size;

      isp_cfg->configs.afc_config.afm_win[1].h_offs = ia_results->af.WindowB.h_offs;
      isp_cfg->configs.afc_config.afm_win[1].v_offs = ia_results->af.WindowB.v_offs;
      isp_cfg->configs.afc_config.afm_win[1].h_size = ia_results->af.WindowB.h_size;
      isp_cfg->configs.afc_config.afm_win[1].v_size = ia_results->af.WindowB.v_size;

      isp_cfg->configs.afc_config.afm_win[2].h_offs = ia_results->af.WindowC.h_offs;
      isp_cfg->configs.afc_config.afm_win[2].v_offs = ia_results->af.WindowC.v_offs;
      isp_cfg->configs.afc_config.afm_win[2].h_size = ia_results->af.WindowC.h_size;
      isp_cfg->configs.afc_config.afm_win[2].v_size = ia_results->af.WindowC.v_size;

      isp_cfg->configs.afc_config.num_afm_win = ia_results->af.Window_Num;
      isp_cfg->configs.afc_config.thres = ia_results->af.Thres;
      isp_cfg->configs.afc_config.var_shift = ia_results->af.VarShift;
      /*
      ALOGD("%s: set afm_win[0]: h_offs %d, v_offs %d, h_size %d, v_size %d", __FUNCTION__,
            isp_cfg->configs.afc_config.afm_win[0].h_offs,
            isp_cfg->configs.afc_config.afm_win[0].v_offs,
            isp_cfg->configs.afc_config.afm_win[0].h_size,
            isp_cfg->configs.afc_config.afm_win[0].v_size);
      ALOGD("%s: set afm_win[1]: h_offs %d, v_offs %d, h_size %d, v_size %d", __FUNCTION__,
            isp_cfg->configs.afc_config.afm_win[1].h_offs,
            isp_cfg->configs.afc_config.afm_win[1].v_offs,
            isp_cfg->configs.afc_config.afm_win[1].h_size,
            isp_cfg->configs.afc_config.afm_win[1].v_size);
      ALOGD("%s: set afm_win[2]: h_offs %d, v_offs %d, h_size %d, v_size %d", __FUNCTION__,
            isp_cfg->configs.afc_config.afm_win[2].h_offs,
            isp_cfg->configs.afc_config.afm_win[2].v_offs,
            isp_cfg->configs.afc_config.afm_win[2].h_size,
            isp_cfg->configs.afc_config.afm_win[2].v_size);
            */
    }

    if (ia_results->active & CAMIA10_AWB_LSC_MASK) {
      isp_cfg->active_configs |= ISP_DSP_AWB_LSC_MASK;
      isp_cfg->enabled[HAL_ISP_AWB_LSC_ID] = ia_results->lsc_enabled;
    }

    if (ia_results->active & CAMIA10_AWB_CCM_MASK) {
      isp_cfg->active_configs |= ISP_AWB_CCM_MASK;
      isp_cfg->enabled[HAL_ISP_AWB_CCM_ID] = ia_results->ctk_enabled;
    }

    if (ia_results->active & CAMIA10_AWB_CURVE_MASK) {
      isp_cfg->active_configs |= ISP_AWB_CURVE_MASK;
      isp_cfg->enabled[HAL_ISP_AWB_CURVE_ID] = BOOL_TRUE;
    }

    if (ia_results->active & CAMIA10_AWB_WP_MASK) {
      isp_cfg->active_configs |= ISP_AWB_WP_SET_MASK;
      isp_cfg->enabled[HAL_ISP_AWB_WP_SET_ID] = BOOL_TRUE;
    }

    if (ia_results->active & CAMIA10_ADPF_DPF_MASK) {
      isp_cfg->active_configs |= ISP_ADPF_DPF_MASK;
      isp_cfg->enabled[HAL_ISP_ADPF_DPF_ID] = ia_results->adpf_enabled;
    }

    if (ia_results->active & CAMIA10_ADPF_FLT_MASK) {
      isp_cfg->active_configs |= ISP_ADPF_FLT_MASK;
      isp_cfg->enabled[HAL_ISP_ADPF_FLT_ID] = ia_results->flt.enabled;
    }

  }

  return true;
}

bool CamIsp11CtrItf::initISPStream(const char* ispDev) {
  unsigned int i;

  CamIsp1xCtrItf::initISPStream(ispDev);

  return true;
}

bool CamIsp11CtrItf::getSensorModedata
(
    struct isp_supplemental_sensor_mode_data* drvCfg,
    CamIA10_SensorModeData* iaCfg
) {
  //ALOGD("-------getSensorModedata----------W-H: %d-%d", drvCfg->isp_input_width, drvCfg->isp_input_height);

  iaCfg->isp_input_width = drvCfg->isp_input_width;
  iaCfg->isp_input_height = drvCfg->isp_input_height;
  iaCfg->isp_output_width = drvCfg->isp_output_width;
  iaCfg->isp_output_height = drvCfg->isp_output_height;
  iaCfg->pixel_clock_freq_mhz = drvCfg->vt_pix_clk_freq_hz / 1000000.0f;
  iaCfg->horizontal_crop_offset = drvCfg->crop_horizontal_start;
  iaCfg->vertical_crop_offset = drvCfg->crop_vertical_start;
  iaCfg->cropped_image_width = drvCfg->crop_horizontal_end - drvCfg->crop_horizontal_start + 1;
  iaCfg->cropped_image_height = drvCfg->crop_vertical_end - drvCfg->crop_vertical_start + 1;
  iaCfg->pixel_periods_per_line = drvCfg->line_length_pck;
  iaCfg->line_periods_per_field = drvCfg->frame_length_lines;
  iaCfg->sensor_output_height = drvCfg->sensor_output_height;
  iaCfg->fine_integration_time_min = drvCfg->fine_integration_time_min;
  iaCfg->fine_integration_time_max_margin = drvCfg->line_length_pck - drvCfg->fine_integration_time_max_margin;
  iaCfg->coarse_integration_time_min = drvCfg->coarse_integration_time_min;
  iaCfg->coarse_integration_time_max_margin = drvCfg->coarse_integration_time_max_margin;
  iaCfg->gain = drvCfg->gain;
  iaCfg->exp_time = drvCfg->exp_time;
  iaCfg->exposure_valid_frame = drvCfg->exposure_valid_frame[0];

  //ALOGD("%s:iaCfg->pixel_clock_freq_mhz %f",__func__,iaCfg->pixel_clock_freq_mhz );
  //ALOGD("%s:iaCfg->gain %d",__func__,iaCfg->gain );
  //ALOGD("%s:iaCfg->exp_time %d",__func__,iaCfg->exp_time );
  return true;
}

void CamIsp11CtrItf::transDrvMetaDataToHal
(
    const void* drvMeta,
    struct HAL_Buffer_MetaData* halMeta
) {
  struct v4l2_buffer_metadata_s* v4l2Meta = NULL;
  struct cifisp_isp_metadata* ispMetaData = NULL;
  int i;

  if (mInitialized == 0)
    return;

  halMeta->awb.DoorType = mIA_results.awb.err_code;
  // yamasaki
  strcpy(halMeta->awb.IllName,mIA_results.awb.IllName);
  //ALOGE("yamasaki: transDrvMetaDataToHal: IlluName = %s",mIA_results.awb.IllName);
  halMeta->MeanLuma = mIA_results.aec.MeanLuma;
  halMeta->maxGainRange = mIA_results.aec.MaxGainRange;
  halMeta->DON_Fac = mIA_results.aec.DON_Fac;
  halMeta->LightMode = mCamIA_DyCfg.LightMode;
  halMeta->overHistPercent = mIA_results.aec.overHistPercent;
  //ALOGE("yamasaki: transDrvMetaDataToHal: LightMode = %d",halMeta->LightMode);
  halMeta->awb.DoorType = mIA_results.awb.err_code;

  if (drvMeta) {
    v4l2Meta = (struct v4l2_buffer_metadata_s*)drvMeta;
    ispMetaData = (struct cifisp_isp_metadata*)v4l2Meta->isp;
    halMeta->timStamp = v4l2Meta->frame_t.vs_t;
  }

  if (ispMetaData) {
    TRACE_D(1, "%s:drv exp time gain %d %d",
            __func__,
            ispMetaData->meas_stat.sensor_mode.exp_time,
            ispMetaData->meas_stat.sensor_mode.gain
           );
    if (mCamIAEngine.get())
      mCamIAEngine->mapSensorExpToHal
      (
          ispMetaData->meas_stat.sensor_mode.gain,
          ispMetaData->meas_stat.sensor_mode.exp_time,
          halMeta->exp_gain,
          halMeta->exp_time
      );
    else
      ALOGW("%s:mCamIAEngine has been desroyed!", __func__);
    /* transfer aaa algorithm results */

    halMeta->awb.wb_gain.gain_blue =
        UtlFixToFloat_U0208(ispMetaData->other_cfg.awb_gain_config.gain_blue);
    halMeta->awb.wb_gain.gain_green_b =
        UtlFixToFloat_U0208(ispMetaData->other_cfg.awb_gain_config.gain_green_b);
    halMeta->awb.wb_gain.gain_green_r =
        UtlFixToFloat_U0208(ispMetaData->other_cfg.awb_gain_config.gain_green_r);
    halMeta->awb.wb_gain.gain_red =
        UtlFixToFloat_U0208(ispMetaData->other_cfg.awb_gain_config.gain_red);

    halMeta->dpf_strength.b =
        UtlFixToFloat_U0800(ispMetaData->other_cfg.dpf_strength_config.b);
    halMeta->dpf_strength.g =
        UtlFixToFloat_U0800(ispMetaData->other_cfg.dpf_strength_config.g);
    halMeta->dpf_strength.r =
        UtlFixToFloat_U0800(ispMetaData->other_cfg.dpf_strength_config.r);
    /* FIXME: should be get flt info from drv meta data.*/
    halMeta->flt.denoise_level =
        mIspCfg.flt_denoise_level;
    halMeta->flt.sharp_level =
        mIspCfg.flt_sharp_level;
  halMeta->ctk.coeff0 =
    UtlFixToFloat_S0407(ispMetaData->other_cfg.ctk_config.coeff0);
  halMeta->ctk.coeff1 =
    UtlFixToFloat_S0407(ispMetaData->other_cfg.ctk_config.coeff1);
  halMeta->ctk.coeff2 =
    UtlFixToFloat_S0407(ispMetaData->other_cfg.ctk_config.coeff2);
  halMeta->ctk.coeff3 =
    UtlFixToFloat_S0407(ispMetaData->other_cfg.ctk_config.coeff3);
  halMeta->ctk.coeff4 =
    UtlFixToFloat_S0407(ispMetaData->other_cfg.ctk_config.coeff4);
  halMeta->ctk.coeff5 =
    UtlFixToFloat_S0407(ispMetaData->other_cfg.ctk_config.coeff5);
  halMeta->ctk.coeff6 =
    UtlFixToFloat_S0407(ispMetaData->other_cfg.ctk_config.coeff6);
  halMeta->ctk.coeff7 =
    UtlFixToFloat_S0407(ispMetaData->other_cfg.ctk_config.coeff7);
  halMeta->ctk.coeff8 =
    UtlFixToFloat_S0407(ispMetaData->other_cfg.ctk_config.coeff8);

	#if 0
	halMeta->dsp_3DNR_old.Enable = 1;
    halMeta->dsp_3DNR_old.chroma_nr_en = 1;
	halMeta->dsp_3DNR_old.chroma_nr_level = 16;
	halMeta->dsp_3DNR_old.luma_nr_en = 1;
	halMeta->dsp_3DNR_old.luma_nr_level = 16;
	halMeta->dsp_3DNR_old.shp_en = 1;
	halMeta->dsp_3DNR_old.shp_level = 16;
	#endif
    //dsp 3dnr ver2
	#if 1  
	halMeta->dsp_3DNR.luma_sp_nr_en = mIspCfg.Dsp3DnrSetConfig.luma_sp_nr_en;
	halMeta->dsp_3DNR.luma_te_nr_en = mIspCfg.Dsp3DnrSetConfig.luma_te_nr_en;
	halMeta->dsp_3DNR.chrm_sp_nr_en = mIspCfg.Dsp3DnrSetConfig.chrm_sp_nr_en;
	halMeta->dsp_3DNR.chrm_te_nr_en = mIspCfg.Dsp3DnrSetConfig.chrm_te_nr_en;
	halMeta->dsp_3DNR.shp_en = mIspCfg.Dsp3DnrSetConfig.shp_en;
	halMeta->dsp_3DNR.luma_sp_nr_level = mIspCfg.Dsp3DnrSetConfig.luma_sp_nr_level;
	halMeta->dsp_3DNR.luma_te_nr_level = mIspCfg.Dsp3DnrSetConfig.luma_te_nr_level;
	halMeta->dsp_3DNR.chrm_sp_nr_level = mIspCfg.Dsp3DnrSetConfig.chrm_sp_nr_level;
	halMeta->dsp_3DNR.chrm_te_nr_level = mIspCfg.Dsp3DnrSetConfig.chrm_te_nr_level;
	halMeta->dsp_3DNR.shp_level = mIspCfg.Dsp3DnrSetConfig.shp_level;

	halMeta->dsp_3DNR.luma_default = mIspCfg.Dsp3DnrSetConfig.luma_default;
	halMeta->dsp_3DNR.luma_sp_rad = mIspCfg.Dsp3DnrSetConfig.luma_sp_rad;
	halMeta->dsp_3DNR.luma_te_max_bi_num = mIspCfg.Dsp3DnrSetConfig.luma_te_max_bi_num;
	halMeta->dsp_3DNR.luma_w0 = mIspCfg.Dsp3DnrSetConfig.luma_w0;
	halMeta->dsp_3DNR.luma_w1 = mIspCfg.Dsp3DnrSetConfig.luma_w1;
	halMeta->dsp_3DNR.luma_w2 = mIspCfg.Dsp3DnrSetConfig.luma_w2;
	halMeta->dsp_3DNR.luma_w3 = mIspCfg.Dsp3DnrSetConfig.luma_w3;
	halMeta->dsp_3DNR.luma_w4 = mIspCfg.Dsp3DnrSetConfig.luma_w4;

	halMeta->dsp_3DNR.chrm_default = mIspCfg.Dsp3DnrSetConfig.chrm_default;
	halMeta->dsp_3DNR.chrm_sp_rad = mIspCfg.Dsp3DnrSetConfig.chrm_sp_rad;
	halMeta->dsp_3DNR.chrm_te_max_bi_num = mIspCfg.Dsp3DnrSetConfig.chrm_te_max_bi_num;
	halMeta->dsp_3DNR.chrm_w0 = mIspCfg.Dsp3DnrSetConfig.chrm_w0;
	halMeta->dsp_3DNR.chrm_w1 = mIspCfg.Dsp3DnrSetConfig.chrm_w1;
	halMeta->dsp_3DNR.chrm_w2 = mIspCfg.Dsp3DnrSetConfig.chrm_w2;
	halMeta->dsp_3DNR.chrm_w3 = mIspCfg.Dsp3DnrSetConfig.chrm_w3;
	halMeta->dsp_3DNR.chrm_w4 = mIspCfg.Dsp3DnrSetConfig.chrm_w4;

	halMeta->dsp_3DNR.shp_default = mIspCfg.Dsp3DnrSetConfig.shp_default;
	halMeta->dsp_3DNR.src_shp_c = mIspCfg.Dsp3DnrSetConfig.src_shp_c;
	halMeta->dsp_3DNR.src_shp_l = mIspCfg.Dsp3DnrSetConfig.src_shp_l;
	halMeta->dsp_3DNR.src_shp_div = mIspCfg.Dsp3DnrSetConfig.src_shp_div;
	halMeta->dsp_3DNR.src_shp_thr = mIspCfg.Dsp3DnrSetConfig.src_shp_thr;
	halMeta->dsp_3DNR.src_shp_w0 = mIspCfg.Dsp3DnrSetConfig.src_shp_w0;
	halMeta->dsp_3DNR.src_shp_w1 = mIspCfg.Dsp3DnrSetConfig.src_shp_w1;
	halMeta->dsp_3DNR.src_shp_w2 = mIspCfg.Dsp3DnrSetConfig.src_shp_w2;	
	halMeta->dsp_3DNR.src_shp_w3 = mIspCfg.Dsp3DnrSetConfig.src_shp_w3;
	halMeta->dsp_3DNR.src_shp_w4 = mIspCfg.Dsp3DnrSetConfig.src_shp_w4;
	
	halMeta->dsp_3DNR.reserves.ex.noise_level =
		((mIspCfg.Dsp3DnrSetConfig.noise_coef_den & 0xffff)<<16)
		| (mIspCfg.Dsp3DnrSetConfig.noise_coef_num & 0xffff);
	halMeta->dsp_3DNR.reserves.ex.full_range_flag = 0x00;
	halMeta->dsp_3DNR.reserves.ex.blk16_sad_th_ratio = 0x05;
	halMeta->dsp_3DNR.reserves.ex.blk16_shf_bit_1 = 0x01;
	halMeta->dsp_3DNR.reserves.ex.blk32_shf_bit_1 = 0x01;
	halMeta->dsp_3DNR.reserves.ex.blk128_shf_bit_1 = 0x01;
	halMeta->dsp_3DNR.reserves.ex.blk16_shf_bit_0 = 0x01;;
	halMeta->dsp_3DNR.reserves.ex.blk32_shf_bit_0 = 0x01;
	halMeta->dsp_3DNR.reserves.ex.blk128_shf_bit_0 = 0x01;
	halMeta->dsp_3DNR.reserves.ex.night_flag = mCamIA_DyCfg.LightMode;

	TRACE_D(1, "ConvertIA luma_sp:%d %d luma_te:%d %d chrm_sp:%d %d chrm_te:%d %d shp:%d %d",
              halMeta->dsp_3DNR.luma_sp_nr_en,
              halMeta->dsp_3DNR.luma_sp_nr_level,
              halMeta->dsp_3DNR.luma_te_nr_en,
              halMeta->dsp_3DNR.luma_te_nr_level,
              halMeta->dsp_3DNR.chrm_sp_nr_en,
              halMeta->dsp_3DNR.chrm_sp_nr_level,
              halMeta->dsp_3DNR.chrm_te_nr_en,
              halMeta->dsp_3DNR.chrm_te_nr_level,
              halMeta->dsp_3DNR.shp_en,
              halMeta->dsp_3DNR.shp_level);

	  TRACE_D(1, "ConvertIA setting luma:%d 0x%x  chrm:%d 0x%x shp:%d 0x%x",
              halMeta->dsp_3DNR.luma_default,
              halMeta->dsp_3DNR.luma_w2,
              halMeta->dsp_3DNR.chrm_default,
              halMeta->dsp_3DNR.chrm_w2,
              halMeta->dsp_3DNR.shp_default,
              halMeta->dsp_3DNR.src_shp_w2);
	#endif

	//new dsp 3dnr para
	halMeta->newDsp3DNR.enable_3dnr = mIspCfg.NewDsp3DnrSetConfig.enable_3dnr;
	halMeta->newDsp3DNR.enable_dpc = mIspCfg.NewDsp3DnrSetConfig.enable_dpc;
	halMeta->newDsp3DNR.ynr.enable_ynr = mIspCfg.NewDsp3DnrSetConfig.enable_ynr;
	halMeta->newDsp3DNR.ynr.enable_tnr = mIspCfg.NewDsp3DnrSetConfig.enable_tnr;
	halMeta->newDsp3DNR.ynr.enable_iir = mIspCfg.NewDsp3DnrSetConfig.enable_iir;
	halMeta->newDsp3DNR.ynr.ynr_time_weight = mIspCfg.NewDsp3DnrSetConfig.ynr_time_weight;
	halMeta->newDsp3DNR.ynr.ynr_spat_weight = mIspCfg.NewDsp3DnrSetConfig.ynr_spat_weight;
	halMeta->newDsp3DNR.uvnr.enable_uvnr = mIspCfg.NewDsp3DnrSetConfig.enable_uvnr;
	halMeta->newDsp3DNR.uvnr.uvnr_weight = mIspCfg.NewDsp3DnrSetConfig.uvnr_weight;
	halMeta->newDsp3DNR.sharp.enable_sharp = mIspCfg.NewDsp3DnrSetConfig.enable_sharp;
	halMeta->newDsp3DNR.sharp.sharp_weight = mIspCfg.NewDsp3DnrSetConfig.sharp_weight;

	TRACE_D(1, "metadatoHal New3dnr: en:%d dpc_en:%d ynr_en:%d tnr_en:%d iir_en:%d, uvnr_en:%d shp_en:%d",
              halMeta->newDsp3DNR.enable_3dnr,
              halMeta->newDsp3DNR.enable_dpc,
              halMeta->newDsp3DNR.ynr.enable_ynr,
              halMeta->newDsp3DNR.ynr.enable_tnr,
              halMeta->newDsp3DNR.ynr.enable_iir,
              halMeta->newDsp3DNR.uvnr.enable_uvnr,
              halMeta->newDsp3DNR.sharp.enable_sharp);

	TRACE_D(1, "metadatoHal new3dnr y_time_level:%d y_spat_level:%d uv_level:%d sharp_level:%d",
              halMeta->newDsp3DNR.ynr.ynr_time_weight,
              halMeta->newDsp3DNR.ynr.ynr_spat_weight,
              halMeta->newDsp3DNR.uvnr.uvnr_weight,
              halMeta->newDsp3DNR.sharp.sharp_weight);
	
    memcpy(halMeta->enabled, mIspCfg.enabled, sizeof(mIspCfg.enabled));
    //map wdr info
    halMeta->wdr.wdr_enable = mIspCfg.enabled[HAL_ISP_WDR_ID];
    halMeta->wdr.dx_used_cnt = CAMERIC_WDR_CURVE_SIZE - 1;
    halMeta->wdr.wdr_pym_cc =
        (ispMetaData->other_cfg.wdr_config.c_wdr[40] >> 16) & 0xff;
    halMeta->wdr.wdr_epsilon =
        (ispMetaData->other_cfg.wdr_config.c_wdr[40] >> 8) & 0xff;
    halMeta->wdr.wdr_lvl_en =
        (ispMetaData->other_cfg.wdr_config.c_wdr[40] >> 4) & 0xf;
    halMeta->enabled[HAL_ISP_WDR_ID] =
        ((ispMetaData->other_cfg.wdr_config.c_wdr[40]) & 0x1)
        ? BOOL_TRUE : BOOL_FALSE;

    halMeta->wdr.wdr_gain_max_clip_enable =
        ((ispMetaData->other_cfg.wdr_config.c_wdr[41] >> 16) & 0x1)
        ? BOOL_TRUE : BOOL_FALSE;
    halMeta->wdr.wdr_gain_max_value =
        (ispMetaData->other_cfg.wdr_config.c_wdr[41] >> 8) & 0xff;
    halMeta->wdr.wdr_bavg_clip =
        ((ispMetaData->other_cfg.wdr_config.c_wdr[41] >> 6) & 0x1)
        ? BOOL_TRUE : BOOL_FALSE;
    halMeta->wdr.wdr_nonl_segm =
        ((ispMetaData->other_cfg.wdr_config.c_wdr[41] >> 5) & 0x1)
        ? BOOL_TRUE : BOOL_FALSE;
    halMeta->wdr.wdr_nonl_open =
        ((ispMetaData->other_cfg.wdr_config.c_wdr[41] >> 4) & 0x1)
        ? BOOL_TRUE : BOOL_FALSE;
    halMeta->wdr.wdr_nonl_mode1 =
        ((ispMetaData->other_cfg.wdr_config.c_wdr[41] >> 3) & 0x1)
        ? BOOL_TRUE : BOOL_FALSE;
    halMeta->wdr.mode =
        ((ispMetaData->other_cfg.wdr_config.c_wdr[41]) & 0x1)
        ? HAL_ISP_WDR_MODE_GLOBAL : HAL_ISP_WDR_MODE_BLOCK;
    halMeta->wdr.wdr_gain_off1 =
        ispMetaData->other_cfg.wdr_config.c_wdr[42];
    halMeta->wdr.wdr_bestlight =
        (ispMetaData->other_cfg.wdr_config.c_wdr[42] >> 16) & 0xfff;
    halMeta->wdr.wdr_noiseratio =
        (ispMetaData->other_cfg.wdr_config.c_wdr[42]) & 0xfff;
    halMeta->wdr.wdr_coe0 =
        ispMetaData->other_cfg.wdr_config.c_wdr[44];
    halMeta->wdr.wdr_coe1 =
        ispMetaData->other_cfg.wdr_config.c_wdr[45];
    halMeta->wdr.wdr_coe2 =
        ispMetaData->other_cfg.wdr_config.c_wdr[46];
    halMeta->wdr.wdr_coe_off =
        ispMetaData->other_cfg.wdr_config.c_wdr[47];

    int regi, i;
    for (regi = 1; regi < 5; regi++) {
      for (int i = 0; i < 8; i++)
        halMeta->wdr.wdr_dx[i + (regi - 1) * 8] =
            (ispMetaData->other_cfg.wdr_config.c_wdr[regi] \
             >> (4 * i)) & 0xf;
    }

    if (halMeta->wdr.mode == HAL_ISP_WDR_MODE_BLOCK) {
      for (regi = 5; regi < 38; regi++)
        halMeta->wdr.wdr_dy.wdr_block_dy[regi - 5] =
            (ispMetaData->other_cfg.wdr_config.c_wdr[regi] >> 16) & 0xffff;
    } else {
      for (regi = 5; regi < 38; regi++)
        halMeta->wdr.wdr_dy.wdr_block_dy[regi - 5] =
            (ispMetaData->other_cfg.wdr_config.c_wdr[regi]) & 0xffff;
    }

	//lsc
	halMeta->lsc.lsc_enable = mIspCfg.enabled[HAL_ISP_LSC_ID];
	int maxChannel = 0;
	int maxValue = 0;
	if(mIspCfg.lsc_config.r_data_tbl[0] > maxValue){
		maxChannel = 0;
		maxValue =  mIspCfg.lsc_config.r_data_tbl[0];
	}
	if(mIspCfg.lsc_config.gr_data_tbl[0] > maxValue){
		maxChannel = 1;
		maxValue =  mIspCfg.lsc_config.gr_data_tbl[0];
	}
	if(mIspCfg.lsc_config.gb_data_tbl[0] > maxValue){
		maxChannel = 2;
		maxValue =  mIspCfg.lsc_config.gb_data_tbl[0];
	}
	if(mIspCfg.lsc_config.b_data_tbl[0] > maxValue){
		maxChannel = 3;
		maxValue =  mIspCfg.lsc_config.b_data_tbl[0];
	}

	if(maxChannel == 0){
		for(int i=0; i < CIFISP_LSC_DATA_TBL_SIZE; i++)
			halMeta->lsc.lsc_max_data_tbl[i] = mIspCfg.lsc_config.r_data_tbl[i];
	}

	if(maxChannel == 1){
		for(int i=0; i < CIFISP_LSC_DATA_TBL_SIZE; i++)
			halMeta->lsc.lsc_max_data_tbl[i] = mIspCfg.lsc_config.gr_data_tbl[i];
	}

	if(maxChannel == 2){
		for(int i=0; i < CIFISP_LSC_DATA_TBL_SIZE; i++)
			halMeta->lsc.lsc_max_data_tbl[i] = mIspCfg.lsc_config.gb_data_tbl[i];
	}

	if(maxChannel == 4){
		for(int i=0; i < CIFISP_LSC_DATA_TBL_SIZE; i++)
			halMeta->lsc.lsc_max_data_tbl[i] = mIspCfg.lsc_config.b_data_tbl[i];
	}

    halMeta->lsc.data_table_cnt = CIFISP_LSC_DATA_TBL_SIZE;
    halMeta->lsc.grad_table_cnt = CIFISP_LSC_GRAD_TBL_SIZE;
    for (i = 0; i < CIFISP_LSC_DATA_TBL_SIZE; i++) {
      halMeta->lsc.LscRDataTbl[i] = ispMetaData->other_cfg.lsc_config.r_data_tbl[i];
      halMeta->lsc.LscGRDataTbl[i] = ispMetaData->other_cfg.lsc_config.gr_data_tbl[i];
      halMeta->lsc.LscGBDataTbl[i] = ispMetaData->other_cfg.lsc_config.gb_data_tbl[i];
      halMeta->lsc.LscBDataTbl[i] = ispMetaData->other_cfg.lsc_config.b_data_tbl[i];
    }
    for (i = 0; i < CIFISP_LSC_GRAD_TBL_SIZE; i++) {
      halMeta->lsc.LscXGradTbl[i] = ispMetaData->other_cfg.lsc_config.x_grad_tbl[i];
      halMeta->lsc.LscYGradTbl[i] = ispMetaData->other_cfg.lsc_config.y_grad_tbl[i];
    }
    for (i = 0; i < CIFISP_LSC_SIZE_TBL_SIZE; i++) {
      halMeta->lsc.LscXSizeTbl[i] = ispMetaData->other_cfg.lsc_config.x_size_tbl[i];
      halMeta->lsc.LscYSizeTbl[i] = ispMetaData->other_cfg.lsc_config.y_size_tbl[i];
    }

    //goc
    if (ispMetaData->other_cfg.goc_config.mode == CIFISP_GOC_MODE_LOGARITHMIC)
      halMeta->goc.mode = HAL_ISP_GAMMA_SEG_MODE_LOGARITHMIC;
    else
      halMeta->goc.mode = HAL_ISP_GAMMA_SEG_MODE_EQUIDISTANT;
    for (i = 0; i < CIFISP_GAMMA_OUT_MAX_SAMPLES; i++) {
      halMeta->goc.gamma_y[i] = ispMetaData->other_cfg.goc_config.gamma_y[i];
    }
  }

}

static int testPos[9] = {64, 56, 48, 40, 32, 24, 16, 8, 0};
bool CamIsp11CtrItf::threadLoop() {
  unsigned int buf_index;
  struct cifisp_stat_buffer* buffer = NULL;
  struct v4l2_buffer v4l2_buf;
  struct CamIA10_Stats ia_stat = {0};
  struct CamIA10_DyCfg ia_dcfg;
  struct CamIA10_Results ia_results = {0};
  struct CamIsp11ConfigSet isp_cfg = {0};
  VL6180x_RangeData_t range_datas;
  
  memset(&ia_dcfg, 0, sizeof(ia_dcfg));
//  ALOGD("%s: enter",__func__);

  if (!getMeasurement(v4l2_buf)) {
    ALOGE("%s: getMeasurement failed", __func__);
    return true;
  }

  if (v4l2_buf.index >= CAM_ISP_NUM_OF_STAT_BUFS) {
    ALOGE("%s: v4l2_buf index: %d is invalidate!", __func__);
    return true;
  }

  convertIspStats(mIspStats[v4l2_buf.index], &ia_stat);

  //get sensor mode data
  if (ia_stat.meas_type & CAMIA10_AEC_MASK) {
    buffer = (struct cifisp_stat_buffer*)(mIspStats[v4l2_buf.index]);
    getSensorModedata(&(buffer->sensor_mode), &(mCamIA_DyCfg.sensor_mode));
  }
  releaseMeasurement(&v4l2_buf);


  if (ia_stat.meas_type & CAMIA10_AFC_MASK) {
    if (mLaserFd > 0 ) {
      if (ioctl(mLaserFd, VL6180_IOCTL_GETDATAS,&range_datas) == 0) {
        //ALOGD("VL1680 errorStatus: 0x%x, distance: %dmm\n", range_datas.errorStatus, range_datas.range_mm);
        //if (range_datas.errorStatus == 0) {
          ia_stat.af.meas_type |= AF_TYPE_LASER;
          ia_stat.af.laser.distance = range_datas.range_mm;
        //}
      }
    }
  }

  osMutexLock(&mApiLock);
  ia_dcfg = mCamIA_DyCfg;
  osMutexUnlock(&mApiLock);

  runIA(&ia_dcfg, &ia_stat, &ia_results);

  //ircut
  //not do this in convertIAResults, that will cause deadlock now
  bool night_mode = mNightMode;
  enum HAL_DAYNIGHT_MODE day_night_sw = mDayNightSwitch;
  if (night_mode == true) {
    int is_night = CIFISP_LS_INVAL;

    switch (day_night_sw) {
      case HAL_DAYNIGHT_DAY:
        is_night = LIGHT_MODE_DAY;
        break;
      case HAL_DAYNIGHT_NIGHT:
        is_night = LIGHT_MODE_NIGHT;
        break;
      default:
        if (ia_results.aec.Night_Trigger != TRIGGER_OFF){
          if (ia_results.aec.Night_Trigger == LIGHT_SENS)// for Light-senstive Trigger
            is_night = mIspStats[v4l2_buf.index]->subdev_stat.ls.val;
          else if (ia_results.aec.Night_Trigger == NO_LIGHT_SENS)// without Light-senstive Trigger
            is_night = ia_results.aec.DON_LightMode;
        }
        break;
    }

    if (is_night != CIFISP_LS_INVAL){
      if (ia_results.aec.Night_Mode == WHITE_BLACK){// This case requires switch the IR cut filter.
        switchSubDevIrCutMode(!is_night);
      }
      mCamIA_DyCfg.LightMode = is_night ? LIGHT_MODE_NIGHT : LIGHT_MODE_DAY;
    }
  } else {
    switchSubDevIrCutMode(1);
    mCamIA_DyCfg.LightMode = LIGHT_MODE_DAY;
  }
 
  //run isp manual config?will override the 3A results
  if (!runISPManual(&ia_results, BOOL_TRUE))
    ALOGE("%s:run ISP manual failed!", __func__);

  applySubDevConfig(&ia_results);
  convertIAResults(&isp_cfg, &ia_results);
  mIA_results = ia_results;
  applyIspConfig(&isp_cfg);

  return true;

}

