#include "CameraIspTunning.h"
#ifdef RK_ISP10
#include <linux/v4l2-controls.h>
#include <media/rk-isp10-config.h>
#include "CamIsp10DevHwItf.h"
#endif
#ifdef RK_ISP11
#include <linux/v4l2-controls.h>
#include <media/rk-isp11-config.h>
#include "CamIsp11DevHwItf.h"
#endif
#include <libexpat/expat.h>
#include "cam_ia_api/cam_ia10_trace.h"
#include <math.h>

#define TRACE_E LOGE

#define CAMERIC_MI_DATAMODE_RAW12 0
#define CAMERIC_MI_DATAMODE_YUV422 1
#define CAMERIC_MI_DATAMODE_RAW8 2
#define CAMERIC_MI_DATAMODE_RAW10 3

#define TUNNING_MAX_GAIN (128.0f)
#define TUNNING_MAX_INTEGRATION_TIME (0.1f)

#define GAIN_MISSING_RANGE 0.15


char CameraIspTunning::mStorePath[HAL_ISP_STORE_PATH_LEN+1] = { 0 };
CameraIspTunning::CameraIspTunning(CamHwItf* camHw):
  StreamPUBase("PU_ISP_TUNNING", true, true),
  mCamHw(camHw) {
  osMutexInit(&mTaskThMutex);
  osEventInit(&mCurTaskOverEvt, true, 0);
  osEventInit(&mCurAllTaskOverEvt, true, 0);
  osEventReset(&mCurAllTaskOverEvt);
}

CameraIspTunning::~CameraIspTunning() {
  while (mTuneTaskcount > 0) {
    osSleep(100);
  }
  stop();
  int nCamTuneTask = mTuneInfoVector.size();
  while (--nCamTuneTask >= 0)
    free(mTuneInfoVector[nCamTuneTask]);
  osMutexDestroy(&mTaskThMutex);
  osEventDestroy(&mCurTaskOverEvt);
  osEventDestroy(&mCurAllTaskOverEvt);
}

static int gTaskIndForXMLParser = 0;
CameraIspTunning* CameraIspTunning::createInstance(CamHwItf* camHw) {
  FILE* fp = NULL;

  CameraIspTunning* profiles = NULL;
  const int BUFF_SIZE = 1024;

  strcpy(&mStorePath[0], RK_ISP_TUNNING_IMAGE_STORE_PATH);
  fp = fopen(RK_ISP_TUNNING_FILE_PATH, "r");
  if (!fp) {
    LOGE("%s:open %s failed!!\n", __func__, RK_ISP_TUNNING_FILE_PATH);
    return profiles;
  }

  LOGD("open xml file(%s) success\n", RK_ISP_TUNNING_FILE_PATH);

  profiles = new CameraIspTunning(camHw);
  gTaskIndForXMLParser = 0;
  XML_Parser parser = XML_ParserCreate(NULL);
  if (parser == NULL) {
    LOGE("XML_ParserCreate failed\n");
    goto fail;
  }

  XML_SetUserData(parser, profiles);
  XML_SetElementHandler(parser, StartElementHandler, NULL);

  for (;;) {
    void* buff = ::XML_GetBuffer(parser, BUFF_SIZE);
    if (buff == NULL) {
      LOGE("failed to in call to XML_GetBuffer()");
      goto fail;
    }

    int bytes_read = ::fread(buff, 1, BUFF_SIZE, fp);
    if (bytes_read < 0) {
      LOGE("failed in call to read");
      goto fail;
    }

    int res = XML_ParseBuffer(parser, bytes_read, bytes_read == 0);
    if (res != 1) {
      LOGE("XML_ParseBuffer error or susppend (%d)\n", res);
    }

    if (bytes_read == 0)
      break;  // done parsing the xml file
  }
  //delete disabled task
  {
    int nCamTuneTask = profiles->mTuneInfoVector.size();
    while (--nCamTuneTask >= 0) {
      if (profiles->mTuneInfoVector[nCamTuneTask]->mTuneEnable == false) {
        free(profiles->mTuneInfoVector[nCamTuneTask]);
        profiles->mTuneInfoVector.erase(profiles->mTuneInfoVector.begin() + nCamTuneTask);
        LOGD("%s:delete this %d cap task from queue!", __func__, nCamTuneTask);
      }
    }
    profiles->mTuneTaskcount = profiles->mTuneInfoVector.size();
    if (profiles->mTuneTaskcount <= 0) {
      LOGD("%s: WARNING:all tasks are disabled !!", __func__);
      goto  fail;
    }
    profiles->mCurTunIndex    =   0;
  }

  LOGD("%s:task count is %d \n", __func__, profiles->mTuneTaskcount);
  goto exit;
fail:
  if (profiles) {
    delete profiles;
    profiles = NULL;
  }
exit:
  if (parser)
    XML_ParserFree(parser);
  fclose(fp);

  return profiles;

}

CameraIspTunning* CameraIspTunning::createInstance(CamHwItf* camHw, struct HAL_ISP_Cap_Req_s* cap_req) {
  int store_path_len;
  CameraIspTunning* profiles = NULL;

  profiles = new CameraIspTunning(camHw);

  CameraIspTunning* pCamIspTunning = profiles;
  ispTuneTaskInfo_s* pCamTuneTaskInfo = NULL;
  ispTuneTaskInfo_s* pNewCaptureTask = NULL;
  pNewCaptureTask = (ispTuneTaskInfo_s*)malloc(sizeof(ispTuneTaskInfo_s));
  if (pNewCaptureTask) {
    pCamIspTunning->mTuneInfoVector.push_back(pNewCaptureTask);
    pCamTuneTaskInfo = pNewCaptureTask;
    memset(pCamTuneTaskInfo, 0, sizeof(ispTuneTaskInfo_s));
    pCamTuneTaskInfo->mWhiteBalance.whiteBalanceMode = WHITEBALANCE_MODE_INVALID;
    pCamTuneTaskInfo->mExpose.exposuseMode  =   EXPOSUSE_MODE_INVALID;
    //get capture info
    if (cap_req->cap_format == HAL_ISP_FMT_RAW12) {
      pCamTuneTaskInfo->mTuneFmt = CAMERIC_MI_DATAMODE_RAW12;
    } else if (cap_req->cap_format == HAL_ISP_FMT_RAW10) {
      pCamTuneTaskInfo->mTuneFmt = CAMERIC_MI_DATAMODE_RAW10;
    } else if (cap_req->cap_format == HAL_ISP_FMT_YUV420 || cap_req->cap_format == HAL_ISP_FMT_YUV422) {
      pCamTuneTaskInfo->mTuneFmt = CAMERIC_MI_DATAMODE_YUV422;
    } else {
      LOGE("%s:not suppot this format 0x%x now !", __func__, cap_req->cap_format);
      goto fail;
    }
    store_path_len = strlen((char *)cap_req->store_path);
    memset(&mStorePath[0], 0, sizeof(mStorePath));
    if (store_path_len > 0) {
      strcpy(&mStorePath[0], (char *)&cap_req->store_path[0]);
      if (mStorePath[store_path_len-1] != '/')
        mStorePath[store_path_len] = '/';
    } else {
      strcpy(&mStorePath[0], RK_ISP_TUNNING_IMAGE_STORE_PATH);
    }
    pCamTuneTaskInfo->mTunePicNum = cap_req->cap_num;
    pCamTuneTaskInfo->mTuneEnable = true;
    pCamTuneTaskInfo->mTuneWidth = cap_req->cap_width;
    pCamTuneTaskInfo->mTuneHeight = cap_req->cap_height;
    if (cap_req->ae_mode == HAL_ISP_AE_MANUAL)
      pCamTuneTaskInfo->mExpose.exposuseMode = EXPOSUSE_MODE_MANUAL;
    else
      pCamTuneTaskInfo->mExpose.exposuseMode = EXPOSUSE_MODE_AUTO;
    pCamTuneTaskInfo->mExpose.integrationTime = ((float)cap_req->exp_time_h + cap_req->exp_time_l / 256.0) / 1000.0;
    pCamTuneTaskInfo->mExpose.gain = (float)cap_req->exp_gain_h + cap_req->exp_gain_l / 256.0;
  } else {
    LOGE("%s:alloc pNewCaptureTask failed !", __func__);
    goto fail;
  }
  profiles->mCurTunIndex = 0;
  profiles->mTuneTaskcount = profiles->mTuneInfoVector.size();
  LOGD("%s:task count is %d \n", __func__, profiles->mTuneTaskcount);
  return profiles;
fail:
  if (profiles) {
    delete profiles;
    profiles = NULL;
  }
  return NULL;
}

