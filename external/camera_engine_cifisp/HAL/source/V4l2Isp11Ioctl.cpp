#include "V4l2Isp11Ioctl.h"
#include <sys/ioctl.h>
//#include <error.h>


using namespace std;

#define GET_ISP_CFG(ID,VAL,ID1, ENABLE) \
  int ret = ioctl(mDevFp, ID, &(VAL)); \
  if (ret < 0) { \
    return false; \
  } \
  return getControl(ID1, ENABLE);

#define SET_ISP_CFG(ID,VAL,ID1, ENABLE) \
  if(ENABLE) { \
    int ret = ioctl(mDevFp, ID, &(VAL)); \
    if (ret < 0) { \
      return false; \
    } \
  } else { \
  } \
  return setControl(ID1, ENABLE);


V4l2Isp11Ioctl::V4l2Isp11Ioctl(int devFp):
  mDevFp(devFp) {
}
V4l2Isp11Ioctl::~V4l2Isp11Ioctl(void) {
}
/* dpcc */
bool V4l2Isp11Ioctl::getDpccCfg(struct cifisp_dpcc_config& dpccCfg, bool& enable) {
  GET_ISP_CFG(CIFISP_IOC_G_DPCC, dpccCfg,
              V4L2_CID_CIFISP_DPCC, enable);
}
bool V4l2Isp11Ioctl::setDpccCfg(struct cifisp_dpcc_config& dpccCfg, bool enable) {
  SET_ISP_CFG(CIFISP_IOC_S_DPCC, dpccCfg,
              V4L2_CID_CIFISP_DPCC, enable);
}

