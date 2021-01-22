#ifndef _CAM_IA10_ENGINE_H_
#define _CAM_IA10_ENGINE_H_

#include <calib_xml/calibdb.h>
#include <cam_calibdb/cam_calibdb_api.h>

#include "cam_ia_api/cam_ia10_engine_api.h"
#include "cam_ia_api/cameric.h"
#include <awb/awb.h>
#include <awb/awbConvert.h>


class CamIA10Engine: public CamIA10EngineItf {
 public:
  CamIA10Engine();
  virtual ~CamIA10Engine();

  virtual RESULT initStatic(char* aiqb_data_file);
  virtual RESULT initDynamic(struct CamIA10_DyCfg* cfg);
  virtual RESULT setStatistics(struct CamIA10_Stats* stats);

  virtual RESULT runAEC();
  virtual RESULT getAECResults(AecResult_t* result);

  virtual RESULT runAWB();
  virtual RESULT getAWBResults(CamIA10_AWB_Result_t* result);
  virtual RESULT closeAWB();
  virtual RESULT restoreAwbApiSet(struct CamIA10_DyCfg* cfg);

  virtual RESULT runADPF();
  virtual RESULT getADPFResults(AdpfResult_t* result);
  virtual RESULT closeADPF();

  virtual RESULT runAF();
  virtual RESULT getAFResults(CamIA10_AFC_Result_t* result);

  virtual RESULT runAWDR();
  virtual RESULT getAWDRResults(AwdrResult_t* result);

  /* manual ISP configs*/
  virtual RESULT runManISP(struct HAL_ISP_cfg_s* manCfg, struct CamIA10_Results* result);

  virtual void mapSensorExpToHal(
      int sensorGain,
      int sensorInttime,
      float& halGain,
      float& halInttime);

  virtual void mapHalExpToSensor
  (
      float hal_gain,
      float hal_time,
      int& sensor_gain,
      int& sensor_time
  );

  virtual void mapHalWinToIsp(
    uint16_t in_width, uint16_t in_height,
    uint16_t in_hOff, uint16_t in_vOff,
    uint16_t drvWidth, uint16_t drvHeight,
    uint16_t& out_width, uint16_t& out_height,
    uint16_t& out_hOff, uint16_t& out_vOff);

  virtual RESULT getWdrConfig(struct HAL_ISP_wdr_cfg_s* wdr_cfg, enum HAL_ISP_WDR_MODE_e wdr_mode);
  virtual RESULT getAwbLscPfl(struct HAL_ISP_Lsc_Profile_s* awb_lsc_pfl);
  virtual RESULT getDbAdpfDpf(struct HAL_ISP_ADPF_DPF_s* adpf_dpf);
  virtual RESULT getDbAdpfFlt(struct HAL_ISP_FLT_Set_s* adpf_flt);
  virtual RESULT getAwbCcmPfl(struct HAL_ISP_AWB_CCM_GET_s* awb_ccm_pfl);
  virtual RESULT getAwbGainIllu(bool_t* enabled, struct HAL_ISP_AWB_s* awb_gain_illu_pfl);
  virtual RESULT getAwbRefGain(struct HAL_ISP_AWB_RefGain_s* awb_refgain);
  virtual RESULT getAwbCurve(struct HAL_ISP_AWB_Curve_s* awb_curve);
  virtual RESULT getAwbWP(struct HAL_ISP_AWB_White_Point_Get_s* awb_wp_get);
  virtual RESULT getApiGocCfg(struct HAL_ISP_GOC_s* goc);
  virtual RESULT getApiCprocCfg(struct HAL_ISP_cproc_cfg_s* cproc_cfg, struct HAL_ISP_CPROC_s* cproc);

 private:
  void convertAwbResult2Cameric
  (
      AwbRunningOutputResult_t* awbResult,
      CamIA10_AWB_Result_t* awbCamicResult
  );

  void updateAwbConfigs
  (
      CamIA10_AWB_Result_t* old,
      CamIA10_AWB_Result_t* newcfg,
      CamIA10_AWB_Result_t* update
  );

  uint32_t calcAfmTenengradShift(const uint32_t MaxPixelCnt);
  uint32_t calcAfmLuminanceShift(const uint32_t MaxPixelCnt);

  CalibDb calidb;
  CamCalibDbHandle_t  hCamCalibDb;

  struct CamIA10_DyCfg  dCfg;
  struct CamIA10_DyCfg  dCfgShd;

  AfHandle_t  hAf;
  AeHandle_t  hAe;
  AdpfHandle_t  hAdpf;
  AwdrHandle_t  hAwdr;
  AwbHandle_t hAwb;
  AfConfig_t afcCfg;
  AwbConfig_t awbcfg;
  AecConfig_t aecCfg;
  float initLinePeriodsPerField;
  float initPixelClockFreqMHZ;
  float initPixelPeriodsPerLine;
  AdpfConfig_t adpfCfg;
  AwdrConfig_t awdrCfg;
  CamIA10_AWB_Result_t lastAwbResult;
  CamIA10_AWB_Result_t curAwbResult;
  AecResult_t      lastAecResult;
  AecResult_t      curAecResult;
  CamIA10_AFC_Result_t lastAfcResult;
  CamIA10_AFC_Result_t curAfcResult;
  struct CamIA10_Stats mStats;

  bool_t  mWdrEnabledState;
  enum LIGHT_MODE mLightMode;

 private:
  RESULT initAEC();
  RESULT initAWB();
  RESULT initADPF();
  RESULT initAF();
  RESULT initAWDR();
};

#endif

