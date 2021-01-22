#include <fcntl.h>
#include <sys/ioctl.h>
#include <iostream>
#include <linux/v4l2-subdev.h>
#include "V4L2DevIoctr.h"
#include "CamUSBDevHwItfImc.h"
#include "common/return_codes.h"
#include "cam_ia_api/cam_ia10_trace.h"


using namespace std;

CamUSBDevHwItf::Path::Path(V4L2DevIoctr* camDev, PATHID pathID, unsigned long dequeueTimeout):
  PathBase(NULL, camDev, pathID, dequeueTimeout),
  mUSBBufAllocator(new ProxyCameraBufferAllocator()) {
  ALOGD("%s: ", __func__);
}

CamUSBDevHwItf::Path::~Path() {
  ALOGD("%s: ", __func__);
}


bool CamUSBDevHwItf::Path::prepare(
    frm_info_t&  frmFmt,
    unsigned int numBuffers,
    CameraBufferAllocator& bufferAllocator,
    bool cached,
    unsigned int minNumBuffersQueued) {
  UNUSED_PARAM(bufferAllocator);
  shared_ptr<BufferBase> buffer;
  unsigned int stride = 0;
  unsigned int mem_usage = 0;


  //ALOGV("%s: %s format %s %dx%d@%d/%dfps, numBuffers %d, minNumBuffersQueued %d", __func__,
  //       string(mPathID), pixFmt, width, height, fps.mNumerator, fps.mDenominator, numBuffers, minNumBuffersQueued);

  if ((mState == STREAMING) || (mState == PREPARED)) {
    //prepare called when streaming, only the same format is allowd
    if (mCamDev->getCurFmt() != V4L2DevIoctr::halFmtToV4l2Fmt(frmFmt.frmFmt)) {
      ALOGW("format is different from current,req:%d,cur:%d", V4L2DevIoctr::halFmtToV4l2Fmt(frmFmt.frmFmt)
            , mCamDev->getCurFmt());
    }
    if ((mCamDev->getCurWidth() == frmFmt.frmSize.width)
        || (mCamDev->getCurHeight() == frmFmt.frmSize.height))
      ALOGW("resolution is different from current,req:%dx%d,cur:%dx%d",
            frmFmt.frmSize.width, frmFmt.frmSize.height,
            mCamDev->getCurWidth(), mCamDev->getCurHeight());
    //just return ture
    return true;
  }
  if (mState != UNINITIALIZED) {
    ALOGE("%s: %d not in UNINITIALIZED state, cannot prepare path", __func__, mPathID);
    return false;
  }
  if (!numBuffers) {
    ALOGE("%s: %d number of buffers must be larger than 0", __func__, mPathID);
    return false;
  }

  releaseBuffers();

  frm_info_t infrmFmt = frmFmt;
  unsigned int inv4l2Fmt = V4L2DevIoctr::halFmtToV4l2Fmt(infrmFmt.frmFmt);
  unsigned int inv4l2ColorSpace =V4L2DevIoctr::halColorSpaceToV4l2ColorSpace(infrmFmt.colorSpace);
  if (0 > mCamDev->setFormat(inv4l2Fmt, inv4l2ColorSpace, infrmFmt.frmSize.width, infrmFmt.frmSize.height, 0)) {
    releaseBuffers();
    return false;
  } else {
    if (inv4l2Fmt != V4L2DevIoctr::halFmtToV4l2Fmt(frmFmt.frmFmt)) {
      ALOGE("%s:%d,required fmt dose not exist,request fmt(%s),best fmt(%s)",
            __func__, __LINE__,
            RK_HAL_FMT_STRING::hal_fmt_map_to_str(frmFmt.frmFmt),
            RK_HAL_FMT_STRING::hal_fmt_map_to_str(V4L2DevIoctr::V4l2FmtToHalFmt(inv4l2Fmt))
           );
      frmFmt.frmFmt = V4L2DevIoctr::V4l2FmtToHalFmt(inv4l2Fmt);
      frmFmt.frmSize = infrmFmt.frmSize;
      releaseBuffers();
      return false;
    } else if ((infrmFmt.frmSize.width != frmFmt.frmSize.width)
               || (infrmFmt.frmSize.height != frmFmt.frmSize.height)) {
      ALOGW("%s:%d,required fmt dose not exist,request fmt(%s@%dx%d),best fmt(%s@%dx%d)",
            __func__, __LINE__,
            RK_HAL_FMT_STRING::hal_fmt_map_to_str(frmFmt.frmFmt),
            frmFmt.frmSize.width, frmFmt.frmSize.height,
            RK_HAL_FMT_STRING::hal_fmt_map_to_str(V4L2DevIoctr::V4l2FmtToHalFmt(inv4l2Fmt)),
            infrmFmt.frmSize.width, infrmFmt.frmSize.height
           );
      frmFmt.frmFmt = V4L2DevIoctr::V4l2FmtToHalFmt(inv4l2Fmt);
      frmFmt.frmSize = infrmFmt.frmSize;
    }
  }

  mNumBuffers = numBuffers;
  //mBufferAllocator = &bufferAllocator;
  mMinNumBuffersQueued = minNumBuffersQueued;
  if ((frmFmt.frmFmt == HAL_FRMAE_FMT_JPEG) || (mPathID == DMA))
    mNumBuffersUndequeueable = 0;
  else
    mNumBuffersUndequeueable = 1;

  mem_usage = (mPathID == DMA) ? CameraBuffer::READ : CameraBuffer::WRITE;
  if (cached) {
    mem_usage |= CameraBuffer::CACHED;
  }

  mem_usage |= CameraBuffer::FULL_RANGE;
  for (unsigned int i = 0; i < mNumBuffers; i++) {
    buffer = mUSBBufAllocator->alloc(RK_HAL_FMT_STRING::hal_fmt_map_to_str(frmFmt.frmFmt)
                                     , frmFmt.frmSize.width, frmFmt.frmSize.height, mem_usage, shared_from_this());

    if (buffer.get() == NULL) {
      releaseBuffers();
      return false;
    }
    buffer->setIndex(i);
    mBufferPool.push_back(buffer);
    if (!i)
      stride = buffer->getStride();
  }

  if (0 > mCamDev->requestBuffers(mNumBuffers)) {
    releaseBuffers();
    return false;
  }

  mNumBuffersQueued = 0;
  for (list<shared_ptr<BufferBase> >::iterator i = mBufferPool.begin(); i != mBufferPool.end(); i++) {
    //mmap buffer
    if (0 > mCamDev->memMap(*i)) {
      releaseBuffers();
      return false;
    }
    if (mCamDev->queueBuffer(*i) < 0) {
      releaseBuffers();
      return false;
    }
    mNumBuffersQueued++;
  }

  //ALOGV("%s: %s is now PREPARED", __func__, string(mPathID));

  mState = PREPARED;
  return true;
}


