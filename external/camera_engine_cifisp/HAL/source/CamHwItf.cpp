
using namespace std;

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/v4l2-subdev.h>
#include "V4L2DevIoctr.h"
#include "CamHwItf.h"
#include "cam_ia_api/cam_ia10_trace.h"
#ifdef RK_ISP10
#include "CamIsp10DevHwItf.h"
#endif
#ifdef RK_ISP11
#include "CamIsp11DevHwItf.h"
#endif
#include "CamIqDebug.h"
#include <sys/time.h>
#include <common/cam_types.h>

#ifdef RK_ISP10
CamIsp10DevHwItf* mCamIsp10DevHwItf;
#endif

#ifdef RK_ISP11
CamIsp11DevHwItf* mCamIsp11DevHwItf;
#endif

struct HAL_ISP_Reboot_Req_s CamHwItf::mApiReboot = { 0 };
CamHwItf::PathBase::PathBase(CamHwItf* camHw, V4L2DevIoctr* camDev, PATHID pathID, unsigned long dequeueTimeout)
  :
  mCamHw(camHw),
  mCamDev(camDev),
  mPathID(pathID),
  mDequeueTimeout(dequeueTimeout),
  mState(UNINITIALIZED),
  mMinNumBuffersQueued(1),
  mNumBuffersUndequeueable(0),
  mBufferAllocator(NULL),
  mSkipFrames(0),
  mPathRefCnt(0),
  mDequeueThread(new DequeueThread(this)) {
  //ALOGD("%s: E", __func__);
  osMutexInit(&mNumBuffersQueuedLock);
  osMutexInit(&mPathLock);
  osMutexInit(&mBufLock);
  osMutexInit(&mNotifierLock);
  osEventInit(&mBufferQueued, true, 0);
  mDumpNum = 0;
  mDumpCnt = 0;
  //ALOGD("%s: X", __func__);
}
CamHwItf::PathBase::~PathBase(void) {
  //ALOGD("%s: E", __func__);
  osMutexDestroy(&mNumBuffersQueuedLock);
  osMutexDestroy(&mPathLock);
  osMutexDestroy(&mBufLock);
  osMutexDestroy(&mNotifierLock);
  osEventDestroy(&mBufferQueued);
  //ALOGD("%s: X", __func__);
}

bool CamHwItf::PathBase::preDumpName(shared_ptr<BufferBase>& pBuffer, struct rk_dump_ctl &dump_ctl, uint32_t idx) {
  uint32_t width  = 0;
  uint32_t height = 0;
  float curtime;
  float curgain;
  FILE* dump_file;
  struct HAL_Buffer_MetaData* metaData;
  char szName[100] = {0};

  width = pBuffer->getWidth();
  height = pBuffer->getHeight();
  metaData = pBuffer->getMetaData();
  if (metaData) {
    ALOGD("expousre gain : %f %f",
          metaData->exp_time, metaData->exp_gain);
    curtime = metaData->exp_time;
    curgain = metaData->exp_gain;

    snprintf(szName, sizeof(szName) - 1, "%st%0.3f_g%0.3f",
             dump_ctl.dump_path, curtime, curgain);
    szName[99] = '\0';

    const char* szFileExt = ".bin";
    const char* szTypeLayout = pBuffer->getFormat();
    bool isData = false;
    bool isJpeg = false;

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

    // colorSpace
    char szColorSpace[10] = "";
    if (mFmtInfo.colorSpace == HAL_COLORSPACE_SMPTE170M) {
      snprintf(szColorSpace, sizeof(szColorSpace) - 1, "_limit");
    }
    else {
      snprintf(szColorSpace, sizeof(szColorSpace) - 1, "_full");
    }

    // ...create sequence number string
    char szNumber[20] = "";
    snprintf(szNumber, sizeof(szNumber) - 1, "_%04d", idx);

    // ...combine all parts
    uint32_t combLen;
    combLen = strlen(szName) + strlen(szDimensions) + strlen(szTypeLayout)+1 + strlen(szDateTime) + strlen(szNumber) + strlen(szFileExt);

    if (combLen >= FILENAME_MAX) {
      ALOGE("%s Max filename length exceeded.\n"
              " len(BaseFileName) = %3d\n"
              " len(Dimensions)   = %3d\n"
              " len(szTypeLayout) = %3d\n"
              " len(szDateTime)   = %3d\n"
              " len(szColorSpace)     = %3d\n"
              " len(szNumber) = %3d\n"
              " len(FileExt)      = %3d\n"
              " --------------------------\n"
              " combLen        >= %3d\n",
              __FUNCTION__,
              strlen(szName), strlen(szDimensions), strlen(szTypeLayout)+1,
              strlen(szDateTime), strlen(szColorSpace), strlen(szNumber), strlen(szFileExt), combLen);
      return false;
    }

    snprintf(mDumpFileName, FILENAME_MAX, "%s%s_%s%s%s%s%s", szName, szDimensions, szTypeLayout, szDateTime, szColorSpace, szNumber, szFileExt);
    mDumpFileName[FILENAME_MAX - 1] = '\0';
  }

  return true;
}