//need process result frame buffer
bool CameraIspTunning::prepare(
    const frm_info_t& frmFmt,
    unsigned int numBuffers,
    shared_ptr<CameraBufferAllocator> allocator) {
  //
  ALOGW("WARNING: will ignore the request fmt,do nothing!");
  mAlloc = allocator;
  return StreamPUBase::prepare(frmFmt, 0, NULL);
}

bool CameraIspTunning::start() {
  //path start
  bool ret = false;

  ret = StreamPUBase::start();
  //start task thread
  if (!ret) {
    ALOGE("%s:%d,failed!", __func__, __LINE__);
    return ret;
  }

  osMutexLock(&mTaskThMutex);
  osEventReset(&mCurTaskOverEvt);
  //reset task infos
  mTuneTaskcount = mTuneInfoVector.size();
  mCurTunIndex    =   0;

  mTaskThRunning = true;
  mTaskThread = shared_ptr<CamThread>(new taskThread(this));
  if (mTaskThread->run("tunningTask"))
    ret = false;
  osMutexUnlock(&mTaskThMutex);
  return ret;
}
void CameraIspTunning::stop() {
  osEventWait(&mCurAllTaskOverEvt);
  //stop task thread
  osMutexLock(&mTaskThMutex);
  mTaskThRunning = false;
  osEventSignal(&mCurTaskOverEvt);
  osMutexUnlock(&mTaskThMutex);
  if (mTaskThread.get())
    mTaskThread->requestExitAndWait();
  mTaskThread.reset();

  //stop
  StreamPUBase::stop();
}

//#define TEST_WRITE_SP_TO_FILE

class CambufNotifierImp : public NewCameraBufferReadyNotifier {
 public:
  CambufNotifierImp():drv_exp_time(0.0f),drv_exp_gain(0.0f) {}
  ~CambufNotifierImp() {}
  virtual bool bufferReady(weak_ptr<BufferBase> buffer, int status) {
    UNUSED_PARAM(status);
    shared_ptr<BufferBase> spCamBuf = buffer.lock();

	if (status != 0) {
        ALOGD("receive error buffer, status %d", status);
        return true;
      }

    if (spCamBuf.get()) {
      struct HAL_Buffer_MetaData* metaData = spCamBuf->getMetaData();
      if (metaData) {
        ALOGD("expousre gain : %f %f",
              metaData->exp_time, metaData->exp_gain);
        drv_exp_time = metaData->exp_time;
        drv_exp_gain = metaData->exp_gain;
      }

#ifdef TEST_WRITE_SP_TO_FILE
      //write to file
      char fname[50] = {0};
      static int frames = 0;
      snprintf(fname, 30, "/tmp/yuv_%dx%d.bin", spCamBuf->getWidth(), spCamBuf->getHeight());
      frames++;
      if ((frames > 50) && (frames < 52)) {
        FILE* yuv_file =  fopen(fname, "a+");
        if (yuv_file) {
          fwrite(spCamBuf->getVirtAddr(), spCamBuf->getDataSize(), 1, yuv_file);
          ALOGD("write 0x%x bytes to file!", spCamBuf->getDataSize());
          fclose(yuv_file);
        } else
          ALOGE("open file %s error", fname);
      }
#endif
    }
    return true;
  }
  float drv_exp_time;
  float drv_exp_gain;
};