bool CamUSBDevHwItf::Path::prepare(
    frm_info_t& frmFmt,
    list<shared_ptr<BufferBase> >& bufPool,
    unsigned int numBuffers,
    unsigned int minNumBuffersQueued) {
  UNUSED_PARAM(frmFmt);
  UNUSED_PARAM(bufPool);
  UNUSED_PARAM(numBuffers);
  UNUSED_PARAM(minNumBuffersQueued);
  return false;
}


void CamUSBDevHwItf::Path::addBufferNotifier(NewCameraBufferReadyNotifier* bufferReadyNotifier) {
  osMutexLock(&mNotifierLock);
  if (bufferReadyNotifier)
    mBufferReadyNotifierList.push_back(bufferReadyNotifier);
  osMutexUnlock(&mNotifierLock);

}
bool CamUSBDevHwItf::Path::removeBufferNotifer(NewCameraBufferReadyNotifier* bufferReadyNotifier) {
  bool ret = false;
  //search this notifier
  osMutexLock(&mNotifierLock);

  for (list<NewCameraBufferReadyNotifier*>::iterator i = mBufferReadyNotifierList.begin(); i != mBufferReadyNotifierList.end(); i++) {
    if (*i == bufferReadyNotifier) {
      mBufferReadyNotifierList.erase(i);
      ret = true;
      break;
    }
  }
  osMutexUnlock(&mNotifierLock);
  return ret;
}

bool CamUSBDevHwItf::Path::releaseBufToOwener(weak_ptr<BufferBase> camBuf) {
  int ret = true;
  //decrese reference count  before queue
  osMutexLock(&mBufLock);
  shared_ptr<BufferBase> spBuf = camBuf.lock();
 
  if ((spBuf.get() != NULL) && (mState == STREAMING)) {
    ret = mCamDev->queueBuffer(spBuf);
    if (!ret) {
      osMutexLock(&mNumBuffersQueuedLock);
      mNumBuffersQueued++;
      osMutexUnlock(&mNumBuffersQueuedLock);
      osEventSignal(&mBufferQueued);
    } else
      ret = false;
  }
  osMutexUnlock(&mBufLock);
  return ret;
}

void CamUSBDevHwItf::Path::releaseBuffers(void) {
  //ALOGV("%s: line %d", __func__, __LINE__);

  if (mState == STREAMING) {
    ALOGD("%s: path is also be using.", __func__);
    return;
  }
  osMutexLock(&mBufLock);
  //TODO:should wait all buffers are not used
  //ummap usb pool buffer
  for (list<shared_ptr<BufferBase> >::iterator i = mBufferPool.begin(); i != mBufferPool.end(); i++) {
    //mmap buffer
    if (((*i)->getVirtAddr() != NULL) && (0 > mCamDev->memUnmap(*i))) {
      ALOGE("%s: ummap usb pool buffer failed", __func__);
    }
  }
  //release buffers after unmap,count 0 means free all allocated buffers
  mCamDev->requestBuffers(0);
  mBufferPool.clear();
  osMutexUnlock(&mBufLock);
}


bool CamUSBDevHwItf::Path::start(void) {
  int ret;

  if (mState == STREAMING) {
    ALOGD("%s: %d is already in STREAMING state", __func__, mPathID);
    mPathRefCnt++;
    return true;
  } else if (mState != PREPARED) {
    ALOGE("%s: %d cannot start, path is not in PREPARED state", __func__, mPathID);
    return false;
  }
  if (mCamDev->streamOn())
    return false;
  mState = STREAMING;
  mPathRefCnt++;

  ret = mDequeueThread->run("pathTh");
  if (ret != RET_SUCCESS) {
    mState = PREPARED;
    ALOGE("%s: %d thread start failed (error %d)", __func__, mPathID, ret);
    return false;
  }
  //ALOGV("%s: %s is now STREAMING", __func__, string(mPathID));
  return true;
}

void CamUSBDevHwItf::Path::stop(void) {
  ALOGD("%s: E", __func__);
  if (mState == STREAMING) {
    if (--mPathRefCnt != 0) {
      ALOGD("path also be used, not stop! pathRef %d", mPathRefCnt);
      return;
    }
  }

  osMutexLock(&mNumBuffersQueuedLock);
  if (mState == STREAMING) {
    mState = PREPARED;
    osEventSignal(&mBufferQueued);
    osMutexUnlock(&mNumBuffersQueuedLock);
    mDequeueThread->requestExitAndWait();
  } else
    osMutexUnlock(&mNumBuffersQueuedLock);


  osMutexLock(&mNumBuffersQueuedLock);
  if (mState == PREPARED) {
    mState = UNINITIALIZED;
    osMutexUnlock(&mNumBuffersQueuedLock);
    mCamDev->streamOff();
    mNumBuffersQueued = 0;
  } else
    osMutexUnlock(&mNumBuffersQueuedLock);
  //TODO should wait all buffers are returned.
  ALOGD("%s: %d is now UNINITIALIZED", __func__, mPathID);
}

CamUSBDevHwItf::CamUSBDevHwItf(struct rk_cam_video_node* usb_video_node) {
  m_flag_init = false;
  mUSBVideoNode = usb_video_node;
}
CamUSBDevHwItf::~CamUSBDevHwItf(void) {
}