/* bls */
bool V4l2Isp11Ioctl::getBlsCfg(struct cifisp_bls_config& blsCfg, bool& enable) {
  GET_ISP_CFG(CIFISP_IOC_G_BLS, blsCfg,
              V4L2_CID_CIFISP_BLS, enable);
}
bool V4l2Isp11Ioctl::setBlsCfg(struct cifisp_bls_config& blsCfg, bool enable) {
  SET_ISP_CFG(CIFISP_IOC_S_BLS, blsCfg,
              V4L2_CID_CIFISP_BLS, enable);
}
/* degamma */
bool V4l2Isp11Ioctl::getSdgCfg(struct cifisp_sdg_config& sdgCfg, bool& enable) {
  GET_ISP_CFG(CIFISP_IOC_G_SDG, sdgCfg,
              V4L2_CID_CIFISP_SDG, enable);
}
bool V4l2Isp11Ioctl::setSdgCfg(struct cifisp_sdg_config& sdgCfg, bool enable) {
  SET_ISP_CFG(CIFISP_IOC_S_SDG, sdgCfg,
              V4L2_CID_CIFISP_SDG, enable);
}
/* lsc */
bool V4l2Isp11Ioctl::getLscCfg(struct cifisp_lsc_config& lscCfg, bool& enable) {
  GET_ISP_CFG(CIFISP_IOC_G_LSC, lscCfg,
              V4L2_CID_CIFISP_LSC, enable);
}
bool V4l2Isp11Ioctl::setLscCfg(struct cifisp_lsc_config& lscCfg, bool enable) {
  SET_ISP_CFG(CIFISP_IOC_S_LSC, lscCfg,
              V4L2_CID_CIFISP_LSC, enable);
}
/* awb measure */
bool V4l2Isp11Ioctl::getAwbMeasCfg(struct cifisp_awb_meas_config& awbCfg, bool& enable) {
  GET_ISP_CFG(CIFISP_IOC_G_AWB_MEAS, awbCfg,
              V4L2_CID_CIFISP_AWB_MEAS, enable);
}
bool V4l2Isp11Ioctl::setAwbMeasCfg(struct cifisp_awb_meas_config& awbCfg, bool enable) {
  SET_ISP_CFG(CIFISP_IOC_S_AWB_MEAS, awbCfg,
              V4L2_CID_CIFISP_AWB_MEAS, enable);
}
/* filter */
bool V4l2Isp11Ioctl::getFltCfg(struct cifisp_flt_config& fltCfg, bool& enable) {
  GET_ISP_CFG(CIFISP_IOC_G_FLT, fltCfg,
              V4L2_CID_CIFISP_FLT, enable);
}
bool V4l2Isp11Ioctl::setFltCfg(struct cifisp_flt_config& fltCfg, bool enable) {
  SET_ISP_CFG(CIFISP_IOC_S_FLT, fltCfg,
              V4L2_CID_CIFISP_FLT, enable);
}
/* demosaic */
bool V4l2Isp11Ioctl::getBdmCfg(struct cifisp_bdm_config& bdmCfg, bool& enable) {
  GET_ISP_CFG(CIFISP_IOC_G_BDM, bdmCfg,
              V4L2_CID_CIFISP_BDM, enable);
}
bool V4l2Isp11Ioctl::setBdmCfg(struct cifisp_bdm_config& bdmCfg, bool enable) {
  SET_ISP_CFG(CIFISP_IOC_S_BDM, bdmCfg,
              V4L2_CID_CIFISP_BDM, enable);
}
/* cross talk */
bool V4l2Isp11Ioctl::getCtkCfg(struct cifisp_ctk_config& ctkCfg, bool& enable) {
  GET_ISP_CFG(CIFISP_IOC_G_CTK, ctkCfg,
              V4L2_CID_CIFISP_CTK, enable);
}
bool V4l2Isp11Ioctl::setCtkCfg(struct cifisp_ctk_config& ctkCfg, bool enable) {
  SET_ISP_CFG(CIFISP_IOC_S_CTK, ctkCfg,
              V4L2_CID_CIFISP_CTK, enable);
}
/* gamma out */
bool V4l2Isp11Ioctl::getGocCfg(struct cifisp_goc_config& gocCfg, bool& enable) {
  GET_ISP_CFG(CIFISP_IOC_G_GOC, gocCfg,
              V4L2_CID_CIFISP_GOC, enable);
}
bool V4l2Isp11Ioctl::setGocCfg(struct cifisp_goc_config& gocCfg, bool enable) {
  SET_ISP_CFG(CIFISP_IOC_S_GOC, gocCfg,
              V4L2_CID_CIFISP_GOC, enable);
}
/* Histogram Measurement */
bool V4l2Isp11Ioctl::getHstCfg(struct cifisp_hst_config& hstCfg, bool& enable) {
  GET_ISP_CFG(CIFISP_IOC_G_HST, hstCfg,
              V4L2_CID_CIFISP_HST, enable);
}
bool V4l2Isp11Ioctl::setHstCfg(struct cifisp_hst_config& hstCfg, bool enable) {
  SET_ISP_CFG(CIFISP_IOC_S_HST, hstCfg,
              V4L2_CID_CIFISP_HST, enable);
}
/* Auto Exposure Measurements */
bool V4l2Isp11Ioctl::getAecCfg(struct cifisp_aec_config& aecCfg, bool& enable) {
  GET_ISP_CFG(CIFISP_IOC_G_AEC, aecCfg,
              V4L2_CID_CIFISP_AEC, enable);
}
bool V4l2Isp11Ioctl::setAecCfg(struct cifisp_aec_config& aecCfg, bool enable) {
  SET_ISP_CFG(CIFISP_IOC_S_AEC, aecCfg,
              V4L2_CID_CIFISP_AEC, enable);
}
/* awb gain */
bool V4l2Isp11Ioctl::getAwbGainCfg(struct cifisp_awb_gain_config& awbgCfg, bool& enable) {
  GET_ISP_CFG(CIFISP_IOC_G_AWB_GAIN, awbgCfg,
              V4L2_CID_CIFISP_AWB_GAIN, enable);
}
bool V4l2Isp11Ioctl::setAwbGainCfg(struct cifisp_awb_gain_config& awbgCfg, bool enable) {
  SET_ISP_CFG(CIFISP_IOC_S_AWB_GAIN, awbgCfg,
              V4L2_CID_CIFISP_AWB_GAIN, enable);
}
/* color process */
bool V4l2Isp11Ioctl::getCprocCfg(struct cifisp_cproc_config& cprocCfg, bool& enable) {
  GET_ISP_CFG(CIFISP_IOC_G_CPROC, cprocCfg,
              V4L2_CID_CIFISP_CPROC, enable);
}
bool V4l2Isp11Ioctl::setCprocCfg(struct cifisp_cproc_config& cprocCfg, bool enable) {
  SET_ISP_CFG(CIFISP_IOC_S_CPROC, cprocCfg,
              V4L2_CID_CIFISP_CPROC, enable);
}
/* afc */
bool V4l2Isp11Ioctl::getAfcCfg(struct cifisp_afc_config& afcCfg, bool& enable) {
  GET_ISP_CFG(CIFISP_IOC_G_AFC, afcCfg,
              V4L2_CID_CIFISP_AFC, enable);
}
bool V4l2Isp11Ioctl::setAfcCfg(struct cifisp_afc_config& afcCfg, bool enable) {
  SET_ISP_CFG(CIFISP_IOC_S_AFC, afcCfg,
              V4L2_CID_CIFISP_AFC, enable);
}
/* ie */
bool V4l2Isp11Ioctl::getIeCfg(struct cifisp_ie_config& ieCfg, bool& enable) {
  GET_ISP_CFG(CIFISP_IOC_G_IE, ieCfg,
              V4L2_CID_CIFISP_IE, enable);
}
bool V4l2Isp11Ioctl::setIeCfg(struct cifisp_ie_config& ieCfg, bool enable) {
  SET_ISP_CFG(CIFISP_IOC_S_IE, ieCfg,
              V4L2_CID_CIFISP_IE, enable);
}
/* dpf */
bool V4l2Isp11Ioctl::getDpfCfg(struct cifisp_dpf_config& dpfCfg, bool& enable) {
  GET_ISP_CFG(CIFISP_IOC_G_DPF, dpfCfg,
              V4L2_CID_CIFISP_DPF, enable);
}
bool V4l2Isp11Ioctl::setDpfCfg(struct cifisp_dpf_config& dpfCfg, bool enable) {
  SET_ISP_CFG(CIFISP_IOC_S_DPF, dpfCfg,
              V4L2_CID_CIFISP_DPF, enable);
}
bool V4l2Isp11Ioctl::getDpfStrengthCfg(struct cifisp_dpf_strength_config& dpfStrCfg, bool& enable) {
  GET_ISP_CFG(CIFISP_IOC_G_DPF_STRENGTH, dpfStrCfg,
              V4L2_CID_CIFISP_DPF, enable);
}
bool V4l2Isp11Ioctl::setDpfStrengthCfg(struct cifisp_dpf_strength_config& dpfStrCfg, bool enable) {
  SET_ISP_CFG(CIFISP_IOC_S_DPF_STRENGTH, dpfStrCfg,
              V4L2_CID_CIFISP_DPF, enable);
}