bool CameraIspTunning::taskThLoop() {
  bool ret = true;
#ifdef RK_ISP10
  CamIsp10DevHwItf* ispDev = dynamic_cast<CamIsp10DevHwItf*>(mCamHw);
#else
  CamIsp11DevHwItf* ispDev = dynamic_cast<CamIsp11DevHwItf*>(mCamHw);
#endif
  osMutexLock(&mTaskThMutex);
  if (mTaskThRunning && (mTuneTaskcount-- > 0)) {
    osMutexUnlock(&mTaskThMutex);
    int curPreW = 0, curPreH = 0;
    float gain, setgain, mingain, maxgain, gainstep, time, settime, mintime, maxtime, timestep;
    setgain = 0.0f;
    settime = 0.0f;
    mSkipFrmNum = 4;
    //TODO
    frm_info_t& fmt  = mCurFmt;
    shared_ptr<CamHwItf::PathBase> path = mCamHw->getPath(CamHwItf::MP);
    //get the current task
    ispTuneTaskInfo_s* curTuneTask = mTuneInfoVector[mCurTunIndex];
    curPreW = curTuneTask->mTuneWidth;
    curPreH = curTuneTask->mTuneHeight;
    mCurCapNum = curTuneTask->mTunePicNum;
    mCurCapIdx = 0;
    if (curTuneTask->mTuneFmt == CAMERIC_MI_DATAMODE_RAW12) {
      //TODO: may  try fomat
      fmt.frmFmt = HAL_FRMAE_FMT_SBGGR12;
      fmt.frmSize.width = curPreW;
      fmt.frmSize.height = curPreH;
      curTuneTask->mForceRGBOut = false;
    } else if (curTuneTask->mTuneFmt == CAMERIC_MI_DATAMODE_RAW10) {
      //TODO: may  try fomat
      fmt.frmFmt = HAL_FRMAE_FMT_SBGGR10;
      fmt.frmSize.width = curPreW;
      fmt.frmSize.height = curPreH;
      curTuneTask->mForceRGBOut = false;
    } else if (curTuneTask->mTuneFmt == CAMERIC_MI_DATAMODE_RAW8) {
      //TODO: may  try fomat
      fmt.frmFmt = HAL_FRMAE_FMT_SBGGR8;
      fmt.frmSize.width = curPreW;
      fmt.frmSize.height = curPreH;
      curTuneTask->mForceRGBOut = false;
    } else if (curTuneTask->mTuneFmt == CAMERIC_MI_DATAMODE_YUV422) {
      fmt.frmFmt = HAL_FRMAE_FMT_NV16;
      fmt.frmSize.width = curPreW;
      fmt.frmSize.height = curPreH;
      curTuneTask->mForceRGBOut = true;
    } else {
      LOGE("this format %d is not support", curTuneTask->mTuneFmt);
    }

    //task settings
    //0. disable all isp modules
    struct HAL_ISP_cfg_s ispCfgs;
    memset(&ispCfgs, 0, sizeof(ispCfgs));
    ispCfgs.updated_mask = 0;
    //ispDev->configureISPModules(&ispCfgs);
    //1. ae setting
    struct HAL_ISP_aec_cfg_s aec_cfg = {0};
    if (curTuneTask->mExpose.exposuseMode == EXPOSUSE_MODE_AUTO) {
      if (curTuneTask->mTuneFmt == CAMERIC_MI_DATAMODE_RAW12 ||
          curTuneTask->mTuneFmt == CAMERIC_MI_DATAMODE_RAW10 ||
          curTuneTask->mTuneFmt == CAMERIC_MI_DATAMODE_RAW8) {
        //get tune task res ae value
        RK_FRMAE_FORMAT taskFmt = fmt.frmFmt;
        fmt.frmFmt = HAL_FRMAE_FMT_NV12;
        if (path.get()) {
          path->prepare(
              fmt,
              4,
              *(mAlloc.get()),
              false);
          CambufNotifierImp* notifier = new CambufNotifierImp();
          path->addBufferNotifier(notifier);
          path->start();
          //wait ae stable
          osSleep(3000);
          //TODO:get cur gain and integrationtime
          setgain = notifier->drv_exp_gain;
          settime = notifier->drv_exp_time;
          ALOGD("%s:ae time %f,gain %f", __func__, settime, setgain);
          path->removeBufferNotifer(notifier);
          path->stop();
          path->releaseBuffers();
          delete notifier;
        }
        fmt.frmFmt = taskFmt;
        ispCfgs.updated_mask |= HAL_ISP_AEC_MASK;
        ispCfgs.enabled[HAL_ISP_AEC_ID] = HAL_ISP_ACTIVE_FALSE;
        aec_cfg.exp_gain = setgain;
        aec_cfg.exp_time = settime;
        ispCfgs.aec_cfg = &aec_cfg;
      } else {
        ispCfgs.updated_mask |= HAL_ISP_AEC_MASK | HAL_ISP_HST_MASK;
        ispCfgs.enabled[HAL_ISP_AEC_ID] = HAL_ISP_ACTIVE_DEFAULT;
        ispCfgs.enabled[HAL_ISP_HST_ID] = HAL_ISP_ACTIVE_DEFAULT;
        mSkipFrmNum = 20;
      }
    } else {
      setgain = curTuneTask->mExpose.gain;
      settime = curTuneTask->mExpose.integrationTime;
      if (curTuneTask->mExpose.aeRound)
        mCurAeRoundNum = curTuneTask->mExpose.number;
      ispCfgs.updated_mask |= HAL_ISP_AEC_MASK;
      ispCfgs.enabled[HAL_ISP_AEC_ID] = HAL_ISP_ACTIVE_FALSE;
      aec_cfg.exp_gain = setgain;
      aec_cfg.exp_time = settime;
      ispCfgs.aec_cfg = &aec_cfg;
    }
    //TODO:sensor otp disable
    //only can set ae for RAW mode, can set ae before or after path prepare .
    //2. awb
    struct HAL_ISP_awb_meas_cfg_s awb_meas_cfg;
    struct HAL_ISP_ctk_cfg_s ctk_cfg;
    if (curTuneTask->mWhiteBalance.whiteBalanceMode == WHITEBALANCE_MODE_AUTO) {
      ispCfgs.updated_mask |= HAL_ISP_AWB_MEAS_MASK;
      ispCfgs.enabled[HAL_ISP_AWB_MEAS_ID] = HAL_ISP_ACTIVE_DEFAULT;
      ispCfgs.updated_mask |= HAL_ISP_AWB_GAIN_MASK;
      ispCfgs.enabled[HAL_ISP_AWB_GAIN_ID] = HAL_ISP_ACTIVE_DEFAULT;
      ispCfgs.updated_mask |= HAL_ISP_LSC_MASK;
      ispCfgs.enabled[HAL_ISP_LSC_ID] = HAL_ISP_ACTIVE_DEFAULT;
      ispCfgs.updated_mask |= HAL_ISP_CTK_MASK;
      ispCfgs.enabled[HAL_ISP_CTK_ID] = HAL_ISP_ACTIVE_DEFAULT;
    } else if (curTuneTask->mWhiteBalance.whiteBalanceMode == WHITEBALANCE_MODE_MANUAL) {
      //set manual awb
      uint32_t illu_index = 1;
      if (strcmp(curTuneTask->mWhiteBalance.illumination, "A") == 0)
        illu_index = 0;
      else if (strcmp(curTuneTask->mWhiteBalance.illumination, "D65") == 0)
        illu_index = 1;
      else if (strcmp(curTuneTask->mWhiteBalance.illumination, "CWF") == 0)
        illu_index = 2;
      else if (strcmp(curTuneTask->mWhiteBalance.illumination, "TL84") == 0)
        illu_index = 3;
      else if (strcmp(curTuneTask->mWhiteBalance.illumination, "D50") == 0)
        illu_index = 4;
      else if (strcmp(curTuneTask->mWhiteBalance.illumination, "D75") == 0)
        illu_index = 5;
      else if (strcmp(curTuneTask->mWhiteBalance.illumination, "HORIZON") == 0)
        illu_index = 6;
      else {
        LOGE("not support this illum %s ,set to D65!!!", curTuneTask->mWhiteBalance.illumination);
      }

      LOGD("current illum is %s ,illu_index %d !!!", curTuneTask->mWhiteBalance.illumination, illu_index);

      ispCfgs.updated_mask |= HAL_ISP_AWB_MEAS_MASK;
      ispCfgs.enabled[HAL_ISP_AWB_MEAS_ID] = HAL_ISP_ACTIVE_FALSE;
      ispCfgs.updated_mask |= HAL_ISP_LSC_MASK;
      ispCfgs.enabled[HAL_ISP_LSC_ID] = HAL_ISP_ACTIVE_DEFAULT;
      awb_meas_cfg.illuIndex = illu_index;
      ispCfgs.awb_cfg = &awb_meas_cfg;

      //ctk_cfg.
      if ((strcmp(curTuneTask->mWhiteBalance.cc_matrix, "unit_matrix") == 0)
          && (strcmp(curTuneTask->mWhiteBalance.cc_offset, "zero") == 0)) {
        ispCfgs.updated_mask |= HAL_ISP_CTK_MASK;
        ispCfgs.enabled[HAL_ISP_CTK_ID] = HAL_ISP_ACTIVE_FALSE;
      } else if ((strcmp(curTuneTask->mWhiteBalance.cc_matrix, "default") == 0)
                 && (strcmp(curTuneTask->mWhiteBalance.cc_offset, "default") == 0)) {
        ispCfgs.updated_mask |= HAL_ISP_CTK_MASK;
        ispCfgs.enabled[HAL_ISP_CTK_ID] = HAL_ISP_ACTIVE_DEFAULT;
      } else if ((strcmp(curTuneTask->mWhiteBalance.cc_matrix, "unit_matrix") == 0)
                 && (strcmp(curTuneTask->mWhiteBalance.cc_offset, "default") == 0)) {
        ispCfgs.updated_mask |= HAL_ISP_CTK_MASK;
        ispCfgs.enabled[HAL_ISP_CTK_ID] = HAL_ISP_ACTIVE_SETTING;
        ctk_cfg.coeff0 =  1.0f;
        ctk_cfg.coeff1 = 0.0f;
        ctk_cfg.coeff2 = 0.0f;
        ctk_cfg.coeff3 = 0.0f;
        ctk_cfg.coeff4 = 1.0f;
        ctk_cfg.coeff5 = 0.0f;
        ctk_cfg.coeff6 = 0.0f;
        ctk_cfg.coeff7 = 0.0f;
        ctk_cfg.coeff8 = 1.0f;
        ctk_cfg.update_mask = HAL_ISP_CTK_UPDATE_CC_MATRIX;
        ispCfgs.ctk_cfg = &ctk_cfg;
      } else if ((strcmp(curTuneTask->mWhiteBalance.cc_matrix, "default") == 0)
                 && (strcmp(curTuneTask->mWhiteBalance.cc_offset, "zero") == 0)) {
        ispCfgs.updated_mask |= HAL_ISP_CTK_MASK;
        ispCfgs.enabled[HAL_ISP_CTK_ID] = HAL_ISP_ACTIVE_SETTING;
        ctk_cfg.ct_offset_b = 0.0f;
        ctk_cfg.ct_offset_g = 0.0f;
        ctk_cfg.ct_offset_r = 0.0f;
        ctk_cfg.update_mask = HAL_ISP_CTK_UPDATE_CC_OFFSET;
        ispCfgs.ctk_cfg = &ctk_cfg;
      } else {
        LOGE("not support this cc_matrix %s !!!", curTuneTask->mWhiteBalance.cc_matrix);
        LOGE("not support this cc_offset %s !!!", curTuneTask->mWhiteBalance.cc_offset);
      }

      if (strcmp(curTuneTask->mWhiteBalance.rggb_gain, "unit") == 0) {
        ispCfgs.updated_mask |= HAL_ISP_AWB_GAIN_MASK;
        ispCfgs.enabled[HAL_ISP_AWB_GAIN_ID] = HAL_ISP_ACTIVE_FALSE;
      } else if (strcmp(curTuneTask->mWhiteBalance.rggb_gain, "default") == 0) {
        ispCfgs.updated_mask |= HAL_ISP_AWB_GAIN_MASK;
        ispCfgs.enabled[HAL_ISP_AWB_GAIN_ID] = HAL_ISP_ACTIVE_DEFAULT;
      } else {
        LOGE("not support this rggb_gain %s !!!", curTuneTask->mWhiteBalance.rggb_gain);
      }
    }

    //3. af
    //TODO
    //4. others
    if (curTuneTask->mDpccEnable == true) {
      ispCfgs.updated_mask |= HAL_ISP_BPC_MASK;
      ispCfgs.enabled[HAL_ISP_BPC_ID] = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      ispCfgs.updated_mask |= HAL_ISP_BPC_MASK;
      ispCfgs.enabled[HAL_ISP_BPC_ID] = HAL_ISP_ACTIVE_FALSE;
    }

    if (curTuneTask->mLscEnable == true) {
      ispCfgs.updated_mask |= HAL_ISP_LSC_MASK;
      ispCfgs.enabled[HAL_ISP_LSC_ID] = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      ispCfgs.updated_mask |= HAL_ISP_LSC_MASK;
      ispCfgs.enabled[HAL_ISP_LSC_ID] = HAL_ISP_ACTIVE_FALSE;
    }
    //TODO :CAC
    //if(curTuneTask->mCacEnable== true)
    //    m_camDevice->cacEnable();

    if (curTuneTask->mGammarEnable == true) {
      ispCfgs.updated_mask |= HAL_ISP_GOC_MASK;
      ispCfgs.enabled[HAL_ISP_GOC_ID] = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      ispCfgs.updated_mask |= HAL_ISP_GOC_MASK;
      ispCfgs.enabled[HAL_ISP_GOC_ID] = HAL_ISP_ACTIVE_FALSE;
    }

    if (curTuneTask->mWdrEnable == true) {
      ispCfgs.updated_mask |= HAL_ISP_WDR_MASK;
      ispCfgs.enabled[HAL_ISP_WDR_ID] = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      ispCfgs.updated_mask |= HAL_ISP_WDR_MASK;
      ispCfgs.enabled[HAL_ISP_WDR_ID] = HAL_ISP_ACTIVE_FALSE;
    }

    if (curTuneTask->mBlsEnable == true) {
      ispCfgs.updated_mask |= HAL_ISP_BLS_MASK;
      ispCfgs.enabled[HAL_ISP_BLS_ID] = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      ispCfgs.updated_mask |= HAL_ISP_BLS_MASK;
      ispCfgs.enabled[HAL_ISP_BLS_ID] = HAL_ISP_ACTIVE_FALSE;
    }

    if (curTuneTask->mAdpfEnable == true) {
      ispCfgs.updated_mask |= HAL_ISP_DPF_MASK;
      ispCfgs.enabled[HAL_ISP_DPF_ID] = HAL_ISP_ACTIVE_DEFAULT;
      ispCfgs.updated_mask |= HAL_ISP_DPF_STRENGTH_MASK;
      ispCfgs.enabled[HAL_ISP_DPF_STRENGTH_ID] = HAL_ISP_ACTIVE_DEFAULT;
    } else {
      ispCfgs.updated_mask |= HAL_ISP_DPF_MASK;
      ispCfgs.enabled[HAL_ISP_DPF_ID] = HAL_ISP_ACTIVE_FALSE;
      ispCfgs.updated_mask |= HAL_ISP_DPF_STRENGTH_MASK;
      ispCfgs.enabled[HAL_ISP_DPF_STRENGTH_ID] = HAL_ISP_ACTIVE_FALSE;
    }

    //TODOS: bls,sdg,flt,bdm,cproc,ie,dpf,dpf strength use default if unconfiged
    //5. config to isp
    ispDev->configureISPModules(&ispCfgs);
    mCurGain = setgain;
    mCurIntegrationTime = settime;

    //6. streaming
    //prepare path
    //start path
    path->prepare(
        fmt,
        4,
        *(mAlloc.get()),
        false);
    path->start();
    //add notifer to path, start process frame
    path->addBufferNotifier(this);
    //wait for complete signal
    LOGD("current task is running,wait for complete......");
    osEventWait(&mCurTaskOverEvt);
    LOGD("current task is finished...");
    osMutexLock(&mTaskThMutex);
    //remove notifer
    path->removeBufferNotifer(this);
    //stop path
    path->stop();
    //return all unprocessed buffers
    releaseBuffers();
    path->releaseBuffers();
    mCurTunIndex++;
    osMutexUnlock(&mTaskThMutex);
  } else {
    LOGD("all tasks have been finished or cancelled.");
    LOGD("exit tunning thread");
    ret = false;
    osEventSignal(&mCurAllTaskOverEvt);
    osMutexUnlock(&mTaskThMutex);
  }
  return ret;
}