shared_ptr<CamHwItf::PathBase> CamUSBDevHwItf::getPath(enum PATHID id) {
  shared_ptr<CamHwItf::PathBase> path;

  switch (id) {
    case MP:
    case SP:
      path = mSp;
      break;
    default:
      break;
  }
  return path;
}

// inputId used as video number
bool CamUSBDevHwItf::initHw(int inputId) {
  bool ret = true;
  //open devices
  if (!m_flag_init) {
    char dev_name[15] = {0};
    if (!mUSBVideoNode)
      snprintf(dev_name, 15, "/dev/video%d", inputId);
    else
      snprintf(dev_name, 15, "/dev/video%d", mUSBVideoNode->video_index);
#ifdef CAMERAHAL_VIDEODEV_NONBLOCK
    m_cam_fd_overlay = open(dev_name, O_RDWR | O_NONBLOCK);
#else
    m_cam_fd_overlay = open(dev_name, O_RDWR);
#endif
    //LOGV("%s :m_cam_fd_overlay %d \n", __func__, m_cam_fd_overlay);
    if (m_cam_fd_overlay < 0) {
      LOGE("ERR(%s):Cannot open %s (error : %s)\n", __func__, dev_name, strerror(errno));
      return false;
    }

    mSpDev = shared_ptr<V4L2DevIoctr>(new V4L2DevIoctr(m_cam_fd_overlay, V4L2_BUF_TYPE_VIDEO_CAPTURE, V4L2_MEMORY_MMAP));
    mSp = shared_ptr<CamHwItf::PathBase>(new Path(mSpDev.get(), CamHwItf::SP));

#ifdef CAMERAHAL_VIDEODEV_NONBLOCK
    m_cam_fd_capture = m_cam_fd_overlay;
#else
    m_cam_fd_capture = m_cam_fd_overlay;
#endif
    //LOGV("%s :m_cam_fd_capture %d \n", __func__, m_cam_fd_capture);
    if (m_cam_fd_capture < 0) {
      LOGE("ERR(%s):Cannot open %s (error : %s)\n", __func__, dev_name, strerror(errno));
      printf("%s:%d\n", __func__, __LINE__);
      return false;
    }

    mMpDev = mSpDev;
    mMp = mSp;

#ifdef CAMERAHAL_VIDEODEV_NONBLOCK
    m_cam_fd_dma = -1;
#else
    m_cam_fd_dma = -1;
#endif

    mDmaPathDev.reset();
    mDMAPath.reset();

    //LOGD("calling Query capabilityn\n");
    if (mSpDev->queryCap(V4L2_CAP_VIDEO_CAPTURE) < 0)
      ret = false;
    mInputId = inputId;
    //LOGD("returned from Query capabilityn\n");
    m_flag_init = 1;
  }
  return ret;
}
void CamUSBDevHwItf::deInitHw() {
  //LOGV("%s :", __func__);

  if (m_flag_init) {

    if (m_cam_fd_capture > -1) {
      mMp.reset();
      mMpDev.reset();
      //mp and sp have the same fd,
      //just close once
      //close(m_cam_fd_capture);
      m_cam_fd_capture = -1;
    }

    if (m_cam_fd_overlay > -1) {
      mSp.reset();
      mSpDev.reset();
      close(m_cam_fd_overlay);
      m_cam_fd_overlay = -1;
    }

    if (m_cam_fd_dma > -1) {
      mDmaPathDev.reset();
      mDMAPath.reset();
      close(m_cam_fd_dma);
      m_cam_fd_dma = -1;
    }
    m_flag_init = 0;
  }
}

//override CamHwItf::setWhiteBalance
int CamUSBDevHwItf::setWhiteBalance(HAL_WB_MODE wbMode) {
  //LOGV("%s set white balance mode:%d ", __func__, wbMode);
  int ret = 0;
  int temprature;
  if (m_wb_mode == wbMode)
    return 0;

  if (wbMode == HAL_WB_AUTO) {
    ret = mSpDev->setCtrl(V4L2_CID_AUTO_WHITE_BALANCE, 1);
    return ret;
  } else
    ret = mSpDev->setCtrl(V4L2_CID_AUTO_WHITE_BALANCE, 0);

  if (ret) {
    LOGW("Could not set WB, error %d", ret);
    return ret;
  }

  switch (wbMode) {
    case HAL_WB_CLOUDY_DAYLIGHT:
      temprature = 6500;
      break;
    case HAL_WB_DAYLIGHT:
      temprature = 5500;
      break;
    case HAL_WB_FLUORESCENT:
      temprature = 4000;
      break;
    case HAL_WB_INCANDESCENT:
      temprature = 2800;
      break;
    case HAL_WB_AUTO:
    default:
      return -1;
  }

  ret = mSpDev->setCtrl(V4L2_CID_WHITE_BALANCE_TEMPERATURE, temprature);

  if (ret == 0)
    m_wb_mode = wbMode;
  else {
    LOGW("Could not set WB, error %d", ret);
  }

  return ret;
}

int CamUSBDevHwItf::setAeBias(int aeBias) {
  struct v4l2_queryctrl queryctrl;

  memset(&queryctrl, 0, sizeof(queryctrl));
  queryctrl.id = V4L2_CID_BRIGHTNESS;

  if ((-1 == mSpDev->queryCtrl(&queryctrl)) ||
      (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED))
    return -1;
  else {
    int brghtness = (aeBias+300) * (queryctrl.maximum-queryctrl.minimum) / 600 +
                    queryctrl.minimum;
    brghtness = (brghtness > queryctrl.maximum) ? queryctrl.maximum :
                (brghtness < queryctrl.minimum) ? queryctrl.minimum : brghtness;
    if (-1 == mSpDev->setCtrl(V4L2_CID_BRIGHTNESS, brghtness)) {
      return -1;
    }
  }

  mCurAeBias = aeBias;
  return 0;
}

