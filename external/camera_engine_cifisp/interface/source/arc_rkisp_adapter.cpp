#include <arc_rkisp_adapter.h>
#include <utils/Log.h>

void convert_to_rkisp_stats(
		const rk_aiq_stats_input_params_t* aiq_stats, struct CamIA10_Stats* stats) {
    //stats->meas_type = aiq_stats.
    //stats->aec.exp_mean = aiq_stats->aec_stats.exp_mean;
    //stats->aec.hist_bins = aiq_stats->aec_stats.hist_bin;

    stats->awb.MeanCr__R = aiq_stats->awb_stats.awb_meas[0].mean_cr__r;
    stats->awb.MeanY__G= aiq_stats->awb_stats.awb_meas[0].mean_y__g;
    stats->awb.MeanCb__B= aiq_stats->awb_stats.awb_meas[0].mean_cb__b;
    stats->awb.NoWhitePixel = aiq_stats->awb_stats.awb_meas[0].num_white_pixel;
/*
    stats->af.LuminanceA = aiq_stats->af_stats.window[0].lum;
    stats->af.PixelCntA= aiq_stats->af_stats.window[0].sum;
    stats->af.LuminanceB = aiq_stats->af_stats.window[1].lum;
    stats->af.PixelCntB= aiq_stats->af_stats.window[1].sum;
    stats->af.LuminanceC = aiq_stats->af_stats.window[2].lum;
    stats->af.PixelCntC= aiq_stats->af_stats.window[2].sum;
*/
}

void convert_to_rkisp_aec_params(
        const rk_aiq_aec_input_params_t* aec_input_params, HAL_AecCfg* config) {

    config->flk = HAL_AE_FLK_AUTO;//aec_input_params->flicker_reduction_mode;
    config->mode = HAL_AE_OPERATION_MODE_AUTO;//aec_input_params->operation_mode;
    config->meter_mode = HAL_AE_METERING_MODE_CENTER;//aec_input_params->metering_mode;
    // window
    if (config->mode != HAL_AE_OPERATION_MODE_AUTO) {
        if (aec_input_params->window == NULL) {
            ALOGD("%s, rk_aiq_aec_input_params_t window is null", __func__);
            return;
        }
        config->win.left_hoff =  aec_input_params->window->h_offset;
        config->win.top_voff =  aec_input_params->window->v_offset;
        config->win.right_width =  aec_input_params->window->v_offset + aec_input_params->window->width;
        config->win.bottom_height =  aec_input_params->window->h_offset + aec_input_params->window->height;
    }
    // bias
    config->ae_bias = (int)(aec_input_params->ev_shift);
}

void convert_from_rkisp_aec_result(
        rk_aiq_aec_result_t* &aec_result, AecResult_t* result) {

    aec_result->exposure.exposure_time_us = result->coarse_integration_time;
    aec_result->exposure.analog_gain = result->analog_gain_code_global;
    aec_result->exposure.digital_gain = result->analog_gain_code_global;
    aec_result->exposure.iso = result->analog_gain_code_global;

    aec_result->sensor_exposure.analog_gain_code_global = result->analog_gain_code_global;
    aec_result->sensor_exposure.coarse_integration_time = result->coarse_integration_time;
    aec_result->sensor_exposure.fine_integration_time = result->regIntegrationTime;
    aec_result->sensor_exposure.digital_gain_global = result->gainFactor;
    aec_result->sensor_exposure.frame_length_lines = result->PixelClockFreqMHZ;
    aec_result->sensor_exposure.line_length_pixels = result->PixelPeriodsPerLine;

    aec_result->flicker_reduction_mode = rk_aiq_ae_flicker_reduction_auto;

    aec_result->aec_config_result.enabled = true;
    aec_result->aec_config_result.win.h_offset = result->meas_win.h_offs;
    aec_result->aec_config_result.win.width= result->meas_win.h_size;
    aec_result->aec_config_result.win.v_offset = result->meas_win.v_offs;
    aec_result->aec_config_result.win.height = result->meas_win.v_size;
    aec_result->aec_config_result.mode = RK_ISP_EXP_MEASURING_MODE_1;

    aec_result->hist_config_result.enabled = true;
    aec_result->hist_config_result.mode = RK_ISP_HIST_MODE_RGB_COMBINED;
    aec_result->hist_config_result.stepSize = result->stepSize;
    //aec_result->hist_config_result.weights = result->GridWeights;
    aec_result->hist_config_result.weights_cnt = RK_AIQ_HISTOGRAM_WEIGHT_GRIDS_SIZE;
    aec_result->hist_config_result.window.h_offset = result->meas_win.h_offs;
    aec_result->hist_config_result.window.width = result->meas_win.h_size;
    aec_result->hist_config_result.window.v_offset = result->meas_win.v_offs;
    aec_result->hist_config_result.window.height = result->meas_win.v_size;

}

