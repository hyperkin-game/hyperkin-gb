#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "../HAL/include/CamIqDebug.h"
#include <libexpat/expat.h>


int anlyMeta(char *fname)
{
  HAL_Buffer_MetaData meta;
  printf("sizeof(HAL_Buffer_MetaData) = %d\n", sizeof(HAL_Buffer_MetaData));

  FILE* yuv_file = fopen(fname, "rb");
  if (yuv_file) {
    fseek(yuv_file, -sizeof(HAL_Buffer_MetaData), SEEK_END);
    fread(&meta, sizeof(HAL_Buffer_MetaData), 1, yuv_file);

    printf("\nawb\n");
    printf("wb_gain.gain_red = %f\n", meta.awb.wb_gain.gain_red);
    printf("wb_gain.gain_green_r = %f\n", meta.awb.wb_gain.gain_green_r);
    printf("wb_gain.gain_blue = %f\n", meta.awb.wb_gain.gain_blue);
    printf("wb_gain.gain_green_b = %f\n", meta.awb.wb_gain.gain_green_b);
    printf("DoorType = %d\n", meta.awb.DoorType);

    printf("\nflt\n");
    printf("denoise_level = %d\n", meta.flt.denoise_level);
    printf("sharp_level = %d\n", meta.flt.sharp_level);

    printf("\nwdr\n");
    printf("wdr_enable = %d\n", meta.wdr.wdr_enable);
    printf("mode = %d\n", meta.wdr.mode);
    printf("dx_used_cnt = %d\n", meta.wdr.dx_used_cnt);
    for (int i=0; i<HAL_ISP_WDR_SECTION_MAX + 1; i++) {
      printf("wdr_dy.wdr_block_dy[%d] = %d\n", i, meta.wdr.wdr_dy.wdr_block_dy[i]);
    }
    for (int i=0; i<HAL_ISP_WDR_SECTION_MAX + 1; i++) {
      printf("wdr_dy.wdr_global_dy[%d] = %d\n", i, meta.wdr.wdr_dy.wdr_global_dy[i]);
    }
    for (int i=0; i<HAL_ISP_WDR_SECTION_MAX; i++) {
      printf("wdr_dx[%d] = %d\n", i, meta.wdr.wdr_dx[i]);
    }

    printf("wdr_noiseratio = %d\n", meta.wdr.wdr_noiseratio);
    printf("wdr_bestlight = %d\n", meta.wdr.wdr_bestlight);
    printf("wdr_gain_off1 = %d\n", meta.wdr.wdr_gain_off1);
    printf("wdr_pym_cc = %d\n", meta.wdr.wdr_pym_cc);
    printf("wdr_epsilon = %d\n", meta.wdr.wdr_epsilon);
    printf("wdr_lvl_en = %d\n", meta.wdr.wdr_lvl_en);
    printf("wdr_flt_sel = %d\n", meta.wdr.wdr_flt_sel);
    printf("wdr_gain_max_clip_enable = %d\n", meta.wdr.wdr_gain_max_clip_enable);
    printf("wdr_gain_max_value = %d\n", meta.wdr.wdr_gain_max_value);
    printf("wdr_bavg_clip = %d\n", meta.wdr.wdr_bavg_clip);
    printf("wdr_nonl_segm = %d\n", meta.wdr.wdr_nonl_segm);
    printf("wdr_nonl_open = %d\n", meta.wdr.wdr_nonl_open);
    printf("wdr_nonl_mode1 = %d\n", meta.wdr.wdr_nonl_mode1);
    printf("wdr_coe0 = %d\n", meta.wdr.wdr_coe0);
    printf("wdr_coe1 = %d\n", meta.wdr.wdr_coe1);
    printf("wdr_coe2 = %d\n", meta.wdr.wdr_coe2);
    printf("wdr_coe_off = %d\n", meta.wdr.wdr_coe_off);

    printf("\ndpf_strength\n");
    printf("r = %f\n", meta.dpf_strength.r);
    printf("g = %f\n", meta.dpf_strength.g);
    printf("b = %f\n", meta.dpf_strength.b);

    printf("\ndpf\n");

    printf("\ndsp_3DNR\n");
    printf("luma_sp_nr_en = %d\n", meta.dsp_3DNR.luma_sp_nr_en);
    printf("luma_te_nr_en = %d\n", meta.dsp_3DNR.luma_te_nr_en);
    printf("chrm_sp_nr_en = %d\n", meta.dsp_3DNR.chrm_sp_nr_en);
    printf("chrm_te_nr_en = %d\n", meta.dsp_3DNR.chrm_te_nr_en);
    printf("shp_en = %d\n", meta.dsp_3DNR.shp_en);
    printf("luma_sp_nr_level = %d\n", meta.dsp_3DNR.luma_sp_nr_level);
    printf("luma_te_nr_level = %d\n", meta.dsp_3DNR.luma_te_nr_level);
    printf("chrm_sp_nr_level = %d\n", meta.dsp_3DNR.chrm_sp_nr_level);
    printf("chrm_te_nr_level = %d\n", meta.dsp_3DNR.chrm_te_nr_level);
    printf("shp_level = %d\n", meta.dsp_3DNR.shp_level);
    printf("luma_default = %d\n", meta.dsp_3DNR.luma_default);
    printf("luma_sp_rad = %d\n", meta.dsp_3DNR.luma_sp_rad);
    printf("luma_te_max_bi_num = %d\n", meta.dsp_3DNR.luma_te_max_bi_num);
    printf("luma_w0 = %d\n", meta.dsp_3DNR.luma_w0);
    printf("luma_w1 = %d\n", meta.dsp_3DNR.luma_w1);
    printf("luma_w2 = %d\n", meta.dsp_3DNR.luma_w2);
    printf("luma_w3 = %d\n", meta.dsp_3DNR.luma_w3);
    printf("luma_w4 = %d\n", meta.dsp_3DNR.luma_w4);
    printf("chrm_default = %d\n", meta.dsp_3DNR.chrm_default);
    printf("chrm_sp_rad = %d\n", meta.dsp_3DNR.chrm_sp_rad);
    printf("chrm_te_max_bi_num = %d\n", meta.dsp_3DNR.chrm_te_max_bi_num);
    printf("chrm_w0 = %d\n", meta.dsp_3DNR.chrm_w0);
    printf("chrm_w1 = %d\n", meta.dsp_3DNR.chrm_w1);
    printf("chrm_w2 = %d\n", meta.dsp_3DNR.chrm_w2);
    printf("chrm_w3 = %d\n", meta.dsp_3DNR.chrm_w3);
    printf("chrm_w4 = %d\n", meta.dsp_3DNR.chrm_w4);
    printf("shp_default = %d\n", meta.dsp_3DNR.shp_default);
    printf("src_shp_w0 = %d\n", meta.dsp_3DNR.src_shp_w0);
    printf("src_shp_w1 = %d\n", meta.dsp_3DNR.src_shp_w1);
    printf("src_shp_w2 = %d\n", meta.dsp_3DNR.src_shp_w2);
    printf("src_shp_w3 = %d\n", meta.dsp_3DNR.src_shp_w3);
    printf("src_shp_w4 = %d\n", meta.dsp_3DNR.src_shp_w4);
    printf("src_shp_thr = %d\n", meta.dsp_3DNR.src_shp_thr);
    printf("src_shp_div = %d\n", meta.dsp_3DNR.src_shp_div);
    printf("src_shp_l = %d\n", meta.dsp_3DNR.src_shp_l);
    printf("src_shp_c = %d\n", meta.dsp_3DNR.src_shp_c);

	printf("\nnew_dsp_3DNR\n");
	printf("enable_3dnr = %d\n", meta.newDsp3DNR.enable_3dnr);
    printf("enable_dpc = %d\n", meta.newDsp3DNR.enable_dpc);
    printf("enable_ynr = %d\n", meta.newDsp3DNR.ynr.enable_ynr);
    printf("enable_tnr = %d\n", meta.newDsp3DNR.ynr.enable_tnr);
	printf("enable_iir = %d\n", meta.newDsp3DNR.ynr.enable_iir);
	printf("ynr_time_weight = %d\n", meta.newDsp3DNR.ynr.ynr_time_weight);
	printf("ynr_spat_weight = %d\n", meta.newDsp3DNR.ynr.ynr_spat_weight);
	printf("enable_uvnr = %d\n", meta.newDsp3DNR.uvnr.enable_uvnr);
	printf("uvnr_weight = %d\n", meta.newDsp3DNR.uvnr.uvnr_weight);
	printf("enable_sharp = %d\n", meta.newDsp3DNR.sharp.enable_sharp);
	printf("sharp_weight = %d\n", meta.newDsp3DNR.sharp.sharp_weight);

    printf("\nctk\n");
    printf("coeff0 = %f\n", meta.ctk.coeff0);
    printf("coeff1 = %f\n", meta.ctk.coeff1);
    printf("coeff2 = %f\n", meta.ctk.coeff2);
    printf("coeff3 = %f\n", meta.ctk.coeff3);
    printf("coeff4 = %f\n", meta.ctk.coeff4);
    printf("coeff5 = %f\n", meta.ctk.coeff5);
    printf("coeff6 = %f\n", meta.ctk.coeff6);
    printf("coeff7 = %f\n", meta.ctk.coeff7);
    printf("coeff8 = %f\n", meta.ctk.coeff8);
    printf("ct_offset_r = %f\n", meta.ctk.ct_offset_r);
    printf("ct_offset_g = %f\n", meta.ctk.ct_offset_g);
    printf("ct_offset_b = %f\n", meta.ctk.ct_offset_b);
    printf("update_mask = %d\n", meta.ctk.update_mask);

    printf("\nlsc\n");
    printf("lsc_enable = %d\n", meta.lsc.lsc_enable);
    for (int i=0; i<HAL_ISP_DATA_TBL_SIZE_MAX; i++) {
      printf("lsc_max_data_tbl[%d] = %d\n", i, meta.lsc.lsc_max_data_tbl[i]);
    }
    printf("data_table_cnt = %d\n", meta.lsc.data_table_cnt);
    printf("grad_table_cnt = %d\n", meta.lsc.grad_table_cnt);
    for (int i=0; i<HAL_ISP_DATA_TBL_SIZE_MAX; i++) {
      printf("LscRDataTbl[%d] = %d\n", i, meta.lsc.LscRDataTbl[i]);
    }
    for (int i=0; i<HAL_ISP_DATA_TBL_SIZE_MAX; i++) {
      printf("LscGRDataTbl[%d] = %d\n", i, meta.lsc.LscGRDataTbl[i]);
    }
    for (int i=0; i<HAL_ISP_DATA_TBL_SIZE_MAX; i++) {
      printf("LscGBDataTbl[%d] = %d\n", i, meta.lsc.LscGBDataTbl[i]);
    }
    for (int i=0; i<HAL_ISP_DATA_TBL_SIZE_MAX; i++) {
      printf("LscBDataTbl[%d] = %d\n", i, meta.lsc.LscBDataTbl[i]);
    }
    for (int i=0; i<HAL_ISP_GRAD_TBL_SIZE_MAX; i++) {
      printf("LscXGradTbl[%d] = %d\n", i, meta.lsc.LscXGradTbl[i]);
    }
    for (int i=0; i<HAL_ISP_GRAD_TBL_SIZE_MAX; i++) {
      printf("LscYGradTbl[%d] = %d\n", i, meta.lsc.LscYGradTbl[i]);
    }
    for (int i=0; i<HAL_ISP_GRAD_TBL_SIZE_MAX; i++) {
      printf("LscXSizeTbl[%d] = %d\n", i, meta.lsc.LscXSizeTbl[i]);
    }
    for (int i=0; i<HAL_ISP_GRAD_TBL_SIZE_MAX; i++) {
      printf("LscYSizeTbl[%d] = %d\n", i, meta.lsc.LscYSizeTbl[i]);
    }

    printf("\ngoc\n");
    printf("meta.goc.mode = %d\n", meta.goc.mode);
    for (int i=0; i<HAL_ISP_GOC_SECTION_MAX; i++) {
      printf("meta.goc.gamma_y[%d] = %d\n", i, meta.goc.gamma_y[i]);
    }

    printf("\n");
    printf("timStamp.tv_sec = %ld\n", meta.timStamp.tv_sec);
    printf("timStamp.tv_usec = %d\n", meta.timStamp.tv_usec);
    printf("exp_time = %f\n", meta.exp_time);
    printf("exp_gain = %f\n", meta.exp_gain);
    for (int i=0; i<HAL_ISP_MODULE_MAX_ID_ID + 1; i++) {
      printf("enabled[%d] = %d\n", i, meta.enabled[i]);
    }

    fclose(yuv_file);
  }
  else
    printf("open %s error !\n", fname);

  return 0;
}