bool CameraIspTunning::processFrame(shared_ptr<BufferBase> inBuf, shared_ptr<BufferBase> outBuf) {
  float curgain, curtime, newtime, newgain, maxtime, maxgain, settime, setgain;
  bool isStore = false;
  char szBaseFileName[100];

  osMutexLock(&mTaskThMutex);
  //get the current task
  ispTuneTaskInfo_s* curTuneTask = mTuneInfoVector[mCurTunIndex];
  struct HAL_Buffer_MetaData* metaData = inBuf->getMetaData();
  //LOG1("tunning thread receive a frame !!");
  //
  //ALOGD("receive frame %d, addr 0x%p", inBuf->getIndex(), inBuf->getVirtAddr());
  if (!mTaskThRunning || (mCurCapNum <= 0) || (mSkipFrmNum-- > 0)) {
    ALOGD("skip frame %d",mSkipFrmNum);
    goto PROCESS_OVER;
  }
  //TODO:get cur gain & time
  if (metaData) {
    curtime = metaData->exp_time;
    curgain = metaData->exp_gain;
    ALOGD("%s:cur exposure time %f,gain %f",__func__,curtime,curgain);
    if ((curtime < 0.00001f) || (curgain < 1.0f)) {
      ALOGW("%s:get expoure time failed,discard this frame!", __func__);
      goto PROCESS_OVER;
    }
  } else {
    //TODO: should never  be here,but now something goes wrong.
    ALOGE("%s:meta data is NULL", __func__);
    goto PROCESS_OVER;
  }

  {
#ifdef TEST_WRITE_SP_TO_FILE
    //write to file
    char fname[50] = {0};
    static int frames = 0;
    snprintf(fname, 50, "/tmp/yuv_ae_%dx%d.bin", inBuf->getWidth(), inBuf->getHeight());
    frames++;
    if (/*(frames > 50) && (frames < 52)*/1) {
      FILE* yuv_file =  fopen(fname, "a+");
      if (yuv_file) {
        fwrite(inBuf->getVirtAddr(), inBuf->getDataSize(), 1, yuv_file);
        ALOGD("write 0x%x bytes to file!", inBuf->getDataSize());
        fclose(yuv_file);
      } else
        ALOGE("open file %s error", fname);
    }
#endif
  }

  maxtime = TUNNING_MAX_INTEGRATION_TIME;
  maxgain = TUNNING_MAX_GAIN;
  //is it the satisfied frame ?
  if (curTuneTask->mExpose.exposuseMode == EXPOSUSE_MODE_MANUAL) {
#ifdef RK_ISP10
    CamIsp10DevHwItf* ispDev = dynamic_cast<CamIsp10DevHwItf*>(mCamHw);
#else
    CamIsp11DevHwItf* ispDev = dynamic_cast<CamIsp11DevHwItf*>(mCamHw);
#endif
    struct HAL_ISP_cfg_s ispCfgs;
    memset(&ispCfgs, 0, sizeof(ispCfgs));
    struct HAL_ISP_aec_cfg_s aec_cfg = {0};
    ispCfgs.updated_mask = HAL_ISP_AEC_MASK;
    ispCfgs.enabled[HAL_ISP_AEC_ID] = HAL_ISP_ACTIVE_FALSE;
    bool isSetExp = true;
    if ((fabs(curgain - mCurGain) > GAIN_MISSING_RANGE) || (fabs((curtime - mCurIntegrationTime)) > 0.0001)) {
      LOGD("%s:not the desired exposure frame,skip it,time(%0.6f,%0.6f),gain(%0.6f,%0.6f)", __func__,
           mCurIntegrationTime, curtime, mCurGain, curgain);
      goto PROCESS_OVER;
    }
    //set different exposure parameter ?
    if (curTuneTask->mTuneFmt == CAMERIC_MI_DATAMODE_RAW12 ||
        curTuneTask->mTuneFmt == CAMERIC_MI_DATAMODE_RAW10 ||
        curTuneTask->mTuneFmt == CAMERIC_MI_DATAMODE_RAW8) {
      //TODO:get maxtime an maxgain
      if ((curTuneTask->mExpose.integrationTimeStep != 0) || (curTuneTask->mExpose.gainStep != 0)) {
        newtime = mCurIntegrationTime + curTuneTask->mExpose.integrationTimeStep;
        if ((newtime > maxtime) && ((newtime - maxtime) - curTuneTask->mExpose.integrationTimeStep < 0.0001)) {
          //next loop
          //LOGD("set new gain+++");
          newgain = mCurGain + curTuneTask->mExpose.gainStep;
          newtime = curTuneTask->mExpose.integrationTime;
          if ((newgain > maxgain) && ((newgain - maxgain) - curTuneTask->mExpose.gainStep < 0.00001)) {
            mCurCapNum = 0;
            LOGD("time and gain are max ,newgain %0.6f,%0.6f!!!!", newgain, maxgain);
            isSetExp = false;
          }
        } else
          newgain = mCurGain;

        if (isSetExp) {
          //TODO:set new time and gain
          aec_cfg.exp_gain = newgain;
          aec_cfg.exp_time = newtime;
          settime = newtime;
          setgain = newgain;
          ispCfgs.aec_cfg = &aec_cfg;
          ispDev->configureISPModules(&ispCfgs);
          LOGD("setIntegrationTime(desired:%0.6f,real:%0.6f)", newtime, settime);
          LOGD("setGain(desired:%0.6f,real:%0.6f)", newgain, setgain);
          mCurGain = newgain;
          mCurIntegrationTime = newtime;
          mSkipFrmNum = 4;
        }
        int filter_result = ispTuneDesiredExp((long)(inBuf->getVirtAddr()),
                                              mCurFmt.frmSize.width, mCurFmt.frmSize.height,
                                              curTuneTask->mExpose.minRaw, curTuneTask->mExpose.maxRaw,
                                              curTuneTask->mExpose.threshold);
        if (filter_result & 0x1) {

          goto PROCESS_OVER;
        } else if (filter_result & 0x2) {
          LOGD("frame is overhead exposure , finish capture frame !!");
          // curTuneTask->mTunePicNum = 0;
          goto PROCESS_OVER;
        }
      }
    } else if (curTuneTask->mTuneFmt == CAMERIC_MI_DATAMODE_YUV422) {
      if (curTuneTask->mExpose.aeRound == true) {
        newtime = curTuneTask->mExpose.integrationTime
                  + curTuneTask->mExpose.integrationTimeStep * ((mCurAeRoundNum + 1) / 2);
        newgain = curTuneTask->mExpose.gain + curTuneTask->mExpose.gainStep * (mCurAeRoundNum / 2);
        aec_cfg.exp_gain = newgain;
        aec_cfg.exp_time = newtime;
        settime = newtime;
        setgain = newgain;
        ispCfgs.aec_cfg = &aec_cfg;
        ispDev->configureISPModules(&ispCfgs);
        //TODO:set new time and gain
        mCurGain = newgain;
        mCurIntegrationTime = newtime;
        mSkipFrmNum = 4;

        LOGD("setIntegrationTime(desired:%0.6f,real:%0.6f)", newtime, settime);
        LOGD("setGain(desired:%0.6f,real:%0.6f)", newgain, setgain);
        if (mCurAeRoundNum == 1) {
          mCurAeRoundNum -= 3;
        } else
          mCurAeRoundNum--;
        if (mCurAeRoundNum < (-(curTuneTask->mExpose.number + 3))) {
          curTuneTask->mTunePicNum = 0;
        }

      }
    }

  }

  mCurCapNum--;
  mCurCapIdx++;
  //generate base file name

  snprintf(szBaseFileName, sizeof(szBaseFileName) - 1, "%st%0.6f_g%0.6f",
           mStorePath, curtime, curgain);
  szBaseFileName[99] = '\0';

  //store this frame
  curTuneTask->y_addr = (unsigned long)(inBuf->getVirtAddr());
  if (curTuneTask->mTuneFmt == CAMERIC_MI_DATAMODE_YUV422) {
    curTuneTask->uv_addr = curTuneTask->y_addr +
                           mCurFmt.frmSize.width * mCurFmt.frmSize.height;
    curTuneTask->mForceRGBOut = true;
  } else {
    curTuneTask->mForceRGBOut = false;
  }

  ispTuneStoreBuffer(curTuneTask, inBuf.get(), szBaseFileName, mCurCapIdx);
PROCESS_OVER:

  //current task has been finished ? start next capture?
  if (mCurCapNum <= 0) {
    //TODO: notify the task has been completed
    osEventSignal(&mCurTaskOverEvt);
  }
  osMutexUnlock(&mTaskThMutex);
  return true;
}