void convert_to_rkisp_awb_params(
        const rk_aiq_awb_input_params_t* awb_input_params, HAL_AwbCfg* config) {
        
    config->mode = HAL_WB_AUTO;//awb_input_params->scene_mode;
    if (config->mode != HAL_WB_AUTO) {
        if (awb_input_params->window == NULL) {
            ALOGD("%s, rk_aiq_awb_input_params_t window is null", __func__);
            return;
        }

        config->win.left_hoff = awb_input_params->window->h_offset;
        config->win.top_voff = awb_input_params->window->v_offset;
        config->win.right_width= awb_input_params->window->h_offset + awb_input_params->window->width;
        config->win.bottom_height= awb_input_params->window->v_offset + awb_input_params->window->height;
    }
}

void convert_from_rkisp_awb_result(
        rk_aiq_awb_result_t* awb_input_params, CamIA10_AWB_Result_t* result) {
    
    awb_input_params->awb_meas_cfg.enabled = true;
    awb_input_params->awb_meas_cfg.awb_meas_mode = RK_ISP_AWB_MEASURING_MODE_YCBCR;//result->MeasMode;
    awb_input_params->awb_meas_cfg.awb_meas_cfg.max_y= result->MeasConfig.MaxY;
    awb_input_params->awb_meas_cfg.awb_meas_cfg.ref_cr_max_r= result->MeasConfig.RefCr_MaxR;
    awb_input_params->awb_meas_cfg.awb_meas_cfg.min_y_max_g= result->MeasConfig.MinY_MaxG;
    awb_input_params->awb_meas_cfg.awb_meas_cfg.ref_cb_max_b= result->MeasConfig.RefCb_MaxB;
    awb_input_params->awb_meas_cfg.awb_meas_cfg.max_c_sum= result->MeasConfig.MaxCSum;
    awb_input_params->awb_meas_cfg.awb_meas_cfg.min_c= result->MeasConfig.MinC;

    awb_input_params->awb_gain_cfg.enabled = true;
    awb_input_params->awb_gain_cfg.awb_gains.red_gain = result->awbGains.Red;
    awb_input_params->awb_gain_cfg.awb_gains.green_b_gain= result->awbGains.GreenB;
    awb_input_params->awb_gain_cfg.awb_gains.green_r_gain= result->awbGains.GreenR;
    awb_input_params->awb_gain_cfg.awb_gains.blue_gain= result->awbGains.Blue;

    awb_input_params->ctk_config.enabled = true;
    //awb_input_params->ctk_config.ctk_matrix = result.CcMatrix;
    awb_input_params->ctk_config.cc_offset.red= result->CcOffset.Red;
    awb_input_params->ctk_config.cc_offset.green= result->CcOffset.Green;
    awb_input_params->ctk_config.cc_offset.blue= result->CcOffset.Blue;

    awb_input_params->lsc_cfg.enabled = true;
    awb_input_params->lsc_cfg.config_width;
    awb_input_params->lsc_cfg.config_height;
    awb_input_params->converged = false;
}

void convert_to_rkisp_af_params(
        const rk_aiq_af_input_params_t* af_input_params, HAL_AfcCfg* config) {
    /*
    config->mode = af_input_params->focus_mode;
    config->oneshot_trigger = af_input_params->trigger_new_search;
    config->win_a = af_input_params->focus_rect;
    */
}

void convert_from_rkisp_af_result(
        rk_aiq_af_result_t* af_input_params, CamIA10_AFC_Result_t* result) {
}