int CamUSBDevHwItf::setAeMode(enum HAL_AE_OPERATION_MODE aeMode) {
  int ret = 0;
  int mode = -1;
  switch (aeMode) {
    case HAL_AE_OPERATION_MODE_AUTO:
      mode = V4L2_EXPOSURE_AUTO;
      break;
    case HAL_AE_OPERATION_MODE_MANUAL:
      mode = V4L2_EXPOSURE_MANUAL;
      break;
    case HAL_AE_OPERATION_MODE_LONG_EXPOSURE:
    case HAL_AE_OPERATION_MODE_ACTION:
    case HAL_AE_OPERATION_MODE_VIDEO_CONFERENCE:
    case HAL_AE_OPERATION_MODE_PRODUCT_TEST:
    case HAL_AE_OPERATION_MODE_ULL:
    case HAL_AE_OPERATION_MODE_FIREWORKS:
    default :
      ret = -1;
  }

  if (ret == 0) {
    //set ae
    struct v4l2_ext_control ctrls[1];
    ctrls[0].id = V4L2_CID_EXPOSURE_AUTO;
    ctrls[0].value = mode;
    if (0 > (ret = mSpDev->setExtCtrls(ctrls, V4L2_CID_CAMERA_CLASS, 1)))
      ALOGE("%s:%d,set mode %d failed.", __func__, __LINE__, aeMode);
  }
  mCurAeMode = aeMode;

  return ret;

}
int CamUSBDevHwItf::getSupportedAeModes(vector<enum HAL_AE_OPERATION_MODE>& aeModes) {
  UNUSED_PARAM(aeModes);
  //TODO: may be got from XML config file
  return -1;
}
int CamUSBDevHwItf::getAeMode(enum HAL_AE_OPERATION_MODE& aeMode) {
  aeMode = mCurAeMode;
  return 0;
}

int CamUSBDevHwItf::getAeBias(int& curAeBias) {
  curAeBias = mCurAeBias;
  return 0;
}

int CamUSBDevHwItf::getSupportedBiasRange(HAL_RANGES_t& range) {
  //UNUSED_PARAM(range);
  //TODO: may be got from XML files
  range.step = 50;
  range.min = -300;
  range.max = 300;
  return 0;
}

int CamUSBDevHwItf::setAbsExp(int exposure) {
#if 0
  int ret = 0;
  struct v4l2_ext_control ctrls[1];
  ctrls[0].id = V4L2_CID_EXPOSURE_ABSOLUTE;
  ctrls[0].value = exposure;
  if (0 > (ret = mSpDev->setExtCtrls(ctrls, V4L2_CID_CAMERA_CLASS, 1)))
    ALOGE("%s:%d,set %d failed.", __func__, __LINE__, exposure);

  mCurAbsExp = exposure;
  return ret;
#else
  if (mCurAbsExp == exposure)
    return 0;

  int ret = mSpDev->setCtrl(V4L2_CID_EXPOSURE, exposure);

  if (ret == 0)
    mCurAbsExp = exposure;
  else {
    LOGW("Could not set exposure, error %d", ret);
  }

  return ret;

#endif
}

int CamUSBDevHwItf::getAbsExp(int& curExposure) {
  curExposure = mCurAbsExp;
  return 0;
}

int CamUSBDevHwItf::getSupportedExpMeterModes(vector<enum HAL_AE_METERING_MODE> modes) {
  UNUSED_PARAM(modes);
  //TODO : may be got from xml file
  return -1;
}
int CamUSBDevHwItf::setExposureMeterMode(enum HAL_AE_METERING_MODE aeMeterMode) {
  int ret = 0;
  int mode = -1;
  switch (aeMeterMode) {
    case HAL_AE_METERING_MODE_AVERAGE:
      mode = V4L2_EXPOSURE_METERING_AVERAGE;
      break;
    case HAL_AE_METERING_MODE_CENTER:
      mode = V4L2_EXPOSURE_METERING_CENTER_WEIGHTED ;
      break;
    case HAL_AE_METERING_MODE_SPOT:
      mode = V4L2_EXPOSURE_METERING_SPOT;
      break;
    case HAL_AE_METERING_MODE_MATRIX:
      mode = V4L2_EXPOSURE_METERING_MATRIX;
      break;
    default :
      ret = -1;
  }

  if (ret == 0) {
    //set ae
    struct v4l2_ext_control ctrls[1];
    ctrls[0].id = V4L2_CID_EXPOSURE_METERING;
    ctrls[0].value = mode;
    if (0 > (ret = mSpDev->setExtCtrls(ctrls, V4L2_CID_CAMERA_CLASS, 1)))
      ALOGE("%s:%d,set mode %d failed.", __func__, __LINE__, aeMeterMode);
  }
  mCurAeMeterMode = aeMeterMode;

  return ret;
}
int CamUSBDevHwItf::getExposureMeterMode(enum HAL_AE_METERING_MODE& aeMeterMode) {
  aeMeterMode = mCurAeMeterMode;
  return 0;
}
int CamUSBDevHwItf::setFps(HAL_FPS_INFO_t fps) {
  int ret = 0;
  struct v4l2_subdev_frame_interval interval;

  if ((fps.numerator != m_fps.numerator) ||
      (fps.denominator != m_fps.denominator)) {

    interval.interval.numerator = fps.numerator;
    interval.interval.denominator = fps.denominator;

    ret = ioctl(m_cam_fd_imgsrc, VIDIOC_SUBDEV_S_FRAME_INTERVAL, &interval);
    if (ret < 0) {
      ALOGE("ERR(%s): ioctl VIDIOC_SUBDEV_S_FRAME_INTERVAL failed(%2.3f fps)!\n",
            __func__,
            fps.denominator / fps.numerator);
      return ret;
    }

    m_fps = fps;
  }

  return ret;
}

int CamUSBDevHwItf::enableAe(bool aeEnable) {
  UNUSED_PARAM(aeEnable);
  //do nothing
  return 0;
}