void CameraIspTunning::StartElementHandler(void* userData, const char* name, const char** atts) {
  CameraIspTunning* pCamIspTunning = (CameraIspTunning*) userData;
  ispTuneTaskInfo_s* pCamTuneTaskInfo = NULL;
  if (!pCamIspTunning->mTuneInfoVector.empty())
    pCamTuneTaskInfo = pCamIspTunning->mTuneInfoVector[gTaskIndForXMLParser - 1];
  if (strcmp(name, "Capture") == 0) {
    ispTuneTaskInfo_s* pNewCaptureTask = (ispTuneTaskInfo_s*)malloc(sizeof(ispTuneTaskInfo_s));
    if (pNewCaptureTask) {
      pCamIspTunning->mTuneInfoVector.push_back(pNewCaptureTask);
      pCamTuneTaskInfo = pNewCaptureTask;
      memset(pCamTuneTaskInfo, 0, sizeof(ispTuneTaskInfo_s));
      pCamTuneTaskInfo->mWhiteBalance.whiteBalanceMode = WHITEBALANCE_MODE_INVALID;
      pCamTuneTaskInfo->mExpose.exposuseMode  =   EXPOSUSE_MODE_INVALID;
      //get capture info
      if (strcmp(atts[1], "CamSys_Fmt_Raw_12b") == 0)
        pCamTuneTaskInfo->mTuneFmt = CAMERIC_MI_DATAMODE_RAW12;
      else if (strcmp(atts[1], "CamSys_Fmt_Raw_8b") == 0)
        pCamTuneTaskInfo->mTuneFmt = CAMERIC_MI_DATAMODE_RAW8;
      else if (strcmp(atts[1], "CamSys_Fmt_Raw_10b") == 0)
        pCamTuneTaskInfo->mTuneFmt = CAMERIC_MI_DATAMODE_RAW10;
      else if (strcmp(atts[1], "CamSys_Fmt_Yuv422_8b") == 0)
        pCamTuneTaskInfo->mTuneFmt = CAMERIC_MI_DATAMODE_YUV422;
      else
        LOGE("%s:not suppot this format %s now !", __func__, atts[1]);
      pCamTuneTaskInfo->mTunePicNum = atoi(atts[3]);
      pCamTuneTaskInfo->mTuneEnable = (atoi(atts[5]) == 0) ? false : true;
      gTaskIndForXMLParser++;

    } else {
      LOGE("%s:alloc pNewCaptureTask failed !", __func__);
      return;
    }
  } else if (strcmp(name, "Resolution") == 0) {
    pCamTuneTaskInfo->mTuneWidth = atoi(atts[1]);
    pCamTuneTaskInfo->mTuneHeight = atoi(atts[3]);
    LOGD("parser:resolution(%dx%d)", pCamTuneTaskInfo->mTuneWidth, pCamTuneTaskInfo->mTuneHeight);

  } else if (strcmp(name, "Exposure") == 0) {
    if (strcmp(atts[1], "manual") == 0)
      pCamTuneTaskInfo->mExpose.exposuseMode = EXPOSUSE_MODE_MANUAL;
    else if (strcmp(atts[1], "auto") == 0)
      pCamTuneTaskInfo->mExpose.exposuseMode = EXPOSUSE_MODE_AUTO;
    else
      pCamTuneTaskInfo->mExpose.exposuseMode = EXPOSUSE_MODE_INVALID;

  } else if (strcmp(name, "Mec") == 0) {
    pCamTuneTaskInfo->mExpose.integrationTime = atof(atts[1]) / 1000;
    pCamTuneTaskInfo->mExpose.gain = atof(atts[3]);
    pCamTuneTaskInfo->mExpose.integrationTimeStep = atof(atts[5]) / 1000;
    pCamTuneTaskInfo->mExpose.gainStep = atof(atts[7]);
    pCamTuneTaskInfo->mExpose.minRaw = atoi(atts[9]);
    pCamTuneTaskInfo->mExpose.maxRaw = atoi(atts[11]);
    pCamTuneTaskInfo->mExpose.threshold = atoi(atts[13]);
    pCamTuneTaskInfo->mExpose.aeRound = (strcmp(atts[15], "true") == 0) ? true : false;
    pCamTuneTaskInfo->mExpose.number = atoi(atts[17]);
  } else if (strcmp(name, "Wdr") == 0) {
    pCamTuneTaskInfo->mWdrEnable = (strcmp(atts[1], "enable") == 0) ? true : false;
  } else if (strcmp(name, "Cac") == 0) {
    pCamTuneTaskInfo->mCacEnable = (strcmp(atts[1], "enable") == 0) ? true : false;
  } else if (strcmp(name, "Gamma") == 0) {
    pCamTuneTaskInfo->mGammarEnable = (strcmp(atts[1], "enable") == 0) ? true : false;
  } else if (strcmp(name, "Lsc") == 0) {
    pCamTuneTaskInfo->mLscEnable = (strcmp(atts[1], "enable") == 0) ? true : false;
  } else if (strcmp(name, "Dpcc") == 0) {
    pCamTuneTaskInfo->mDpccEnable = (strcmp(atts[1], "enable") == 0) ? true : false;
  } else if (strcmp(name, "Bls") == 0) {
    pCamTuneTaskInfo->mBlsEnable = (strcmp(atts[1], "enable") == 0) ? true : false;
  } else if (strcmp(name, "Adpf") == 0) {
    pCamTuneTaskInfo->mAdpfEnable = (strcmp(atts[1], "enable") == 0) ? true : false;
  } else if (strcmp(name, "Avs") == 0) {
    pCamTuneTaskInfo->mAvsEnable = (strcmp(atts[1], "enable") == 0) ? true : false;
  } else if (strcmp(name, "Af") == 0) {
    pCamTuneTaskInfo->mAfEnable = (strcmp(atts[1], "enable") == 0) ? true : false;
  } else if (strcmp(name, "WhiteBalance") == 0) {
    memset(&pCamTuneTaskInfo->mWhiteBalance, 0, sizeof(pCamTuneTaskInfo->mWhiteBalance));
    if (strcmp(atts[1], "manual") == 0)
      pCamTuneTaskInfo->mWhiteBalance.whiteBalanceMode = WHITEBALANCE_MODE_MANUAL;
    else if (strcmp(atts[1], "auto") == 0)
      pCamTuneTaskInfo->mWhiteBalance.whiteBalanceMode = WHITEBALANCE_MODE_AUTO;
    else
      pCamTuneTaskInfo->mWhiteBalance.whiteBalanceMode = WHITEBALANCE_MODE_INVALID;

  } else if (strcmp(name, "Mwb") == 0) {
    strncpy(pCamTuneTaskInfo->mWhiteBalance.illumination, atts[1], strlen(atts[1]));
    strncpy(pCamTuneTaskInfo->mWhiteBalance.cc_matrix, atts[3], strlen(atts[3]));
    strncpy(pCamTuneTaskInfo->mWhiteBalance.cc_offset, atts[5], strlen(atts[5]));
    strncpy(pCamTuneTaskInfo->mWhiteBalance.rggb_gain, atts[7], strlen(atts[7]));
  }

}


