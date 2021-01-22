#include <rk_aiq.h>
#include <arc_rkisp_adapter.h>
#include <cam_ia_api/cam_ia10_engine_api.h>
#include <cam_ia_api/cam_ia10_engine.h>
#include <utils/Log.h>

rk_aiq_ctx_t* rk_aiq_init(const char* iq_xml_file) {
    CamIA10Engine* iqEngine = new CamIA10Engine();
    if (iqEngine->initStatic((char*)iq_xml_file) != RET_SUCCESS) {
        ALOGE("%s: initstatic failed", __func__);
        rk_aiq_deinit((rk_aiq_ctx_t*)iqEngine);
        return NULL;
    }

    return (rk_aiq_ctx_t*)iqEngine;
}

void rk_aiq_deinit(rk_aiq_ctx_t* ctx) {
    if (ctx != NULL) {
        CamIA10Engine* iqEngine =
            (CamIA10Engine*)ctx;
        delete iqEngine;
        iqEngine = NULL;
    }
}

int rk_aiq_stats_set(rk_aiq_ctx_t* ctx,
                     const rk_aiq_stats_input_params_t* stats) {
    if (ctx != NULL) {
        CamIA10Engine* iqEngine =
            (CamIA10Engine*)ctx;
		struct CamIA10_Stats sensor_stats;
		convert_to_rkisp_stats(stats, &sensor_stats);
        iqEngine->setStatistics(&sensor_stats);
    }
    return 0;
}

int rk_aiq_aec_run(rk_aiq_ctx_t* ctx,
                   const rk_aiq_aec_input_params_t* aec_input_params,
                   rk_aiq_aec_result_t* aec_result) {
    if (ctx != NULL) {
        CamIA10Engine* iqEngine =
            (CamIA10Engine*)ctx;
		HAL_AecCfg config;
        convert_to_rkisp_aec_params(aec_input_params, &config);
        iqEngine->runAEC();
		AecResult_t result;
        iqEngine->getAECResults(&result);
		convert_from_rkisp_aec_result(aec_result, &result);
    }
    return 0;
}

int rk_aiq_awb_run(rk_aiq_ctx_t* ctx,
                   const rk_aiq_awb_input_params_t* awb_input_params,
                   rk_aiq_awb_result_t* awb_result) {
    if (ctx != NULL) {
        CamIA10Engine* iqEngine =
            (CamIA10Engine*)ctx;
		HAL_AwbCfg config;
		CamIA10_AWB_Result_t result;
		convert_to_rkisp_awb_params(awb_input_params, &config);
        iqEngine->runAWB();
        iqEngine->getAWBResults(&result);
		convert_from_rkisp_awb_result(awb_result, &result);
    }
    return 0;
}

int rk_aiq_af_run(rk_aiq_ctx_s* ctx,
                  const rk_aiq_af_input_params_t* af_input_params,
                  rk_aiq_af_result_t* af_result) {
    if (ctx != NULL) {
        CamIA10Engine* iqEngine =
            (CamIA10Engine*)ctx;
		HAL_AfcCfg config;
		CamIA10_AFC_Result_t result;
		convert_to_rkisp_af_params(af_input_params, &config);
        iqEngine->runAF();
		iqEngine->getAFResults(&result);
		convert_from_rkisp_af_result(af_result, &result);
    }
    return 0;
}

int rk_aiq_misc_run(rk_aiq_ctx_t* ctx,
                    const rk_aiq_misc_isp_input_params_t* misc_input_params,
                    rk_aiq_misc_isp_resuts_t** misc_results) {
}
