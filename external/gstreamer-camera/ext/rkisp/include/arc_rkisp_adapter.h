#ifndef __ARC_RKISP_ADAPTER__
#define __ARC_RKISP_ADAPTER__
#include <rk_aiq_types.h>
#include <rk_aiq.h>
#include <cam_ia_api/cam_ia10_engine_api.h>

void convert_to_rkisp_stats(
        const rk_aiq_stats_input_params_t* aiq_stats, struct CamIA10_Stats* stats);

void convert_to_rkisp_aec_params(
        const rk_aiq_aec_input_params_t* aec_input_params, HAL_AecCfg* config);

void convert_from_rkisp_aec_result(
        rk_aiq_aec_result_t* &aec_result, AecResult_t* result);

void convert_to_rkisp_awb_params(
        const rk_aiq_awb_input_params_t* awb_input_params, HAL_AwbCfg* config);

void convert_from_rkisp_awb_result(
        rk_aiq_awb_result_t* awb_result, CamIA10_AWB_Result_t* result);

void convert_to_rkisp_af_params(
        const rk_aiq_af_input_params_t* af_input_params, HAL_AfcCfg* config);

void convert_from_rkisp_af_result(
        rk_aiq_af_result_t* af_result, CamIA10_AFC_Result_t* result);

#endif