int CameraIspTunning::ispTuneStoreBufferRAW
(
    ispTuneTaskInfo_s*    pIspTuneTaskInfo,
    FILE*                pFile,
    BufferBase*       pBuffer,
    bool              putHeader,
    bool              is16bit
) {
  int result = 0;
  long long TimeStampUs = 0;
  struct timeval ts;
  pBuffer->getTimestamp(&ts);
  TimeStampUs == ts.tv_sec * 1000000 + ts.tv_usec;
  TRACE_D(1, "%s (enter)\n", __FUNCTION__);

  if (!pIspTuneTaskInfo) {
    return -1;
  }

  if (!pFile) {
    return -1;
  }


  // get base address & size of local plane
  uint32_t RawPlaneSize = pBuffer->getDataSize();
  uint8_t* pRawTmp, *pRawBase;
  pRawTmp = pRawBase = (uint8_t*) pIspTuneTaskInfo->y_addr;

  if (1) {
    // write out raw image; no matter what pSomContext->ForceRGBOut requests

    // write pgm header
    fprintf(pFile,
            "%sP5\n%d %d\n#####<DCT Raw>\n#<Type>%u</Type>\n#<Layout>%u</Layout>\n#<TimeStampUs>%lli</TimeStampUs>\n#####</DCT Raw>\n%d\n",
            putHeader ? "" : "\n", pBuffer->getWidth(), pBuffer->getHeight(),
            0u, 0u, TimeStampUs, is16bit ? 65535 : 255);

    // write raw plane to file
    if (1 != fwrite(pRawBase, RawPlaneSize, 1, pFile)) {
      result = -1;
    }
  }

  TRACE_D(1, "%s (exit)\n", __FUNCTION__);
  return result;
}


void CameraIspTunning::ConvertYCbCr444combToRGBcomb
(
    uint8_t*     pYCbCr444,
    uint32_t    PlaneSizePixel
) {
  uint32_t pix;
  for (pix = 0; pix < PlaneSizePixel; pix++) {
    // where to put the RGB data
    uint8_t* pPix = pYCbCr444;

    // get YCbCr pixel data
    int32_t Y  = *pYCbCr444++;
    int32_t Cb = *pYCbCr444++; // TODO: order in marvin output is CrCb and not CbCr as expected
    int32_t Cr = *pYCbCr444++; //       s. above

    // remove offset as in VideoDemystified 3; page 18f; YCbCr to RGB(0..255)
    Y  -=  16;
    Cr -= 128;
    Cb -= 128;

    // convert to RGB
////#define USE_FLOAT
#if (1)
    // Standard Definition TV (BT.601) as in VideoDemystified 3; page 18f; YCbCr to RGB(0..255)
#ifdef USE_FLOAT
    float R = 1.164 * Y + 1.596 * Cr;
    float G = 1.164 * Y - 0.813 * Cr - 0.391 * Cb;
    float B = 1.164 * Y + 2.018 * Cb;
#else
    int32_t R = (((int32_t)(1.164 * 1024)) * Y + ((int32_t)(1.596 * 1024)) * Cr) >> 10;
    int32_t G = (((int32_t)(1.164 * 1024)) * Y - ((int32_t)(0.813 * 1024)) * Cr - ((int32_t)(0.391 * 1024)) * Cb) >> 10;
    int32_t B = (((int32_t)(1.164 * 1024)) * Y + ((int32_t)(2.018 * 1024)) * Cb) >> 10;
#endif
#else
    // High Definition TV (BT.709) as in VideoDemystified 3; page 19; YCbCr to RGB(0..255)
#ifdef USE_FLOAT
    float R = 1.164 * Y + 1.793 * Cr;
    float G = 1.164 * Y - 0.534 * Cr - 0.213 * Cb;
    float B = 1.164 * Y + 2.115 * Cb;
#else
    int32_t R = (((int32_t)(1.164 * 1024)) * Y + ((int32_t)(1.793 * 1024)) * Cr) >> 10;
    int32_t G = (((int32_t)(1.164 * 1024)) * Y - ((int32_t)(0.534 * 1024)) * Cr - ((int32_t)(0.213 * 1024)) * Cb) >> 10;
    int32_t B = (((int32_t)(1.164 * 1024)) * Y + ((int32_t)(2.115 * 1024)) * Cb) >> 10;
#endif
#endif
    // clip
    if (R < 0) R = 0; else if (R > 255) R = 255;
    if (G < 0) G = 0; else if (G > 255) G = 255;
    if (B < 0) B = 0; else if (B > 255) B = 255;

    // write back RGB data
    *pPix++ = (uint8_t) R;
    *pPix++ = (uint8_t) G;
    *pPix++ = (uint8_t) B;
  }
}