bool V4l2Isp11Ioctl::getWdrCfg(struct cifisp_wdr_config& wdrCfg, bool& enable) {
  GET_ISP_CFG(CIFISP_IOC_G_WDR, wdrCfg,
              V4L2_CID_CIFISP_WDR, enable);
}
bool V4l2Isp11Ioctl::setWdrCfg(struct cifisp_wdr_config& wdrCfg, bool enable) {
  SET_ISP_CFG(CIFISP_IOC_S_WDR, wdrCfg,
              V4L2_CID_CIFISP_WDR, enable);
}

/* last capture config */
bool V4l2Isp11Ioctl::getLastCapCfg(struct cifisp_last_capture_config& lastCapCfg) {
  int ret = ioctl(mDevFp, CIFISP_IOC_G_LAST_CONFIG, &lastCapCfg);
  if (ret < 0) {
    return false;
  }
  return true;
}
/* get statistics */
bool V4l2Isp11Ioctl::getAwbStat(struct cifisp_awb_stat& awbStat) {
  UNUSED_PARAM(awbStat);
  //TODO
  return false;
}
bool V4l2Isp11Ioctl::getAeStat(struct cifisp_ae_stat& aeStat) {
  UNUSED_PARAM(aeStat);
  //TODO
  return false;
}
bool V4l2Isp11Ioctl::getAfStat(struct cifisp_af_stat& afStat) {
  UNUSED_PARAM(afStat);
  //TODO
  return false;
}

bool V4l2Isp11Ioctl::setControl(int id, bool enable) {
  struct v4l2_control ctrl;

  ctrl.id = id;

  if (enable)
    ctrl.value = 1;
  else
    ctrl.value = 0;

  int ret = ioctl(mDevFp, VIDIOC_S_CTRL, &ctrl);

  if (ret < 0) {
    //LOGE("ERR(%s):VIDIOC_S_CTRL(id = %#x (%d), value = %d) failed ret = %d\n",
    //    __func__, ctrl.id, ctrl.id-V4L2_CID_PRIVATE_BASE, ctrl.value, ret);

  }

  return ret < 0 ? false : true;
}

bool V4l2Isp11Ioctl::getControl(int id, bool& enable) {
  struct v4l2_control ctrl;

  ctrl.id = id;

  int ret = ioctl(mDevFp, VIDIOC_G_CTRL, &ctrl);

  if (ret < 0) {
    //LOGE("ERR(%s):VIDIOC_S_CTRL(id = %#x (%d), value = %d) failed ret = %d\n",
    //    __func__, ctrl.id, ctrl.id-V4L2_CID_PRIVATE_BASE, ctrl.value, ret);

  }

  if (ctrl.value)
    enable = true;
  else
    enable = false;

  return ret < 0 ? false : true;
}