int CamUSBDevHwItf::getSupportedAntiBandModes(vector<enum HAL_AE_FLK_MODE>& flkModes) {
  flkModes.push_back(HAL_AE_FLK_OFF);
  flkModes.push_back(HAL_AE_FLK_50);
  flkModes.push_back(HAL_AE_FLK_60);
  return 0;
}

int CamUSBDevHwItf::setAntiBandMode(enum HAL_AE_FLK_MODE flkMode) {
  int mode = -1, ret = 0;
  if (mFlkMode == flkMode)
    return 0;
  switch (flkMode) {
    case HAL_AE_FLK_OFF:
      mode = V4L2_CID_POWER_LINE_FREQUENCY_DISABLED;
      break;
    case HAL_AE_FLK_50:
      mode = V4L2_CID_POWER_LINE_FREQUENCY_50HZ ;
      break;
    case HAL_AE_FLK_60:
      mode = V4L2_CID_POWER_LINE_FREQUENCY_60HZ  ;
      break;
    case HAL_AE_FLK_AUTO:
      mode = V4L2_CID_POWER_LINE_FREQUENCY_AUTO ;
      break;
    default :
      ret = -1;
  }

  if (ret < 0)
    return -1;
  ret = mSpDev->setCtrl(V4L2_CID_POWER_LINE_FREQUENCY, mode);

  if (ret == 0)
    mFlkMode = flkMode;
  else {
    LOGW("Could not set antiband mode %d", flkMode);
  }

  return ret;
}

//AWB
int CamUSBDevHwItf::getSupportedWbModes(vector<HAL_WB_MODE>& modes) {
  //TODO: may be got from xml file
  //UNUSED_PARAM(modes);
  modes.push_back(HAL_WB_AUTO);
  modes.push_back(HAL_WB_CLOUDY_DAYLIGHT);
  modes.push_back(HAL_WB_DAYLIGHT);
  modes.push_back(HAL_WB_FLUORESCENT);
  modes.push_back(HAL_WB_INCANDESCENT);
  return 0;
}

int CamUSBDevHwItf::getWhiteBalanceMode(HAL_WB_MODE& wbMode) {
  wbMode = m_wb_mode;
  return 0;
}
int CamUSBDevHwItf::enableAwb(bool awbEnable) {
  UNUSED_PARAM(awbEnable);
  //do nothing
  return 0;
}

//AF
int CamUSBDevHwItf::setFocusPos(int position) {
  //TODO : V4L2_CID_FOCUS_ABSOLUTE is a extended class control, could it be setted by mSpDev->setCtrl ?
  //TODO: should check current focus mode ?
  int ret = mSpDev->setCtrl(V4L2_CID_FOCUS_ABSOLUTE, position);

  if (ret < 0) {
    LOGE("Could not set focus, error %d", ret);
  }
  mLastLensPosition = position;
  return ret;
}
int CamUSBDevHwItf::getFocusPos(int& position) {
  int ret = mSpDev->getCtrl(V4L2_CID_FOCUS_ABSOLUTE);

  if (ret < 0) {
    LOGE("Could not get focus, error %d", ret);
  } else {
    position = ret;
  }

  return ret;
}
int CamUSBDevHwItf::getSupportedFocusModes(vector<enum HAL_AF_MODE> fcModes) {
  //TODO:may be got from xml file
  UNUSED_PARAM(fcModes);
  return -1;
}
int CamUSBDevHwItf::setFocusMode(enum HAL_AF_MODE fcMode) {
  int ret = 0;
  int mode = -1;
  switch (fcMode) {
    case HAL_AF_MODE_AUTO:
      mode = V4L2_AUTO_FOCUS_RANGE_AUTO ;
      break;
    case HAL_AF_MODE_MACRO:
      mode = V4L2_AUTO_FOCUS_RANGE_MACRO  ;
      break;
    case HAL_AF_MODE_INFINITY:
      mode = V4L2_AUTO_FOCUS_RANGE_INFINITY;
      break;
    case HAL_AF_MODE_FIXED:
    case HAL_AF_MODE_EDOF:
    default :
      mode = V4L2_AUTO_FOCUS_RANGE_AUTO;
  }

  if ((fcMode == HAL_AF_MODE_CONTINUOUS_VIDEO) || (fcMode == HAL_AF_MODE_CONTINUOUS_PICTURE)) {
    struct v4l2_ext_control ctrls[1];
    ctrls[0].id = V4L2_CID_FOCUS_AUTO;
    ctrls[0].value = 1;
    if (0 > (ret = mSpDev->setExtCtrls(ctrls, V4L2_CID_CAMERA_CLASS, 1)))
      ALOGE("%s:%d,set mode %d failed.", __func__, __LINE__, fcMode);
  } else {
    struct v4l2_ext_control ctrls[1];
    if ((mAfMode == HAL_AF_MODE_CONTINUOUS_VIDEO) || (mAfMode == HAL_AF_MODE_CONTINUOUS_PICTURE)) {
      //stop continuous focus firstly
      ctrls[0].id = V4L2_CID_FOCUS_AUTO;
      ctrls[0].value = 0;
      if (0 > (ret = mSpDev->setExtCtrls(ctrls, V4L2_CID_CAMERA_CLASS, 1)))
        ALOGE("%s:%d,set mode %d failed.", __func__, __LINE__, fcMode);

    }
    ctrls[0].id = V4L2_CID_AUTO_FOCUS_RANGE;
    ctrls[0].value = mode;
    if (0 > (ret = mSpDev->setExtCtrls(ctrls, V4L2_CID_CAMERA_CLASS, 1)))
      ALOGE("%s:%d,set mode %d failed.", __func__, __LINE__, fcMode);
  }
  mAfMode = fcMode;

  return ret;
}
int CamUSBDevHwItf::getFocusMode(enum HAL_AF_MODE& fcMode) {
  fcMode = mAfMode;
  return 0;
}

