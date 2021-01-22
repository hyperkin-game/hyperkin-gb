#include <ebase/types.h>
#include <ebase/trace.h>
#include <ebase/builtins.h>
#include <sys/ioctl.h>
#include <calib_xml/calibdb.h>
#include <utils/Log.h>

#include <HAL/CamIsp10CtrItf.h>
#include <af_ctrl.h>
#include <isp_ctrl.h>
#include <rkisp_interface.h>
#include <HAL/camapi_interface.h>

#ifndef __cplusplus
#define __cplusplus
#endif

using namespace std;

typedef struct rkisp_inf_s {
	CamIsp10CtrItf* ispDev;
	CamApiItf* apiItf;
} rkisp_inf_t;

/* -------- CamIsp10CtrItf interface -----------*/
int
rkisp_start(void* &engine, int vidFd, const char* ispNode,
	const char* tuningFile, enum CAMISP_CTRL_MODE ctrl_mode) {
	int ret;
	struct isp_supplemental_sensor_mode_data sensor_mode_data;

	rkisp_inf_t* rkisp_inf = new rkisp_inf_t;
	rkisp_inf->ispDev = new CamIsp10CtrItf(0, vidFd);
	rkisp_inf->apiItf = new CamApiItf();

	//init
	rkisp_inf->apiItf->initApiItf(rkisp_inf->ispDev);

	//config
	ret = getSensorModeData(vidFd, &sensor_mode_data);
	rkisp_inf->apiItf->configIsp_l(&sensor_mode_data);

	//init ispDev
	rkisp_inf->ispDev->init(tuningFile, ispNode/*"/dev/video1"*/, ctrl_mode);
	rkisp_inf->ispDev->start(true);
	engine = rkisp_inf;
	ALOGD("%s: interface isp dev started", __func__);
}

int
rkisp_stop(void* &engine) {
	if (engine != NULL) {
		ALOGD("%s: rkisp interface ready to deinit", __func__);
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		rkisp_inf->ispDev->stop();
		rkisp_inf->ispDev->deInit();
		delete rkisp_inf->apiItf;
		delete rkisp_inf->ispDev;
		rkisp_inf = NULL;
	}
}

int rkisp_getAeTime(void* &engine, float &time)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->getAeTime(time);
	} else {
		return -1;
	}
}

int rkisp_getAeGain(void* &engine, float &gain)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->getAeGain(gain);
	} else {
		return -1;
	}
}

int rkisp_getAeMaxExposureTime(void* &engine, float &time)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->GetAeMaxExposureTime(time);
	} else {
		return -1;
	}
}

int rkisp_getAeMaxExposureGain(void* &engine, float &gain)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->GetAeMaxExposureGain(gain);
	} else {
		return -1;
	}
}

int rkisp_setAeMaxExposureTime(void* &engine, float time)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->SetAeMaxExposureTime(time);
	} else {
		return -1;
	}
}

int rkisp_setAeMaxExposureGain(void* &engine, float gain)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->SetAeMaxExposureGain(gain);
	} else {
		return -1;
	}
}

int rkisp_getAeState(void* &engine, enum HAL_AE_STATE &ae_state)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->getAeState(&ae_state);
	} else {
		return -1;
	}
}

int rkisp_getAeMeanLuma(void* &engine, float &meanLuma)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->getAeMeanLuma(meanLuma);
	} else {
		return -1;
	}
}

int rkisp_setWhiteBalance(void* &engine, HAL_WB_MODE wbMode)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->setWhiteBalance(wbMode);
	} else {
		return -1;
	}
}

int rkisp_setAeMode(void* &engine, enum HAL_AE_OPERATION_MODE aeMode)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->setAeMode(aeMode);
	} else {
		return -1;
	}
}

int rkisp_setManualGainAndTime(void* &engine, float hal_gain, float hal_time)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->setManualGainAndTime(hal_gain, hal_time);
	} else {
		return -1;
	}
}

int rkisp_setAntiBandMode(void* &engine, enum HAL_AE_FLK_MODE flkMode)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->setAntiBandMode(flkMode);
	} else {
		return -1;
	}
}

int rkisp_setAeBias(void* &engine, int aeBias)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->setAeBias(aeBias);
	} else {
		return -1;
	}
}

int rkisp_setFps(void* &engine, HAL_FPS_INFO_t fps)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->setFps(fps);
	} else {
		return -1;
	}
}