int CameraIspTunning::ispTuneStoreBufferYUV422Semi
(
    ispTuneTaskInfo_s*    pIspTuneTaskInfo,
    FILE*                pFile,
    BufferBase*       pBuffer,
    bool              putHeader
) {
  int result = 0;
  TRACE_D(1, "%s:%d (enter)\n", __FUNCTION__, __LINE__);

  if (!pIspTuneTaskInfo) {
    return -1;
  }

  if (!pFile) {
    return -1;
  }
  // get base addresses & sizes of local planes
  uint32_t YCPlaneSize = pBuffer->getWidth() * pBuffer->getHeight();
  uint8_t* pYTmp, *pYBase, *pCbCrTmp, *pCbCrBase;
  pYTmp    = pYBase    = (uint8_t*) pIspTuneTaskInfo->y_addr;  // pPicBufMetaData->Data.YCbCr.semiplanar.Y.pBuffer;
  pCbCrTmp = pCbCrBase = (uint8_t*) pIspTuneTaskInfo->uv_addr;  // pPicBufMetaData->Data.YCbCr.semiplanar.CbCr.pBuffer;


  // write out raw or RGB image?
  if (!pIspTuneTaskInfo->mForceRGBOut) {

    // write pgm header
    fprintf(pFile, "%sP5\n%d %d\n255\n", putHeader ? "" : "\n", pBuffer->getWidth(),  pBuffer->getHeight());
    // a single write will do
    if (1 != fwrite(pYBase, 2 * YCPlaneSize, 1, pFile)) {
      result = -1;
    }
  } else {

    // we need a temporary helper buffer capable of holding 3 times the YPlane size (upscaled to 4:4:4 by pixel replication)
    uint8_t* pYCbCr444 = (uint8_t*)malloc(3 * YCPlaneSize);
    if (pYCbCr444 == NULL) {
      result = -1;
    } else {
      // upscale and combine each 4:2:2 pixel to 4:4:4 while removing any gaps at line ends as well
      uint8_t* pYCbCr444Tmp = pYCbCr444;
      uint32_t x, y;
      for (y = 0; y < pBuffer->getHeight(); y++) {
        // get line starts
        uint8_t* pY = pYTmp;
        uint8_t* pC = pCbCrTmp;

        // walk through line
        for (x = 0; x < pBuffer->getWidth(); x += 2) {
          uint8_t Cb, Cr;
          *pYCbCr444Tmp++ = *pY++;
          *pYCbCr444Tmp++ = Cb = *pC++;
          *pYCbCr444Tmp++ = Cr = *pC++;
          *pYCbCr444Tmp++ = *pY++;
          *pYCbCr444Tmp++ = Cb;
          *pYCbCr444Tmp++ = Cr;
        }

        // update line starts
        pYTmp    += pBuffer->getWidth();
        pCbCrTmp += pBuffer->getWidth();
      }

      // inplace convert consecutive YCbCr444 to RGB; both are combined color component planes
      ConvertYCbCr444combToRGBcomb(pYCbCr444, YCPlaneSize);

      // write ppm header
      fprintf(pFile, "%sP6\n%d %d\n255\n", putHeader ? "" : "", pBuffer->getWidth(), pBuffer->getHeight());

      // finally write result
      if (1 != fwrite(pYCbCr444, 3 * YCPlaneSize, 1, pFile)) {
        result = -1;
      }

      // release helper buffer
      free(pYCbCr444);
    }
  }


  TRACE_D(1, "%s (exit)\n", __FUNCTION__);
  return result;
}