void send_dump_msg(int queue_id, uint32_t pathid, char *path, unsigned int num) {
  bool ret = true;
  rk_ipc_message msg;

  msg.msg_type = RK_IPC_MSG_TYPE_NORMAL;
  msg.msg_id = RK_IPC_MSG_ID_DUMP;
  memset(msg.msg_detail.dump_ctl.dump_path, 0, sizeof(msg.msg_detail.dump_ctl.dump_path));
  memcpy(msg.msg_detail.dump_ctl.dump_path, path, strlen(path));
  msg.msg_detail.dump_ctl.dump_num = num;
  msg.msg_detail.dump_ctl.dump_pathid = pathid;
  if ((msgsnd(queue_id, &msg, sizeof(msg)-sizeof(long), 0)) == -1){
    printf("iqcap: call msgsnd error, %s\n!", strerror(errno));
  }
}

void show_help() {
  printf("\nhelp information:\n");
  printf("iqcap -d MP/SP PATHNAME\n");
  printf("\n");
}

int main(int argc, char* argv[])
{
  key_t key;
  int queue_id;
  rk_ipc_message msg;
  char dump_path[RK_DUMP_PATH_MAX];

  if (argc < 3) {
    printf("wrong command!\n");
    show_help();
    return -1;
  }

  printf("iqcap start\n");
  if ((key=ftok(RK_IPC_MSG_QUEUE_ROUTE, RK_IPC_MSG_QUEUE_KEY)) == -1){
    printf("iqcap: call ftok error, %s!", strerror(errno));
    return -1;
  }
  printf("iqcap key of msgque 0x%x\n", key);

  if ((queue_id=msgget(key, IPC_CREAT)) == -1){
    printf("iqcap: call msgget error, %s!", strerror(errno));
    return -1;
  }

  if (strcmp(argv[1], "-d") == 0){
    int end_pos;
    uint32_t path_id = 0;

    if (strcmp(argv[2], "MP") == 0){
      path_id = 0;
    }
    else if (strcmp(argv[2], "SP") == 0){
      path_id = 1;
    }

    end_pos = strlen(argv[3])-1;
    memset(dump_path, 0, sizeof(dump_path));
    if (end_pos >= 0 && argv[3][end_pos] != '/')
      snprintf(dump_path, RK_DUMP_PATH_MAX, "%s/", argv[3]);
    else
      snprintf(dump_path, RK_DUMP_PATH_MAX, "%s", argv[3]);
    send_dump_msg(queue_id, path_id, dump_path, atoi(argv[4]));
  }
  else if (strcmp(argv[1], "-a") == 0){
    anlyMeta(argv[2]);
  }
  else {
    printf("wrong command!\n");
    show_help();
  }

  printf("iqcap end\n");
  return 0;
}