int rkisp_getFps(void* &engine, HAL_FPS_INFO_t &fps)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->getFps(fps);
	} else {
		return -1;
	}
}

int rkisp_setAeWindow(void* &engine, int left_hoff, int top_voff, int right_width, int bottom_height)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->setAeWindow(left_hoff, top_voff, right_width, bottom_height);
	} else {
		return -1;
	}
}

int rkisp_getAeWindow(void* &engine, int &left_hoff, int &top_voff, int &right_width, int &bottom_height)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->getAeWindow(left_hoff, top_voff, right_width, bottom_height);
	} else {
		return -1;
	}
}

int rkisp_setExposureMeterMode(void* &engine, enum HAL_AE_METERING_MODE aeMeterMode)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->setExposureMeterMode(aeMeterMode);
	} else {
		return -1;
	}
}

int rkisp_getExposureMeterMode(void* &engine, enum HAL_AE_METERING_MODE& aeMeterMode)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->getExposureMeterMode(aeMeterMode);
	} else {
		return -1;
	}
}

int rkisp_setExposureMeterCoeff(void* &engine, unsigned char meter_coeff[])
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->setExposureMeterCoeff(meter_coeff);
	} else {
		return -1;
	}
}

int rkisp_getExposureMeterCoeff(void* &engine, unsigned char meter_coeff[])
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->getExposureMeterCoeff(meter_coeff);
	} else {
		return -1;
	}
}

int rkisp_setAeSetPoint(void* &engine, float set_point)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->setAeSetPoint(set_point);
	} else {
		return -1;
	}
}

int rkisp_getAeSetPoint(void* &engine, float &set_point)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->getAeSetPoint(set_point);
	} else {
		return -1;
	}
}

int rkisp_set3ALocks(void* &engine, int locks)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->set3ALocks(locks);
	} else {
		return -1;
	}
}

int rkisp_get3ALocks(void* &engine, int& curLocks)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->get3ALocks(curLocks);
	} else {
		return -1;
	}
}

int rkisp_setFocusMode(void* &engine, enum HAL_AF_MODE fcMode)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->setFocusMode(fcMode);
	} else {
		return -1;
	}
}

int rkisp_getFocusMode(void* &engine, enum HAL_AF_MODE& fcMode)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->getFocusMode(fcMode);
	} else {
		return -1;
	}
}

int rkisp_setFocusWin(void* &engine, HAL_Window_t afwin)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->setFocusWin(afwin);
	} else {
		return -1;
	}
}

int rkisp_getFocusWin(void* &engine, HAL_Window_t& afwin)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->getFocusWin(afwin);
	} else {
		return -1;
	}
}

int rkisp_trigggerAf(void* &engine, bool trigger)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->trigggerAf(trigger);
	} else {
		return -1;
	}
}

int rkisp_getBrightness(void* &engine, int& brightVal)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->getApiBrightness(brightVal);
	} else {
		return -1;
	}
}

int rkisp_getContrast(void* &engine, int& contrast)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->getApiContrast(contrast);
	} else {
		return -1;
	}
}

int rkisp_getSaturation(void* &engine, int& sat)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->getApiSaturation(sat);
	} else {
		return -1;
	}
}

int rkisp_getHue(void* &engine, int& hue)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->getApiHue(hue);
	} else {
		return -1;
	}
}

int rkisp_setBrightness(void* &engine, int brightVal)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->setBrightness(brightVal);
	} else {
		return -1;
	}
}

int rkisp_setContrast(void* &engine, int contrast)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->setContrast(contrast);
	} else {
		return -1;
	}
}

int rkisp_setSaturation(void* &engine, int sat)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->setSaturation(sat);
	} else {
		return -1;
	}
}

int rkisp_setHue(void* &engine, int hue)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->setHue(hue);
	} else {
		return -1;
	}
}

int rkisp_setFilterLevel(void* &engine, enum HAL_MODE_e mode,
	enum HAL_FLT_DENOISE_LEVEL_e denoise, enum HAL_FLT_SHARPENING_LEVEL_e sharp)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->setFilterLevel(mode, denoise, sharp);
	} else {
		return -1;
	}
}

int rkisp_getFilterLevel(void* &engine, enum HAL_MODE_e& mode,
	enum HAL_FLT_DENOISE_LEVEL_e& denoise, enum HAL_FLT_SHARPENING_LEVEL_e& sharp)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		return rkisp_inf->apiItf->getFilterLevel(mode, denoise, sharp);
	} else {
		return -1;
	}
}