int CameraIspTunning::ispTuneStoreBuffer
(
    ispTuneTaskInfo_s*    pIspTuneTaskInfo,
    BufferBase*       pBuffer,
    char*     szNmae,
    int      index
) {
  int result = 0;
  FILE* pStoreFile = NULL;

  char szFileName[FILENAME_MAX] = "";
  char szFileNameRight[FILENAME_MAX] = "";
  uint32_t width  = 0;
  uint32_t height = 0;
  bool   isLeftRight = false;
  bool   isNewFile = false;

  TRACE_D(1, "%s:%d (enter)\n", __FUNCTION__, __LINE__);

  if ((pIspTuneTaskInfo == NULL) || (pBuffer == NULL)) {
    return -1;
  }

  // get dimensions for filename creation
  width = pBuffer->getWidth();
  height = pBuffer->getHeight();
  if (strcmp(pBuffer->getFormat(), RK_HAL_FMT_STRING::HAL_FMT_STRING_NV16) == 0)
    isLeftRight = true;

  if (result != 0) {
    return result;
  }


  // open new file?
  if (1) {
    isNewFile = true;

    // build filename first:
    // ...get suitable file extension (.raw, .jpg, .yuv, .rgb)
    // ...get data type & layout string
    const char* szFileExt = "";
    bool isData = false;
    bool isJpeg = false;
    const char* szTypeLayout = ""; // not used for DATA or JPEG
    if (strcmp(pBuffer->getFormat(), RK_HAL_FMT_STRING::HAL_FMT_STRING_NV16) == 0) {
      szFileExt = pIspTuneTaskInfo->mForceRGBOut ? ".ppm" : ".pgm";
      szTypeLayout = pIspTuneTaskInfo->mForceRGBOut ? "" : "_yuv422_semi";
    } else if (strcmp(pBuffer->getFormat(), RK_HAL_FMT_STRING::HAL_FMT_STRING_SBGGR8) == 0) {
      szFileExt  = ".pgm";
      szTypeLayout = "_raw8_BGGR";
    } else if (strcmp(pBuffer->getFormat(), RK_HAL_FMT_STRING::HAL_FMT_STRING_SGBRG8) == 0) {
      szFileExt  = ".pgm";
      szTypeLayout = "_raw8_GBRG";
    } else if (strcmp(pBuffer->getFormat(), RK_HAL_FMT_STRING::HAL_FMT_STRING_SGRBG8) == 0) {
      szFileExt  = ".pgm";
      szTypeLayout = "_raw8_GRBG";
    } else if (strcmp(pBuffer->getFormat(), RK_HAL_FMT_STRING::HAL_FMT_STRING_SRGGB8) == 0) {
      szFileExt  = ".pgm";
      szTypeLayout = "_raw8_RGGB";
    } else if (strcmp(pBuffer->getFormat(), RK_HAL_FMT_STRING::HAL_FMT_STRING_SBGGR10) == 0) {
      szFileExt  = ".pgm";
      szTypeLayout = "_raw16_BGGR";
    } else if (strcmp(pBuffer->getFormat(), RK_HAL_FMT_STRING::HAL_FMT_STRING_SGBRG10) == 0) {
      szFileExt  = ".pgm";
      szTypeLayout = "_raw16_GBRG";
    } else if (strcmp(pBuffer->getFormat(), RK_HAL_FMT_STRING::HAL_FMT_STRING_SGRBG10) == 0) {
      szFileExt  = ".pgm";
      szTypeLayout = "_raw16_GRBG";
    } else if (strcmp(pBuffer->getFormat(), RK_HAL_FMT_STRING::HAL_FMT_STRING_SRGGB10) == 0) {
      szFileExt  = ".pgm";
      szTypeLayout = "_raw16_RGGB";
    } else if (strcmp(pBuffer->getFormat(), RK_HAL_FMT_STRING::HAL_FMT_STRING_SBGGR12) == 0) {
      szFileExt  = ".pgm";
      szTypeLayout = "_raw16_BGGR";
    } else if (strcmp(pBuffer->getFormat(), RK_HAL_FMT_STRING::HAL_FMT_STRING_SGBRG12) == 0) {
      szFileExt  = ".pgm";
      szTypeLayout = "_raw16_GBRG";
    } else if (strcmp(pBuffer->getFormat(), RK_HAL_FMT_STRING::HAL_FMT_STRING_SGRBG12) == 0) {
      szFileExt  = ".pgm";
      szTypeLayout = "_raw16_GRBG";
    } else if (strcmp(pBuffer->getFormat(), RK_HAL_FMT_STRING::HAL_FMT_STRING_SRGGB12) == 0) {
      szFileExt  = ".pgm";
      szTypeLayout = "_raw16_RGGB";
    } else {
      result = -1;
    }

    if (result != 0) {
      return result;
    }

    // ...create image dimension string
    char szDimensions[20] = "";
    if (!isData && !isJpeg) { // but neither for DATA nor for JPEG
      snprintf(szDimensions, sizeof(szDimensions) - 1, "_%dx%d", width, height);
    }

    // ...create date/time string
    char szDateTime[20] = "";
    if (!isJpeg) { // but not for JPEG
      time_t t;
      t = time(NULL);

      // always use creation time of first file for file name in a sequence of files
      strftime(szDateTime, sizeof(szDateTime), "_%Y%m%d_%H%M%S", localtime(&t));
    }

    // ...create sequence number string
    char szNumber[20] = "";
    snprintf(szNumber, sizeof(szNumber) - 1, "_%04d", index);

    // ...combine all parts
    uint32_t combLen;
    if (strstr(szFileExt, "ppm"))
      combLen = strlen(szNmae) + strlen(szDimensions)  + strlen(szTypeLayout) + strlen(szDateTime) + strlen(pIspTuneTaskInfo->mWhiteBalance.illumination) + strlen(szFileExt);
    else
      combLen = strlen(szNmae) + strlen(szDimensions)  + strlen(szTypeLayout) + strlen(szDateTime)/*+strlen(szNumber)*/ + strlen(szFileExt);

    if (combLen >= FILENAME_MAX) {
      TRACE_E("%s Max filename length exceeded.\n"
              " len(BaseFileName) = %3d\n"
              " len(Dimensions)   = %3d\n"
              " len(szTypeLayout)   = %3d\n"
              " len(szDateTime)   = %3d\n"
              " len(szNumber)   = %3d\n"
              " len(FileExt)      = %3d\n"
              " --------------------------\n"
              " combLen        >= %3d\n",
              __FUNCTION__,
              strlen(szNmae), strlen(szDimensions), strlen(szTypeLayout),
              strlen(szDateTime), strlen(szNumber), strlen(szFileExt), combLen);
      return -1;
    }

    if (strstr(szFileExt, "ppm") && strlen(pIspTuneTaskInfo->mWhiteBalance.illumination))
      snprintf(szFileName, FILENAME_MAX, "%s_%s%s%s%s%s%s", szNmae, pIspTuneTaskInfo->mWhiteBalance.illumination, szDimensions, szTypeLayout, szDateTime, szNumber, szFileExt);
    else
      snprintf(szFileName, FILENAME_MAX, "%s%s%s%s%s%s", szNmae, szDimensions, szTypeLayout, szDateTime, szNumber, szFileExt);
    szFileName[FILENAME_MAX - 1] = '\0';

    // then open file
    ALOGD("Dump file name: %s", szFileName);
    pStoreFile = fopen(szFileName, "wb");
    if (pStoreFile == NULL) {
      TRACE_E("%s Couldn't open file '%s'.\n", __FUNCTION__, szFileName);
      return -1;
    }

  }

  // depending on data format, layout & size call subroutines to:
  // ...get data into local buffer(s)
  // ...write local buffer(s) to file while removing any trailing stuffing from lines where applicable
  // ...averaging pixel data is handled by @ref somCtrlStoreBufferRAW() function internally
  if (strcmp(pBuffer->getFormat(), RK_HAL_FMT_STRING::HAL_FMT_STRING_NV16) == 0) {
    result = ispTuneStoreBufferYUV422Semi(pIspTuneTaskInfo, pStoreFile, pBuffer, isNewFile);
  } else if ((strcmp(pBuffer->getFormat(), RK_HAL_FMT_STRING::HAL_FMT_STRING_SBGGR8) == 0)
             || (strcmp(pBuffer->getFormat(), RK_HAL_FMT_STRING::HAL_FMT_STRING_SGBRG8) == 0)
             || (strcmp(pBuffer->getFormat(), RK_HAL_FMT_STRING::HAL_FMT_STRING_SGRBG8) == 0)
             || (strcmp(pBuffer->getFormat(), RK_HAL_FMT_STRING::HAL_FMT_STRING_SRGGB8) == 0)
            ) {
    result = ispTuneStoreBufferRAW(pIspTuneTaskInfo, pStoreFile, pBuffer, isNewFile, false);

  } else if ((strcmp(pBuffer->getFormat(), RK_HAL_FMT_STRING::HAL_FMT_STRING_SBGGR10) == 0)
             || (strcmp(pBuffer->getFormat(), RK_HAL_FMT_STRING::HAL_FMT_STRING_SGBRG10) == 0)
             || (strcmp(pBuffer->getFormat(), RK_HAL_FMT_STRING::HAL_FMT_STRING_SGRBG10) == 0)
             || (strcmp(pBuffer->getFormat(), RK_HAL_FMT_STRING::HAL_FMT_STRING_SRGGB10) == 0)
             || (strcmp(pBuffer->getFormat(), RK_HAL_FMT_STRING::HAL_FMT_STRING_SBGGR12) == 0)
             || (strcmp(pBuffer->getFormat(), RK_HAL_FMT_STRING::HAL_FMT_STRING_SGBRG12) == 0)
             || (strcmp(pBuffer->getFormat(), RK_HAL_FMT_STRING::HAL_FMT_STRING_SGRBG12) == 0)
             || (strcmp(pBuffer->getFormat(), RK_HAL_FMT_STRING::HAL_FMT_STRING_SRGGB12) == 0)
            ) {
    result = ispTuneStoreBufferRAW(pIspTuneTaskInfo, pStoreFile, pBuffer, isNewFile, true);

  } else {
    result = -1;
  }

  if (pStoreFile) {
    // close file
    fclose(pStoreFile);
  }
  if (result != 0) {
    return result;
  }

  TRACE_D(1, "%s (exit)\n", __FUNCTION__);
  return result;
}

int  CameraIspTunning::ispTuneDesiredExp(long raw_ddr, int width, int height, int min_raw, int max_raw, int threshold) {
  int max_raw_num = 0, min_raw_num = 0;
  int num, value, result = 0;
  int proportion;
  short unsigned int* p = NULL;
  p = (short unsigned int*)raw_ddr;
  //TRACE_D(0, "%s @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@(enter)\n", __FUNCTION__);

  min_raw <<= 8;
  max_raw <<= 8;

  for (num = 0; num < width * height; num++) {
    value = *p++;
    if (value > max_raw)
      max_raw_num++;
    if (value < min_raw)
      min_raw_num++;
  }

  //TRACE_D(0, "min_raw = %d \n", min_raw);
  //TRACE_D(0, "max_raw = %d \n", max_raw);
  //TRACE_D(0, "threshold = %d \n", threshold);
  //TRACE_D(0, "width*height = %d \n", width*height);

  //TRACE_D(0, "min_raw_num = %.1f \n", min_raw_num);
  //TRACE_D(0, "max_raw_num = %.1f \n", max_raw_num);

  proportion = min_raw_num * 100 / (width * height);
  if (proportion > threshold) {
    result |= 0x1;
  }
  TRACE_D(0, "min_raw %d !!!!!!!!!!!\n", proportion);
  proportion = max_raw_num * 100 / (width * height);
  if (proportion > threshold) {
    result |= 0x2;
    //TRACE_D(0, "%.5f ~~~~~~~~~~~\n", proportion);

  }
  TRACE_D(0, "max_raw %d ~~~~~~~~~~~\n", proportion);

  //TRACE_D(0, "%s @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@(exit)\n", __FUNCTION__);
  return result;
}