int CamUSBDevHwItf::getAfStatus(enum HAL_AF_STATUS& afStatus) {
  int ret = 0;
  struct v4l2_ext_control ctrls[1];
  ctrls[0].id = V4L2_CID_AUTO_FOCUS_STATUS;
  if (0 > (ret = mSpDev->setExtCtrls(ctrls, V4L2_CID_CAMERA_CLASS, 1)))
    ALOGE("%s:%d,start auto focus  failed.", __func__, __LINE__);
  else
    afStatus = (enum HAL_AF_STATUS)(ctrls[0].value);
  return ret;

}

int CamUSBDevHwItf::trigggerAf(bool trigger) {
  int ret = 0;
  if ((mAfMode == HAL_AF_MODE_AUTO) || (mAfMode == HAL_AF_MODE_MACRO)) {

    struct v4l2_ext_control ctrls[1];
    if (trigger)
      ctrls[0].id = V4L2_CID_AUTO_FOCUS_START;
    else
      ctrls[0].id = V4L2_CID_AUTO_FOCUS_STOP;
    ctrls[0].value = 1;
    if (0 > (ret = mSpDev->setExtCtrls(ctrls, V4L2_CID_CAMERA_CLASS, 1)))
      ALOGE("%s:%d,trigger af %d  failed.", __func__, __LINE__, trigger);
  }
  return ret;
}

int CamUSBDevHwItf::enableAf(bool afEnable) {
  UNUSED_PARAM(afEnable);
  return -1;
}

int CamUSBDevHwItf::getSupported3ALocks(vector<enum HAL_3A_LOCKS>& locks) {
  UNUSED_PARAM(locks);
  //TODO:may be got from xml file
  return -1;
}
int CamUSBDevHwItf::set3ALocks(int locks) {
  int ret = 0;
  int setLock = locks & HAL_3A_LOCKS_ALL;

  struct v4l2_ext_control ctrls[1];
  ctrls[0].id = V4L2_CID_3A_LOCK;
  ctrls[0].value = setLock;
  if (0 > (ret = mSpDev->setExtCtrls(ctrls, V4L2_CID_CAMERA_CLASS, 1)))
    ALOGE("%s:%d,set 3A lock 0x%x  failed.", __func__, __LINE__, setLock);
  m3ALocks = setLock;
  return ret;

}
int CamUSBDevHwItf::get3ALocks(int& curLocks) {
  curLocks = m3ALocks;
  return 0;
}