bool CamHwItf::PathBase::dumpFrame(shared_ptr<BufferBase>& pBuffer, char *szFileName) {
  bool ret = true;
  FILE* dump_file;
  struct HAL_Buffer_MetaData* metaData;
  struct timeval tv;

  gettimeofday(&tv,NULL);
  ALOGD("start time:0x%x\n",tv.tv_sec*1000 + tv.tv_usec/1000);

  //write to file
  metaData = pBuffer->getMetaData();
  dump_file = fopen(szFileName, "a+");
  if (dump_file) {
    fwrite(pBuffer->getVirtAddr(), pBuffer->getDataSize(), 1, dump_file);
    fwrite(metaData, sizeof(struct HAL_Buffer_MetaData), 1, dump_file);
    ALOGD("write 0x%x bytes to file!", pBuffer->getDataSize()+sizeof(struct HAL_Buffer_MetaData));
    fclose(dump_file);
  } else {
    ret = false;
    ALOGE("open file %s error", szFileName);
  }

  gettimeofday(&tv,NULL);
  ALOGD("end time:0x%x\n\n",tv.tv_sec*1000 + tv.tv_usec/1000);

  return ret;
}

bool CamHwItf::PathBase::dequeueFunc(void) {
  int err;
  bool ret = true;
  shared_ptr<BufferBase> buffer;
  struct rk_dump_ctl dump_ctl;
  CameraIqDebug* pIqDebugInst;
  bool dump_ret;

  //ALOGV("%s: line %d", __func__, __LINE__);
  //mPathLock.lock();
  pIqDebugInst = CameraIqDebug::getInstance();
  osMutexLock(&mNumBuffersQueuedLock);
  if (mState == STREAMING) {
    //mPathLock.unlock();

    if (mNumBuffersQueued > mNumBuffersUndequeueable) {
      osMutexUnlock(&mNumBuffersQueuedLock);
      err = mCamDev->dequeueBuffer(buffer, mDequeueTimeout);
    } else {
      osMutexUnlock(&mNumBuffersQueuedLock);
      osEventWait(&mBufferQueued);
      return true;
    }

    if (err < 0) {
      //notify listenners
      osMutexLock(&mNotifierLock);
      weak_ptr<BufferBase> wpBuffer;
      for (list<NewCameraBufferReadyNotifier* >::iterator i = mBufferReadyNotifierList.begin(); i != mBufferReadyNotifierList.end(); i++)
        ret = (*i)->bufferReady(wpBuffer, err);
      osMutexUnlock(&mNotifierLock);
      if (err != -ETIMEDOUT) {
        ALOGE("%s: %d dequeue buffer failed, exiting thread loop", __func__, mPathID);
        ret = false;
      } else {
        ALOGW("%s: %d dequeue timeout (%ldms)", __func__, mPathID, mDequeueTimeout);
      }
    } else {
      if ((mSkipFrames > 0) || (mNumBuffersQueued <= mMinNumBuffersQueued)) {
        if (mSkipFrames > 0)
          mSkipFrames--;
        mCamDev->queueBuffer(buffer);
        return true;
      }
      //get metadata
      struct HAL_Buffer_MetaData metaData;
      if (mCamDev->getBufferMetaData(buffer->getIndex(), &metaData)) {
        mCamHw->transDrvMetaDataToHal(metaData.metedata_drv, &metaData);
        buffer->setMetaData(&metaData);
        buffer->setTimestamp(&(metaData.timStamp));
      }
      osMutexLock(&mNumBuffersQueuedLock);
      mNumBuffersQueued--;
      osMutexUnlock(&mNumBuffersQueuedLock);
      if (mDumpNum == 0) {
        pIqDebugInst->getDumpCtl(dump_ctl, mPathID);
        if (dump_ctl.dump_enable == true) {
          //prepare dump file name
          dump_ret = preDumpName(buffer, dump_ctl, 0);
          if (dump_ret == true) {
            mDumpNum = dump_ctl.dump_num;
            mDumpCnt = 0;
            ALOGD("Start dump file, DumpNum=%d", mDumpNum);
          }
        }
      }

      if (mDumpCnt < mDumpNum) {
        mDumpBufList.push_back(buffer);
        mDumpCnt++;

        if (mDumpCnt < mDumpNum) buffer->incUsedCnt();
        ALOGD("Save dump buffer to list, cnt=%d, DumpNum=%d, index=%d", mDumpCnt, mDumpNum, buffer->getIndex());
      }
      if (mDumpCnt >= mDumpNum) {
        if (mDumpNum > 0) {
          int index = 1;
          for (list<shared_ptr<BufferBase> >::iterator i = mDumpBufList.begin(); i != mDumpBufList.end(); i++) {
            //write frame data to file
            ALOGD("Start write frame data to file, cnt=%d, DumpNum=%d, index=%d", index, mDumpNum, (*i)->getIndex());
            //preDumpName(*i, dump_ctl, index);
            dump_ret = dumpFrame(*i, mDumpFileName);
            ALOGD("End write frame data to file, cnt=%d, DumpNum=%d, index=%d", index, mDumpNum, (*i)->getIndex());

            index++;
            if ((*i) != buffer) (*i)->decUsedCnt();
          }

          mDumpNum = 0;
          mDumpCnt = 0;
          mDumpBufList.clear();
        }
      }

      //increase reference before notify
      osMutexLock(&mNotifierLock);
      buffer->incUsedCnt();
      for (list<NewCameraBufferReadyNotifier* >::iterator i = mBufferReadyNotifierList.begin(); i != mBufferReadyNotifierList.end(); i++) {
        ret = (*i)->bufferReady(buffer, 0);
      }
      osMutexUnlock(&mNotifierLock);
      buffer->decUsedCnt();
    }

  } else {
    osMutexUnlock(&mNumBuffersQueuedLock);
    ALOGD("%s: %d stopped STREAMING", __func__, mPathID);
    ret = false;
  }

  if (!ret) {
    ALOGD("%s: %d exiting Thread loop", __func__, mPathID);
  }
  return ret;
}