int rkisp_setNightMode(void* &engine, bool night_mode)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		rkisp_inf->apiItf->setNightMode(night_mode);
		return 0;
	} else {
		return -1;
	}
}

int rkisp_getNightMode(void* &engine, bool& night_mode)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		night_mode = rkisp_inf->apiItf->getNightMode();
		return 0;
	} else {
		return -1;
	}
}

int rkisp_setDayNightSwitch(void* &engine, enum HAL_DAYNIGHT_MODE sw)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		rkisp_inf->apiItf->setDayNightSwitch(sw);
		return 0;
	} else {
		return -1;
	}
}

int rkisp_getDayNightSwitch(void* &engine, enum HAL_DAYNIGHT_MODE& sw)
{
	if (engine != NULL) {
		rkisp_inf_t* rkisp_inf = (rkisp_inf_t*)engine;
		sw = rkisp_inf->apiItf->getDayNightSwitch();
		return 0;
	} else {
		return -1;
	}
}

/* ------------- CamIA10EngineItf interface ---------------------*/
int
rkisp_iq_init(void* engine, const char* tuningFile/*, struct CamIA10_DyCfg* ia_dcfg*/) {
	shared_ptr<CamIA10EngineItf> iqEngine = getCamIA10EngineItf();
	if (iqEngine->initStatic((char*)tuningFile) != RET_SUCCESS) {
		ALOGE("%s: initstatic failed", __func__);
		rkisp_iq_deinit(engine);
		return -1;
	}
	/*
	if (ia_dcfg)
		iqEngine->initDynamic(ia_dcfg);
	*/
	engine = iqEngine.get();
	return 0;
}

void
rkisp_iq_deinit(void* engine) {
	if (engine != NULL) {
		shared_ptr<CamIA10EngineItf> iqEngine =
			shared_ptr<CamIA10EngineItf>((CamIA10EngineItf*)engine);
		iqEngine.reset();
		iqEngine = NULL;
	}
}

int
rkisp_iq_statistics_set(void* engine, struct CamIA10_Stats* ia_stats) {
	if (engine != NULL) {
		shared_ptr<CamIA10EngineItf> iqEngine =
			shared_ptr<CamIA10EngineItf>((CamIA10EngineItf*)engine);
		iqEngine->setStatistics(ia_stats);
	}
	return 0;
}

int
rkisp_iq_ae_run(void* engine) {
	if (engine != NULL) {
		shared_ptr<CamIA10EngineItf> iqEngine =
			shared_ptr<CamIA10EngineItf>((CamIA10EngineItf*)engine);
		iqEngine->runAEC();
	}
	return 0;
}

int
rkisp_iq_get_aec_result(void* engine, AecResult_t* result) {
	if (engine != NULL) {
		shared_ptr<CamIA10EngineItf> iqEngine =
			shared_ptr<CamIA10EngineItf>((CamIA10EngineItf*)engine);
		iqEngine->getAECResults(result);
	}
	return 0;
}

int
rkisp_iq_af_run(void* engine) {
	if (engine != NULL) {
		shared_ptr<CamIA10EngineItf> iqEngine =
			shared_ptr<CamIA10EngineItf>((CamIA10EngineItf*)engine);
		iqEngine->runAF();
	}
	return 0;
}

int
rkisp_iq_get_af_result(void* engine, CamIA10_AFC_Result_t* result) {
	if (engine != NULL) {
		shared_ptr<CamIA10EngineItf> iqEngine =
			shared_ptr<CamIA10EngineItf>((CamIA10EngineItf*)engine);
		iqEngine->getAFResults(result);
	}
	return 0;
}

int
rkisp_iq_awb_run(void* engine) {
	if (engine != NULL) {
		shared_ptr<CamIA10EngineItf> iqEngine =
			shared_ptr<CamIA10EngineItf>((CamIA10EngineItf*)engine);
		iqEngine->runAWB();
	}
	return 0;
}

int
rkisp_iq_get_awb_result(void* engine, CamIA10_AWB_Result_t* result) {
	if (engine != NULL) {
		shared_ptr<CamIA10EngineItf> iqEngine =
			shared_ptr<CamIA10EngineItf>((CamIA10EngineItf*)engine);
		iqEngine->getAWBResults(result);
	}
	return 0;
}