bool CamUSBDevHwItf::enumSupportedFmts(vector<RK_FRMAE_FORMAT>& frmFmts) {
  vector<unsigned int> fmtVec;
  mSpDev->enumFormat(fmtVec);
  for (int i = 0; i < fmtVec.size(); i++) {
    frmFmts.push_back(mSpDev->V4l2FmtToHalFmt(fmtVec[i]));
  }
  return true;
}
bool CamUSBDevHwItf::enumSupportedSizes(RK_FRMAE_FORMAT frmFmt, vector<frm_size_t>& frmSizes) {
  mSpDev->enumFrmSize(mSpDev->halFmtToV4l2Fmt(frmFmt), frmSizes);
  return true;
}
bool CamUSBDevHwItf::enumSupportedFps(RK_FRMAE_FORMAT frmFmt, frm_size_t frmSize, vector<HAL_FPS_INFO_t>& fpsVec) {
  mSpDev->enumFrmFps(mSpDev->halFmtToV4l2Fmt(frmFmt),
                     frmSize.width, frmSize.height,
                     fpsVec);
  return true;
}
int  CamUSBDevHwItf::tryFormat(frm_info_t inFmt, frm_info_t& outFmt) {
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

bool CamUSBDevHwItf::enumSensorFmts(vector<frm_info_t>& frmInfos) {
  //enum fmts
  vector<unsigned int> fmtVec;
  mSpDev->enumFormat(fmtVec);
  for (int i = 0; i < fmtVec.size(); i++) {
    //enum size for every fmt
    vector<frm_size_t> frmSizeVec;
    mSpDev->enumFrmSize(fmtVec[i], frmSizeVec);
    //enum fps for every size
    for (int j = 0; j < frmSizeVec.size(); j++) {
      vector<HAL_FPS_INFO_t> fpsVec;
      mSpDev->enumFrmFps(fmtVec[i],
                         frmSizeVec[j].width, frmSizeVec[j].height,
                         fpsVec);
      //copy out
      frm_info_t fmtInfo;
      fmtInfo.frmFmt = mSpDev->V4l2FmtToHalFmt(fmtVec[i]);
      fmtInfo.frmSize.width = frmSizeVec[j].width;
      fmtInfo.frmSize.height = frmSizeVec[j].height;
      for (int n = 0; n < fpsVec.size(); n++) {
        fmtInfo.fps = fpsVec[n].denominator / fpsVec[n].numerator ;
        frmInfos.push_back(fmtInfo);
      }
    }
  }
  return true;
}

//flash control
int CamUSBDevHwItf::getSupportedFlashModes(vector<enum HAL_FLASH_MODE>& flModes) {
  UNUSED_PARAM(flModes);
  //TODO:may be got from xml file
  return -1;
}
int CamUSBDevHwItf::setFlashLightMode(enum HAL_FLASH_MODE flMode, int intensity, int timeout) {
  int ret = 0;
  struct v4l2_ext_control ctrls[5];
  int ctrNum = 0;


  if (flMode == HAL_FLASH_OFF) {
    ctrls[0].id = V4L2_CID_FLASH_LED_MODE;
    ctrls[0].value = V4L2_FLASH_LED_MODE_NONE;
    ctrNum = 1;
  } else if (flMode == HAL_FLASH_ON) {
    ctrls[0].id = V4L2_CID_FLASH_LED_MODE;
    ctrls[0].value = V4L2_FLASH_LED_MODE_FLASH;

    ctrls[1].id = V4L2_CID_FLASH_INTENSITY;
    ctrls[1].value = intensity;

    ctrls[2].id = V4L2_CID_FLASH_TIMEOUT;
    ctrls[2].value = timeout;

    ctrls[3].id = V4L2_CID_FLASH_STROBE_SOURCE;
    ctrls[3].value = V4L2_FLASH_STROBE_SOURCE_SOFTWARE;
    // should check V4L2_CID_FLASH_READY before strobe ?
    ctrls[4].id = V4L2_CID_FLASH_STROBE;
    ctrls[4].value = 1;
    ctrNum = 5;
  } else if (flMode == HAL_FLASH_TORCH) {

    ctrls[0].id = V4L2_CID_FLASH_LED_MODE;
    ctrls[0].value = V4L2_FLASH_LED_MODE_TORCH;

    ctrls[1].id = V4L2_CID_FLASH_TORCH_INTENSITY;
    ctrls[1].value = intensity;

    ctrNum = 2;
  }

  if ((ctrNum > 0) &&
      (0 > (ret = mSpDev->setExtCtrls(ctrls, V4L2_CTRL_CLASS_FLASH, ctrNum)))
     )
    ALOGE("%s:%d  failed.", __func__, __LINE__);
  mFlMode = flMode;
  return ret;
}
int CamUSBDevHwItf::getFlashLightMode(enum HAL_FLASH_MODE& flMode) {
  flMode = mFlMode;
  return true;
}
//color effect
int CamUSBDevHwItf::getSupportedImgEffects(vector<enum HAL_IAMGE_EFFECT>& imgEffs) {
  //TODO:may be got from xml file
  UNUSED_PARAM(imgEffs);
  return -1;
}
int CamUSBDevHwItf::setImageEffect(enum HAL_IAMGE_EFFECT image_effect) {
  int ret = 0;
  //LOGV("%s %d 0x%x)", __func__, image_effect, V4L2_CID_COLORFX);

  enum v4l2_colorfx colorfx;
  enum HAL_IAMGE_EFFECT newEffect;

  switch (image_effect) {
    case HAL_EFFECT_SEPIA:
      colorfx = V4L2_COLORFX_SEPIA;
      newEffect = HAL_EFFECT_SEPIA;
      break;
    case HAL_EFFECT_MONO:
      colorfx = V4L2_COLORFX_BW;
      newEffect = HAL_EFFECT_MONO;
      break;
    case HAL_EFFECT_NEGATIVE:
      colorfx = V4L2_COLORFX_NEGATIVE;
      newEffect = HAL_EFFECT_NEGATIVE;
      break;
    case HAL_EFFECT_EMBOSS:
      colorfx = V4L2_COLORFX_EMBOSS;
      newEffect = HAL_EFFECT_EMBOSS;
      break;
    case HAL_EFFECT_SKETCH:
      colorfx = V4L2_COLORFX_SKETCH;
      newEffect = HAL_EFFECT_SKETCH;
      break;
    default:
      colorfx = V4L2_COLORFX_NONE;
      newEffect = HAL_EFFECT_NONE;
      break;
  }

  ret = mSpDev->setCtrl(V4L2_CID_COLORFX, (unsigned int) colorfx);
  if (ret < 0) {
    LOGE("ERR(%s):Fail on V4L2_CID_COLORFX", __func__);
    return ret;
  }

  m_image_effect = newEffect;
  return ret;
}
int CamUSBDevHwItf::getImageEffect(enum HAL_IAMGE_EFFECT& image_effect) {
  image_effect = m_image_effect;
  return true;
}

//scene
int CamUSBDevHwItf::getSupportedSceneModes(vector<enum HAL_SCENE_MODE>& sceneModes) {
  //TODO:may be got from xml file
  UNUSED_PARAM(sceneModes);
  return -1;
}
int CamUSBDevHwItf::setSceneMode(enum HAL_SCENE_MODE sceneMode) {
  int ret = 0;
  int mode = V4L2_SCENE_MODE_NONE;
  switch (sceneMode) {
    case HAL_SCENE_MODE_AUTO :
      mode = V4L2_SCENE_MODE_NONE;
      break;
    case HAL_SCENE_MODE_BACKLIGHT:
      mode = V4L2_SCENE_MODE_BACKLIGHT;
      break;
    case HAL_SCENE_MODE_BEACH_SNOW:
      mode = V4L2_SCENE_MODE_BEACH_SNOW;
      break;
    case HAL_SCENE_MODE_CANDLE_LIGHT:
      mode = V4L2_SCENE_MODE_CANDLE_LIGHT;
      break;
    case HAL_SCENE_MODE_DAWN_DUSK:
      mode = V4L2_SCENE_MODE_DAWN_DUSK;
      break;
    case HAL_SCENE_MODE_FALL_COLORS:
      mode = V4L2_SCENE_MODE_FALL_COLORS;
      break;
    case HAL_SCENE_MODE_FIREWORKS:
      mode = V4L2_SCENE_MODE_FIREWORKS;
      break;
    case HAL_SCENE_MODE_LANDSCAPE:
      mode = V4L2_SCENE_MODE_LANDSCAPE;
      break;
    case HAL_SCENE_MODE_NIGHT:
      mode = V4L2_SCENE_MODE_NIGHT;
      break;
    case HAL_SCENE_MODE_PARTY_INDOOR:
      mode = V4L2_SCENE_MODE_PARTY_INDOOR;
      break;
    case HAL_SCENE_MODE_PORTRAIT:
      mode = V4L2_SCENE_MODE_PORTRAIT;
      break;
    case HAL_SCENE_MODE_SPORTS:
      mode = V4L2_SCENE_MODE_SPORTS;
      break;
    case HAL_SCENE_MODE_SUNSET:
      mode = V4L2_SCENE_MODE_SUNSET;
      break;
    case HAL_SCENE_MODE_TEXT:
      mode = V4L2_SCENE_MODE_TEXT;
      break;
    default:
      ALOGE("%s:%d,not support this mode %d.", __func__, __LINE__, sceneMode);
      return -1;
  }

  struct v4l2_ext_control ctrls[1];
  ctrls[0].id = V4L2_CID_SCENE_MODE ;
  ctrls[0].value = mode;
  if (0 > (ret = mSpDev->setExtCtrls(ctrls, V4L2_CID_CAMERA_CLASS, 1)))
    ALOGE("%s:%d,set mode %d  failed.", __func__, __LINE__, sceneMode);
  mSceneMode = sceneMode;
  return ret;
}
int CamUSBDevHwItf::getSceneMode(enum HAL_SCENE_MODE& sceneMode) {
  sceneMode = mSceneMode;
  return 0;
}

int CamUSBDevHwItf::getSupportedISOModes(vector<enum HAL_ISO_MODE>& isoModes) {
  //TODO:may be got from xml file
  UNUSED_PARAM(isoModes);
  return -1;
}
int CamUSBDevHwItf::setISOMode(enum HAL_ISO_MODE isoMode, int sens) {
  int ret = 0;
  struct v4l2_ext_control ctrls[2];
  int ctrNum = 1;
  if (isoMode == HAL_ISO_MODE_AUTO) {
    ctrls[0].id = V4L2_CID_ISO_SENSITIVITY_AUTO  ;
    ctrls[0].value = V4L2_ISO_SENSITIVITY_AUTO;
  } else {
    ctrls[0].id = V4L2_CID_ISO_SENSITIVITY_AUTO  ;
    ctrls[0].value = V4L2_ISO_SENSITIVITY_MANUAL;
    ctrls[1].id = V4L2_CID_ISO_SENSITIVITY  ;
    ctrls[1].value = sens;
    ctrNum = 2;
  }
  if (0 > (ret = mSpDev->setExtCtrls(ctrls, V4L2_CID_CAMERA_CLASS, ctrNum)))
    ALOGE("%s:%d,set mode %d  failed.", __func__, __LINE__, isoMode);
  mIsoMode = isoMode;
  return ret;
}
int CamUSBDevHwItf::getISOMode(enum HAL_ISO_MODE& isoMode) {
  isoMode = mIsoMode;
  return 0;
}

int CamUSBDevHwItf::getSupportedZoomRange(HAL_RANGES_t& zoomRange) {
  //TODO:may be got from xml file
  UNUSED_PARAM(zoomRange);
  return -1;
}
int CamUSBDevHwItf::setZoom(int zoomVal) {
  int ret = 0;
  struct v4l2_ext_control ctrls[2];
  int ctrNum = 1;
  ctrls[0].id = V4L2_CID_ZOOM_ABSOLUTE ;
  ctrls[0].value = zoomVal;
  if (0 > (ret = mSpDev->setExtCtrls(ctrls, V4L2_CID_CAMERA_CLASS, ctrNum)))
    ALOGE("%s:%d,set  %d  failed.", __func__, __LINE__, zoomVal);
  mZoom = zoomVal;
  return ret;
}
int CamUSBDevHwItf::getZoom(int& zoomVal) {
  zoomVal = mZoom;
  return 0;
}

//brightness
int CamUSBDevHwItf::getSupportedBtRange(HAL_RANGES_t& brightRange) {
  //TODO:may be got from xml file
  UNUSED_PARAM(brightRange);
  return -1;
}
int CamUSBDevHwItf::setBrightness(int brightVal) {
  int ret = mSpDev->setCtrl(V4L2_CID_BRIGHTNESS, brightVal);

  if (ret == 0)
    mBrightness = brightVal;
  else {
    LOGW("Could not set Brightness, error %d", ret);
  }

  return ret;
}
int CamUSBDevHwItf::getBrithtness(int& brightVal) {
  mBrightness = brightVal;
  return 0;
}
//contrast
int CamUSBDevHwItf::getSupportedCtRange(HAL_RANGES_t& contrastRange) {
  //TODO:may be got from xml file
  UNUSED_PARAM(contrastRange);
  return -1;
}
int CamUSBDevHwItf::setContrast(int contrast) {
  int ret = mSpDev->setCtrl(V4L2_CID_CONTRAST, contrast);

  if (ret == 0)
    mContrast = contrast;
  else {
    LOGW("Could not set contrast, error %d", ret);
  }

  return ret;
}
int CamUSBDevHwItf::getContrast(int& contrast) {
  mContrast = contrast;
  return 0;
}
//saturation
int CamUSBDevHwItf::getSupportedStRange(HAL_RANGES_t& saturationRange) {
  //TODO:may be got from xml file
  UNUSED_PARAM(saturationRange);
  return -1;
}
int CamUSBDevHwItf::setSaturation(int sat) {
  int ret = mSpDev->setCtrl(V4L2_CID_SATURATION, sat);

  if (ret == 0)
    mSaturation = sat;
  else {
    LOGW("Could not set saturation, error %d", ret);
  }

  return ret;
}
int CamUSBDevHwItf::getSaturation(int& sat) {
  mSaturation = sat;
  return 0;
}
//hue
int CamUSBDevHwItf::getSupportedHueRange(HAL_RANGES_t& hueRange) {
  //TODO:may be got from xml file
  UNUSED_PARAM(hueRange);
  return -1;
}
int CamUSBDevHwItf::setHue(int hue) {
  int ret = mSpDev->setCtrl(V4L2_CID_HUE, hue);

  if (ret == 0)
    mHue = hue;
  else {
    LOGW("Could not set hue, error %d", ret);
  }

  return ret;
}
int CamUSBDevHwItf::getHue(int& hue) {
  mHue = hue;
  return 0;
}

int CamUSBDevHwItf::setFlip(int flip) {
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

int CamUSBDevHwItf::getFlip(int& flip) {
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

int CamUSBDevHwItf::queryBusInfo(unsigned char* busInfo) {
  return mSpDev->queryBusInfo(busInfo);
}

//ircut
//-1 means unsupported
int CamUSBDevHwItf::isSupportedIrCut() {
  return mSpDev->getCtrl(V4L2_CID_BAND_STOP_FILTER);
}
//>0 means IRCUT is working
int CamUSBDevHwItf::getIrCutState() {
  return mSpDev->getCtrl(V4L2_CID_BAND_STOP_FILTER);
}

int CamUSBDevHwItf::setIrCutState(int state) {
  return mSpDev->setCtrl(V4L2_CID_BAND_STOP_FILTER, state);

}