bool CamHwItf::PathBase::setInput(Input inp) const {
  if (mCamDev->setInput(inp))
    return false;
  return true;
}

unsigned int CamHwItf::PathBase::getMinNumUndequeueableBuffers(void) const {
  return mMinNumBuffersQueued;
}
unsigned int CamHwItf::PathBase::getNumQueuedBuffers(void) const {
  unsigned int numBuf;
  osMutexLock(&mNumBuffersQueuedLock);
  numBuf = mNumBuffersQueued;
  osMutexUnlock(&mNumBuffersQueuedLock);
  return numBuf;
}

bool CamHwItf::PathBase::getFmtInfo(frm_info_t *frm_info) const {
  *frm_info = mFmtInfo;
  return true;
}

CamHwItf::CamHwItf(void): m_flag_init(false) {
  //ALOGD("%s: E", __func__);
  //ALOGD("%s: X", __func__);
}

int CamHwItf::setFocusPos(int position) {
  int ret = mSpDev->setCtrl(V4L2_CID_FOCUS_ABSOLUTE, position);

  if (ret < 0) {
    LOGE("Could not set focus, error %d", ret);
  }
  mLastLensPosition = position;
  return ret;
}

int CamHwItf::setExposure(unsigned int vts, unsigned int exposure, unsigned int gain, unsigned int gain_percent) {
  int ret;
  struct v4l2_ext_control exp_gain[4];
  struct v4l2_ext_controls ctrls;

  exp_gain[0].id = V4L2_CID_EXPOSURE;

  exp_gain[0].value = exposure;
  exp_gain[1].id = V4L2_CID_GAIN;
  exp_gain[1].value = gain;
  exp_gain[2].id = RK_V4L2_CID_GAIN_PERCENT;
  exp_gain[2].value = gain_percent;
  exp_gain[3].id = RK_V4L2_CID_VTS;
  exp_gain[3].value = vts;

  ctrls.count = 4;
  ctrls.ctrl_class = V4L2_CTRL_CLASS_USER;
  ctrls.controls = exp_gain;
  ctrls.reserved[0] = 0;
  ctrls.reserved[1] = 0;

  ret = ioctl(m_cam_fd_overlay, VIDIOC_S_EXT_CTRLS, &ctrls);

  if (ret < 0) {
    LOGE("ERR(%s):set of  AE seting to sensor config failed! err: %s\n",
         __func__,
         strerror(errno));
    return ret;
  } else {
    mExposureSequence = exp_gain[0].value;
    TRACE_D(1, "%s(%d): mExposureSequence: %d", __FUNCTION__, __LINE__, mExposureSequence);
  }

  return ret;
}

