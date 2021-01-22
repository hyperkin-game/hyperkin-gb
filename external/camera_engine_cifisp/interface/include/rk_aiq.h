#ifndef __RK_AIQ_H__
#define __RK_AIQ_H__

#include "rk_aiq_types.h"

#ifdef __cplusplus                                                                                                                         
extern "C" {
#endif

typedef struct rk_aiq_ctx_s rk_aiq_ctx_t;

rk_aiq_ctx_t* rk_aiq_init(const char* iq_xml_file);

void rk_aiq_deinit(rk_aiq_ctx_t* ctx);

typedef struct rk_aiq_stats_input_params_s {
    unsigned long long                   frame_id;                                /*!< The frame identifier which identifies to which frame the given statistics correspond. Must be positive. */
    unsigned long long                   frame_timestamp;                         /*!< Mandatory although function will not return error, if not given.*/
    rk_aiq_isp_awb_measure_result_t      awb_stats;
    rk_aiq_isp_aec_measure_result_t      aec_stats;
    rk_aiq_isp_af_meas_stat_t            af_stats;
    rk_aiq_exposure_sensor_descriptor_t  sensor_des;
    rk_aiq_pipe_resolution_info_t        pipe_res_info;
} rk_aiq_stats_input_params_t;

int rk_aiq_stats_set(rk_aiq_ctx_t* ctx,
                     const rk_aiq_stats_input_params_t* stats);

typedef struct rk_aiq_aec_input_params_s {
    rk_aiq_frame_use_t frame_use;                                 /*!< Mandatory. Target frame type of the AEC calculations (Preview, Still, video etc.). */
    rk_aiq_flash_mode_t flash_mode;                               /*!< Mandatory. Manual flash mode. If AEC should make flash decision, set mode to ia_aiq_flash_mode_auto. */
    rk_aiq_ae_operation_mode_t operation_mode;                    /*!< Mandatory. AEC operation mode. */
    rk_aiq_ae_metering_mode_t metering_mode;                      /*!< Mandatory. AEC metering mode. */
    rk_aiq_ae_priority_mode_t priority_mode;                      /*!< Mandatory. AEC priority mode. */
    rk_aiq_ae_flicker_reduction_t flicker_reduction_mode;         /*!< Mandatory. AEC flicker reduction mode. */
    rk_aiq_exposure_sensor_descriptor_t *sensor_descriptor;       /*!< Mandatory although function will not return error, if not given.
                                                                     Sensor specific descriptor and limits of the used sensor mode for target frame use.
                                                                     AEC will not limit and calculate sensor specific parameters, if not given */
    rk_aiq_window_t* window;                                    /*!< Optional. standard coordinate <-1000, 1000>*/
    float ev_shift;                                             /*!< Optional. Exposure Value shift [-4,4]. */
    long *manual_exposure_time_us;                              /*!< Optional. Manual exposure time in microseconds. NULL if NA. Otherwise, array of values
                                                                     of num_exposures length. Order of exposure times corresponds to exposure_index of ae_results,
                                                                     e.g., manual_exposure_time_us[ae_results->exposures[0].exposure_index] = 33000; */
    float *manual_analog_gain;                                  /*!< Optional. Manual analog gain. NULL if NA. Otherwise, array of values of num_exposures length.
                                                                     Order of gain values corresponds to exposure_index of ae_results,
                                                                     e.g., manual_analog_gain[ae_results->exposures[0].exposure_index] = 4.0f; */
    short *manual_iso;                                          /*!< Optional. Manual ISO. Overrides manual_analog_gain. NULL if NA. Otherwise, array of values
                                                                     e.g., manual_iso[ae_results->exposures[0].exposure_index] = 100; */

} rk_aiq_aec_input_params_t;

int rk_aiq_aec_run(rk_aiq_ctx_t* ctx,
                   const rk_aiq_aec_input_params_t* aec_input_params,
                   rk_aiq_aec_result_t* aec_result);

typedef struct rk_aiq_awb_input_params_s {
    rk_aiq_frame_use_t frame_use;                       /*!< Mandatory. Target frame type of the AWB calculations (Preview, Still, video etc.). */
    rk_aiq_awb_operation_mode_e scene_mode;             /*!< Mandatory. AWB scene mode. */
    rk_aiq_awb_manual_cct_range_t *manual_cct_range;    /*!< Optional. Manual CCT range. Used only if AWB scene mode 'ia_aiq_awb_operation_manual_cct_range' is used. */
    rk_aiq_window_t* window;                                    /*!< Optional. standard coordinate <-1000, 1000>*/
} rk_aiq_awb_input_params_t;

int rk_aiq_awb_run(rk_aiq_ctx_t* ctx,
                   const rk_aiq_awb_input_params_t* awb_input_params,
                   rk_aiq_awb_result_t* awb_result);

typedef struct rk_aiq_af_input_params_s {
    rk_aiq_frame_use_t frame_use;                       /*!< Mandatory. Target frame type of the AWB calculations (Preview, Still, video etc.). */
    int lens_position;                                          /*!< Mandatory. Current lens position. */
    unsigned long long lens_movement_start_timestamp;           /*!< Mandatory. Lens movement start timestamp in us. Timestamp is compared against statistics timestamp
                                                                     to determine if lens was moving during statistics collection. */
    rk_aiq_af_operation_mode_t focus_mode;                        /*!< Mandatory. Focusing mode. */
    rk_aiq_af_range_t focus_range;                                /*!< Mandatory. Focusing range. Only valid when focus_mode is ia_aiq_af_operation_mode_auto. */
    rk_aiq_af_metering_mode_t focus_metering_mode;                /*!< Mandatory. Metering mode (multispot, touch). */
    rk_aiq_flash_mode_t flash_mode;                               /*!< Mandatory. User setting for flash. */
    rk_aiq_window_t* focus_rect;                                   /*!< Optional. */
    rk_aiq_manual_focus_parameters_t *manual_focus_parameters;    /*!< Optional. Manual focus parameters (manual lens position, manual focusing distance). Used only if
                                                                     focus mode 'ia_aiq_af_operation_mode_manual' is used. */
    bool trigger_new_search;                                    /*!< TRUE if new AF search is needed, FALSE otherwise. Host is responsible for flag cleaning. */
} rk_aiq_af_input_params_t;

int rk_aiq_af_run(rk_aiq_ctx_s* ctx,
                  const rk_aiq_af_input_params_t* af_input_params,
                  rk_aiq_af_result_t* af_result);

//including contrast, hue, brightness, etc.
typedef struct rk_aiq_misc_isp_input_params_s {
   /* TODO */ 
    void* TODO;
} rk_aiq_misc_isp_input_params_t;

int rk_aiq_misc_run(rk_aiq_ctx_t* ctx,
                    const rk_aiq_misc_isp_input_params_t* misc_input_params,
                    rk_aiq_misc_isp_resuts_t* misc_results);
#ifdef __cplusplus                                                                                                                         
}
#endif

#endif //__RK_AIQ_H__