//ircut
//-1 means unsupported
int CamHwItf::isSupportedIrCut() {
  return mSpDev->getCtrl(V4L2_CID_BAND_STOP_FILTER);
}
//>0 means IRCUT is working
int CamHwItf::getIrCutState() {
  return mSpDev->getCtrl(V4L2_CID_BAND_STOP_FILTER);
}

int CamHwItf::setIrCutState(int state) {
  return mSpDev->setCtrl(V4L2_CID_BAND_STOP_FILTER, state);
}

int CamHwItf::tryFormat(frm_info_t inFmt, frm_info_t& outFmt) {
  unsigned int v4l2PixFmt = mSpDev->halFmtToV4l2Fmt(inFmt.frmFmt);
  unsigned int width = inFmt.frmSize.width;
  unsigned int height = inFmt.frmSize.height;
  int ret = 0;
  if (0 > (ret = mSpDev->tryFormat(v4l2PixFmt, width, height)))
    ALOGE("%s:%d failed ,error is %s.", __func__, __LINE__, strerror(errno));
  else {
    outFmt.frmFmt = mSpDev->V4l2FmtToHalFmt(v4l2PixFmt);
    outFmt.frmSize.width = width;
    outFmt.frmSize.height = height;
    //TODO :try fps ?
    outFmt.fps = inFmt.fps;
  }
  return ret;
}

int CamHwItf::setAutoAdjustFps(bool auto_adjust_fps){
  return 0;
}

int CamHwItf::setFlip(int flip) {
  int ret = 0;
  int setFlip = flip & (HAL_FLIP_H | HAL_FLIP_V);
  int needSetFlip = setFlip;

  if (needSetFlip & HAL_FLIP_H)
    ret = mSpDev->setCtrl(V4L2_CID_HFLIP, 1);
  else
    ret = mSpDev->setCtrl(V4L2_CID_HFLIP, 0);

  if (needSetFlip & HAL_FLIP_V)
    ret = mSpDev->setCtrl(V4L2_CID_VFLIP, 1);
  else
    ret = mSpDev->setCtrl(V4L2_CID_VFLIP, 0);

  if (ret == 0)
    mFlip = setFlip;
  else {
    LOGW("Could not set flip, error %d", ret);
  }

  return ret;
}

int CamHwItf::getFlip(int& flip) {
  int ret = 0;
  int hFlip = 0;
  int vFlip = 0;

  ret = mSpDev->getCtrl(V4L2_CID_HFLIP);
  if (ret < 0)
    ALOGE("Could not get hflip, error %d", ret);
  else if (ret)
    hFlip = HAL_FLIP_H;

  ret = mSpDev->getCtrl(V4L2_CID_VFLIP);
  if (ret < 0)
    ALOGE("Could not get vflip, error %d", ret);
  else if (ret)
    vFlip = HAL_FLIP_V;

  flip = hFlip | vFlip;

  return 0;
}

int CamHwItf::getSysInfo(struct HAL_ISP_Sys_Info_s& info) {
  struct camera_module_info_s camera_module;
  struct sensor_config_info_s sensor_config;
  struct isp_supplemental_sensor_mode_data sensor_mode_data;
  float max_exp_time;

  if (mSpDev->getCameraModuleInfo(&camera_module) != 0) {
    ALOGE("%s: get camera module info error!", __func__);
    return -1;
  }

  if (mSpDev->getSensorModeData(&sensor_mode_data) != 0) {
    ALOGE("%s: get sensor mode data error!", __func__);
    return -1;
  }

  if (mSpDev->getSensorConfigInfo(&sensor_config) != 0) {
    ALOGE("%s: get sensor config info error!", __func__);
    return -1;
  }

  memset(&info, 0, sizeof(info));
  info.major_ver = 0x01;
  info.minor_ver = 0x01;
  strcpy((char *)info.platform, "rv1108");
  strcpy((char *)info.sensor, camera_module.sensor_name);
  strcpy((char *)info.module, camera_module.module_name);
  strcpy((char *)info.lens, camera_module.len_name);
  info.otp_info.awb_otp = 0;
  info.otp_info.lsc_otp = 0;

  max_exp_time = sensor_mode_data.frame_length_lines *
    sensor_mode_data.line_length_pck * 1000 /
    sensor_mode_data.vt_pix_clk_freq_hz;
  info.max_exp_time_h = max_exp_time;
  info.max_exp_time_l = (max_exp_time - info.max_exp_time_h) * 256.0;
  info.max_exp_gain_h = sensor_mode_data.max_exp_gain_h;
  info.max_exp_gain_l = sensor_mode_data.max_exp_gain_l;

  info.reso_num = sensor_config.config_num;
  info.sensor_fmt = sensor_config.sensor_fmt[0];
  for (int i = 0; i < HAL_ISP_SENSOR_RESOLUTION_NUM; i++) {
    info.reso[i].width = sensor_config.reso[i].width;
    info.reso[i].height = sensor_config.reso[i].height;
  }
}

int CamHwItf::setSensorExp(struct HAL_ISP_Sensor_Exposure_s& exp) {
  float hal_gain;
  float hal_time;

  if (exp.ae_mode == HAL_ISP_AE_MANUAL) {
    hal_gain = exp.exp_gain_h + exp.exp_gain_l / 256.0;
    hal_time = (exp.exp_time_h + exp.exp_time_l / 256.0) / 1000.0;
    setManualGainAndTime(hal_gain, hal_time);
    setAeMode(HAL_AE_OPERATION_MODE_MANUAL);
  } else {
    setAeMode(HAL_AE_OPERATION_MODE_AUTO);
  }
}

int CamHwItf::setSensorMirror(struct HAL_ISP_Sensor_Mirror_s& mirror) {
  UNUSED_PARAM(mirror);
}

int CamHwItf::getSensorInfo(struct HAL_ISP_Sensor_Info_s& info) {
  int sensorGain;
  int sensorInttime;
  float halGain;
  float halInttime;
  struct isp_supplemental_sensor_mode_data sensor_mode_data;

  if (mSpDev->getSensorModeData(&sensor_mode_data) != 0) {
    ALOGE("%s: get sensor mode data error!", __func__);
    return -1;
  }

  sensorGain = sensor_mode_data.gain;
  sensorInttime = sensor_mode_data.exp_time;
  mIspDev->mapSensorExpToHal(sensorGain, sensorInttime, halGain, halInttime);
  info.exp_time_h = halInttime * 1000;
  info.exp_time_l = (halInttime * 1000 - info.exp_time_h) * 256;
  info.exp_gain_h = halGain;
  info.exp_gain_l = (halGain - info.exp_gain_h) * 256;

  info.frame_length_lines = sensor_mode_data.frame_length_lines;
  info.line_length_pck = sensor_mode_data.line_length_pck;
  info.vt_pix_clk_freq_hz = sensor_mode_data.vt_pix_clk_freq_hz;
  if (sensor_mode_data.binning_factor_x != 1 ||
      sensor_mode_data.binning_factor_y != 1)
    info.binning = 1;
  else
    info.binning = 0;
}

int CamHwItf::setSensorReg(struct HAL_ISP_Sensor_Reg_s& reg) {
  struct sensor_reg_rw_s sensor_rw;

  if ((reg.reg_addr_len != 1 && reg.reg_addr_len != 2) ||
    (reg.reg_data_len != 1 && reg.reg_data_len != 2)) {
    ALOGE("%s: parameter error, reg_addr_len %d, reg_data_len %d.", __func__, reg.reg_addr_len, reg.reg_data_len);
    return -1;
  }

  sensor_rw.reg_access_mode = 1;
  sensor_rw.addr = reg.reg_addr;
  sensor_rw.data = reg.reg_data;
  sensor_rw.reg_addr_len = reg.reg_addr_len - 1;
  sensor_rw.reg_data_len = reg.reg_data_len - 1;
  if (mSpDev->accessSensor(&sensor_rw)) {
    ALOGE("%s: access sensor register error!", __func__);
    return -1;
  }
}

int CamHwItf::getSensorReg(struct HAL_ISP_Sensor_Reg_s& reg) {
  struct sensor_reg_rw_s sensor_rw;

  sensor_rw.reg_access_mode = 0;
  sensor_rw.addr = reg.reg_addr;
  sensor_rw.reg_addr_len = reg.reg_addr_len;
  sensor_rw.reg_data_len = reg.reg_data_len;
  if (mSpDev->accessSensor(&sensor_rw)) {
    ALOGE("%s: access sensor register error!", __func__);
    return -1;
  }

  reg.reg_data = sensor_rw.data;
  reg.reg_addr_len = sensor_rw.reg_addr_len + 1;
  reg.reg_data_len = sensor_rw.reg_data_len + 1;
}

int CamHwItf::startCap(struct HAL_ISP_Cap_Req_s& req) {
  return 0;
}

int CamHwItf::getCapRes(struct HAL_ISP_Cap_Result_s& result) {
  return 0;
}

int CamHwItf::reInitHW(struct HAL_ISP_Reboot_Req_s& req) {
  return 0;
}

void CamHwItf::transDrvMetaDataToHal
(
    const void* drvMeta,
    struct HAL_Buffer_MetaData* halMeta
) {
  return;
}

int CamHwItf::getCameraInfos(struct rk_cams_dev_info* cam_infos) {
  int ret = 0, i = 0, j = 0, fd;
  char video_dev_path[15];
  struct v4l2_capability capability;
  v4l2_input input;
  for (i = 0; i < RK_ENUM_VIDEO_NUM_MAX; i++) {
    int video_node_num;
    int video_node_input_num;
    struct rk_cam_video_node* node;
    video_dev_path[0] = 0;
    snprintf(video_dev_path, sizeof(video_dev_path), "/dev/video%d", i);
    fd = open(video_dev_path, O_RDONLY);
    if (fd < 0) {
      //LOGE("Open %s failed! strr: %s",video_dev_path,strerror(errno));
      continue;
    }

    TRACE_D(1, "open %s", video_dev_path);
    memset(&capability, 0, sizeof(struct v4l2_capability));
    if (ioctl(fd, VIDIOC_QUERYCAP, &capability) < 0) {
      LOGE("Video device(%s): query capability not supported.\n", video_dev_path);
      close(fd);
      continue;
    }
    TRACE_D(1, "driver %s ", capability.driver);
    if (strstr((char*)(capability.driver), "rkisp")) {
      struct rk_isp_dev_info* isp_dev = &cam_infos->isp_dev;
      video_node_num = isp_dev->isp_dev_node_nums;
      //isp dev
      if ((strstr((char*)(capability.card), "self"))
          || (strstr((char*)(capability.card), "main"))
          || (strstr((char*)(capability.card), "ispdev"))
          || (strstr((char*)(capability.card), "y12"))
          || strstr((char*)(capability.card), "dma")) {
        node = &(isp_dev->video_nodes[video_node_num]);
        node->video_index = i;
        strncpy(node->card, (char*)(capability.card), strlen((char*)(capability.card)));
        //ALOGD("CARD %s",node->card);
        for (j = 0; j < RK_CAM_ATTRACED_INUPUT_MAX; j++) {
          video_node_input_num = node->input_nums;
          input.index = j;
          if (ioctl(fd, VIDIOC_ENUMINPUT, &input) == 0) {
            node->input[video_node_input_num].index = j;
            node->input[video_node_input_num].dev = node;
            node->input[video_node_input_num].type = RK_CAM_ATTACHED_TO_ISP;
            if (strstr((char*)(input.name), "DMA"))
              strncpy(node->input[video_node_input_num].name,
                      (char*)(input.name), strlen((char*)(input.name)));
            else {
              // sensor name
              strcpy(node->input[video_node_input_num].name,
                     strtok((char*)(input.name), " "));
              //only involve once
              if (strstr((char*)(capability.card), "self")) {
                cam_infos->cam[cam_infos->num_camers] =
                    &(node->input[video_node_input_num]);
                cam_infos->num_camers++;
              }
            }
            TRACE_D(1, "input name %s,id %d", node->input[video_node_input_num].name, j);
          } else
            break;
          node->input_nums++;
        }
        isp_dev->isp_dev_node_nums++;
      }
    } else if (strstr((char*)(capability.driver), "cif")) {
      //cif dev
      struct rk_cif_dev_infos* cifs = &cam_infos->cif_devs;
      video_node_num = cifs->cif_dev_node_nums;
      struct rk_cam_video_node* node = &(cifs->cif_devs[video_node_num].video_node);
      cifs->cif_devs[video_node_num].cif_index = video_node_num;
      node->video_index = i;
      strncpy(node->card, (char*)(capability.card), strlen((char*)(capability.card)));
      TRACE_D(1, "CARD %s", node->card);
      for (j = 0; j < RK_CAM_ATTRACED_INUPUT_MAX; j++) {
        video_node_input_num = node->input_nums;
        input.index = j;
        if (ioctl(fd, VIDIOC_ENUMINPUT, &input) == 0) {
          node->input[video_node_input_num].index = j;
          node->input[video_node_input_num].dev = &(cifs->cif_devs[video_node_num]);
          node->input[video_node_input_num].type = RK_CAM_ATTACHED_TO_CIF;
          strncpy(node->input[video_node_input_num].name,
                  (char*)(input.name), strlen((char*)(input.name)));
          cam_infos->cam[cam_infos->num_camers] =
              &(node->input[video_node_input_num]);
          cam_infos->num_camers++;
          TRACE_D(1, "input name %s,id %d", node->input[video_node_input_num].name, j);
        } else
          break;
        node->input_nums++;
      }
      cifs->cif_dev_node_nums++;
    } else if (strstr((char*)(capability.driver), "UVC")) {
      //usb dev
      struct rk_usb_cam_dev_infos* usb_cams = &cam_infos->usb_devs;
      video_node_num = usb_cams->usb_dev_node_nums;
      struct rk_cam_video_node* node = &(usb_cams->video_nodes[video_node_num]);
      node->video_index = i;
      strncpy(node->card, (char*)(capability.card), strlen((char*)(capability.card)));
      for (j = 0; j < RK_CAM_ATTRACED_INUPUT_MAX; j++) {
        video_node_input_num = node->input_nums;
        input.index = j;
        if (ioctl(fd, VIDIOC_ENUMINPUT, &input) == 0) {
          node->input[video_node_input_num].index = j;
          node->input[video_node_input_num].dev = node;
          node->input[video_node_input_num].type = RK_CAM_ATTACHED_TO_USB;
          strncpy(node->input[video_node_input_num].name,
                  (char*)(input.name), strlen((char*)(input.name)));
          cam_infos->cam[cam_infos->num_camers] =
              &(node->input[video_node_input_num]);
          cam_infos->num_camers++;
        } else
          break;
        node->input_nums++;
      }
      usb_cams->usb_dev_node_nums++;
    } else {
      close(fd);
      continue;
    }

    close(fd);
  }
  TRACE_D(1, "connected  camera nums %d,", cam_infos->num_camers);
  for (i = 0; i < cam_infos->num_camers; i++)
    TRACE_D(1, "cam name %s,connect to controller type %d,input index %d",
            cam_infos->cam[i]->name, cam_infos->cam[i]->type, cam_infos->cam[i]->index);
  return ret;
}


shared_ptr<CamHwItf> getCamHwItf(struct rk_isp_dev_info* isp_dev_info) {
  shared_ptr<CamHwItf> instance;
#ifdef RK_ISP10
  instance = shared_ptr<CamHwItf>(new CamIsp10DevHwItf(isp_dev_info));
  mCamIsp10DevHwItf = static_cast<CamIsp10DevHwItf*>(instance.get());
  return instance;
#endif

#ifdef RK_ISP11
  instance = shared_ptr<CamHwItf>(new CamIsp11DevHwItf(isp_dev_info));
  mCamIsp11DevHwItf = static_cast<CamIsp11DevHwItf*>(instance.get());
  return instance;
#endif
}

