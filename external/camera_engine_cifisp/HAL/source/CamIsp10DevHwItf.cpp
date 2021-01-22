#include <fcntl.h>
#include <sys/ioctl.h>
#include <iostream>
#include "V4L2DevIoctr.h"
#include "CamIsp10CtrItf.h"
#include "CamIsp10DevHwItf.h"
#include "common/return_codes.h"
#include "cam_ia_api/cam_ia10_trace.h"
#include "CameraIspTunning.h"
#ifdef SUPPORT_ION
#include "IonCameraBuffer.h"
#endif
#ifdef SUPPORT_DMABUF
#include "DMABufCameraBuffer.h"
#endif

using namespace std;

CamIsp10DevHwItf::Path::Path(CamIsp10DevHwItf* camIsp,
                             V4L2DevIoctr* camDev,
                             PATHID pathID,
                             unsigned long dequeueTimeout):
  mCamIsp(camIsp),
  PathBase(camIsp, camDev, pathID, dequeueTimeout) {
  mFmtInfo.frmFmt = HAL_FRMAE_FMT_NV12;
}

CamIsp10DevHwItf::Path::~Path() {

}


bool CamIsp10DevHwItf::Path::prepare(
    frm_info_t&  frmFmt,
    unsigned int numBuffers,
    CameraBufferAllocator& bufferAllocator,
    bool cached,
    unsigned int minNumBuffersQueued) {
  shared_ptr<BufferBase> buffer;
  unsigned int stride = 0;
  unsigned int mem_usage = 0;


  //ALOGV("%s: path id %d format %s %dx%d, numBuffers %d, minNumBuffersQueued %d", __func__,
  //      mPathID, RK_HAL_FMT_STRING::hal_fmt_map_to_str(frmFmt.frmFmt),
  //  frmFmt.frmSize.width,frmFmt.frmSize.height, numBuffers, minNumBuffersQueued);

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
  mBufferAllocator = &bufferAllocator;
  mMinNumBuffersQueued = minNumBuffersQueued;
  mFmtInfo = frmFmt;
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
    buffer = mBufferAllocator->alloc(RK_HAL_FMT_STRING::hal_fmt_map_to_str(frmFmt.frmFmt)
                                     , (frmFmt.frmSize.width), (frmFmt.frmSize.height), mem_usage, shared_from_this());

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
    if (mCamDev->queueBuffer(*i) < 0) {
      releaseBuffers();
      return false;
    }

    //mmap metada buffer
    mCamDev->memMap(*i);
    mNumBuffersQueued++;
  }

  //ALOGV("%s: %s is now PREPARED", __func__, string(mPathID));

  mState = PREPARED;
  return true;
}


bool CamIsp10DevHwItf::Path::prepare(
    frm_info_t& frmFmt,
    list<shared_ptr<BufferBase> >& bufPool,
    unsigned int numBuffers,
    unsigned int minNumBuffersQueued) {
  shared_ptr<BufferBase> buffer;

  //ALOGV("%s: %s format %s %dx%d@%d/%dfps, stride %d, numBuffers %d, minNumBuffersQueued %d", __func__,
  //      string(mPathID), pixFmt, width, height, fps.mNumerator, fps.mDenominator, stride, numBuffers, minNumBuffersQueued);

  if ((mState == STREAMING) || (mState == PREPARED)) {
    //prepare called when streaming, only the same format is allowd
    if (mCamDev->getCurFmt() != V4L2DevIoctr::halFmtToV4l2Fmt(frmFmt.frmFmt)) {
      ALOGW("format is different from current,req:%d,cur:%d",
            V4L2DevIoctr::halFmtToV4l2Fmt(frmFmt.frmFmt), mCamDev->getCurFmt());
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
  mMinNumBuffersQueued = minNumBuffersQueued;
  mNumBuffersQueued = 0;
  mFmtInfo = frmFmt;
  if ((frmFmt.frmFmt == HAL_FRMAE_FMT_JPEG) || (mPathID == DMA))
    mNumBuffersUndequeueable = 0;
  else
    mNumBuffersUndequeueable = 1;

  unsigned int j = 0;
  for (list<shared_ptr<BufferBase> >::iterator i = bufPool.begin(); i != bufPool.end(); i++) {
    (*i)->setIndex(j++);
    mBufferPool.push_back(*i);
    if (j == numBuffers)
      break;

  }

  if (0 > mCamDev->requestBuffers(mNumBuffers)) {
    releaseBuffers();
    return false;
  }

  mNumBuffersQueued = 0;
  for (list<shared_ptr<BufferBase> >::iterator i = mBufferPool.begin(); i != mBufferPool.end(); i++) {
    if (mCamDev->queueBuffer(*i) < 0) {
      releaseBuffers();
      return false;
    }

    //mmap metada buffer
    mCamDev->memMap(*i);
    mNumBuffersQueued++;
  }

  //ALOGV("%s: %s is now PREPARED", __func__, string(mPathID));

  mState = PREPARED;
  return true;
}


void CamIsp10DevHwItf::Path::addBufferNotifier(NewCameraBufferReadyNotifier* bufferReadyNotifier) {
  osMutexLock(&mNotifierLock);
  if (bufferReadyNotifier)
    mBufferReadyNotifierList.push_back(bufferReadyNotifier);
  osMutexUnlock(&mNotifierLock);

}
bool CamIsp10DevHwItf::Path::removeBufferNotifer(NewCameraBufferReadyNotifier* bufferReadyNotifier) {
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

bool CamIsp10DevHwItf::Path::releaseBufToOwener(weak_ptr<BufferBase> camBuf) {
  int ret = true;
  osMutexLock(&mBufLock);
  shared_ptr<BufferBase> spBuf = camBuf.lock();
  //ALOGD("%s: enter index = %d usedcnt = %d", __func__, spBuf->getIndex(), spBuf.use_count());
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

  //ALOGD("%s: exit! index = %d", __func__, spBuf->getIndex());
  return ret;
}

void CamIsp10DevHwItf::Path::releaseBuffers(void) {
  if (mState == STREAMING) {
    ALOGD("%s: path is also be using.", __func__);
    return;
  }
  osMutexLock(&mBufLock);
  //TODO:should wait all buffers are not used
  for (list<shared_ptr<BufferBase> >::iterator i = mBufferPool.begin(); i != mBufferPool.end(); i++) {
    //mmap buffer
    if (0 > mCamDev->memUnmap(*i)) {
      ALOGE("%s: ummap metadata  buffer failed", __func__);
    }
  }
  mBufferPool.clear();
  osMutexUnlock(&mBufLock);
}


bool CamIsp10DevHwItf::Path::start(void) {
  int ret;
  struct isp_supplemental_sensor_mode_data sensor_mode_data;
  struct sensor_config_info_s sensor_config;

  if (mState == STREAMING) {
    ALOGD("%s: %d is already in STREAMING state", __func__, mPathID);
    mPathRefCnt++;
    return true;
  } else if (mState != PREPARED) {
    ALOGE("%s: %d cannot start, path is not in PREPARED state", __func__, mPathID);
    return false;
  }

  ret = mCamDev->getSensorModeData(&sensor_mode_data);
  if (ret < 0) {
    ALOGE("%s: Path(%d) getSensorModeData failed", __func__,
          mPathID);
    return false;
  }

  //if (mCamDev->getSensorConfigInfo(&sensor_config) != 0) {
  // ALOGE("%s: get sensor config info error!", __func__);
  //  return false;
  //}

  if (mCamIsp->configIsp(&sensor_mode_data, NULL, true) < 0) {
    ALOGW("%s: Path(%d) configIsp failed", __func__,
          mPathID);
  } else {
    ALOGD("%s: Path(%d) configIsp succed", __func__,
          mPathID);
  }

  if (mCamDev->streamOn())
    goto failed_streamon;

  mState = STREAMING;
  mPathRefCnt++;
  /* TODO: skip some frames to wait AE stable, otherwise fliker may be
   * observed at the beginning.
   */
  mSkipFrames = 3;
  ret = mDequeueThread->run("pathTh");
  if (ret != RET_SUCCESS) {
    //mState = PREPARED;
    ALOGE("%s: %d thread start failed (error %d)", __func__, mPathID, ret);
    goto failed_threadrun;
  }
  return true;
failed_threadrun:
  mState = PREPARED;
  mPathRefCnt--;
failed_streamon:
  mCamIsp->configIsp(NULL, NULL, false);
  return false;
}

void CamIsp10DevHwItf::Path::stop(void) {
  //ALOGD("%s: E", __func__);
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
    // call this after all notifers have been called
    if (mCamIsp->configIsp(NULL, NULL, false) < 0) {
      ALOGW("%s: Path(%d) configIsp failed", __func__,
            mPathID);
    }
  } else
    osMutexUnlock(&mNumBuffersQueuedLock);


  osMutexLock(&mNumBuffersQueuedLock);
  if (mState == PREPARED) {
    mState = UNINITIALIZED;
    osMutexUnlock(&mNumBuffersQueuedLock);
    mCamDev->streamOff();
    mNumBuffersQueued = 0;
    mFmtInfo.frmFmt = HAL_FRMAE_FMT_NV12;
  } else
    osMutexUnlock(&mNumBuffersQueuedLock);
  //TODO should wait all buffers are returned.
  //ALOGD("%s: %d is now UNINITIALIZED", __func__, mPathID);
}

CamIsp10DevHwItf::CamIsp10DevHwItf(struct rk_isp_dev_info* isp_dev_info) {

  //ALOGD("%s: E", __func__);
  m_flag_init = false;
  memset(&mIspCfg, 0, sizeof(mIspCfg));
  osMutexInit(&mApiLock);
  mISPDevInfo = isp_dev_info;
  mISPBrightness = -120;
  mISPContrast = -200.0f;
  mISPHue = -200.0f;
  mISPSaturation = -200.0f;
  mAfMode = HAL_AF_MODE_NOT_SET;
  mAfTrigger = BOOL_FALSE;
  mAfSupport = BOOL_FALSE;
  mCapThdRun = false;
  memset(&mAfWin, 0, sizeof(mAfWin));
  //ALOGD("%s: x", __func__);
}
CamIsp10DevHwItf::~CamIsp10DevHwItf(void) {
  //ALOGD("%s: E", __func__);
  //ALOGD("%s: x", __func__);

}
shared_ptr<CamHwItf::PathBase> CamIsp10DevHwItf::getPath(enum PATHID id) {
  shared_ptr<CamHwItf::PathBase> path;

  switch (id) {
    case MP:
      path = mMp;
      break;
    case SP:
      path = mSp;
      break;
    case DMA:
      path = mDMAPath;
      break;
    default:
      break;
  }
  return path;
}
bool CamIsp10DevHwItf::initHw(int inputId) {
  bool ret = true;
  struct camera_module_info_s camera_module;
  bool default_iq;
  char  dev_name[32];
  const char* dev_name_p;
  //open devices
  if (!m_flag_init) {
    if (mISPDevInfo) {
      if (mISPDevInfo->isp_dev_node_nums < 4) {
        ALOGE("%s: erro isp video node num %d", __func__, mISPDevInfo->isp_dev_node_nums);
        return false;
      } else {
        //number 0 is self dev
        sprintf(dev_name, "/dev/video%d", mISPDevInfo->video_nodes[0].video_index);
        dev_name_p = dev_name;
      }
    } else
      dev_name_p = CAMERA_OVERLAY_DEV_NAME;

#ifdef CAMERAHAL_VIDEODEV_NONBLOCK
    m_cam_fd_overlay = open(dev_name_p, O_RDWR | O_NONBLOCK | O_CLOEXEC);
#else
    m_cam_fd_overlay = open(dev_name_p, O_RDWR);
#endif
    //LOGV("%s :m_cam_fd_overlay %d \n", __func__, m_cam_fd_overlay);
    if (m_cam_fd_overlay < 0) {
      LOGE("ERR(%s):Cannot open %s (error : %s)\n", __func__, dev_name_p, strerror(errno));
      ret =  false;
      goto err;
    }

    mSpDev = shared_ptr<V4L2DevIoctr>(new V4L2ISPDevIoctr(m_cam_fd_overlay, V4L2_BUF_TYPE_VIDEO_CAPTURE, V4L2_MEMORY_DMABUF));
    mSp = shared_ptr<CamHwItf::PathBase>(new Path(this, mSpDev.get(), CamHwItf::SP));

    if (mISPDevInfo) {
      if (mISPDevInfo->isp_dev_node_nums < 4) {
        ALOGE("%s: erro isp video node num %d", __func__, mISPDevInfo->isp_dev_node_nums);
        ret =  false;
        goto err;
      } else {
        //number 2 is main dev
        sprintf(dev_name, "/dev/video%d", mISPDevInfo->video_nodes[2].video_index);
        dev_name_p = dev_name;
      }
    } else
      dev_name_p = CAMERA_CAPTURE_DEV_NAME;
#ifdef CAMERAHAL_VIDEODEV_NONBLOCK
    m_cam_fd_capture = open(dev_name_p, O_RDWR | O_NONBLOCK | O_CLOEXEC);
#else
    m_cam_fd_capture = open(dev_name_p, O_RDWR);
#endif
    //LOGV("%s :m_cam_fd_capture %d \n", __func__, m_cam_fd_capture);
    if (m_cam_fd_capture < 0) {
      LOGE("ERR(%s):Cannot open %s (error : %s)\n", __func__, dev_name_p, strerror(errno));
      printf("%s:%d\n", __func__, __LINE__);
      ret =  false;
      goto err;
    }

    mMpDev = shared_ptr<V4L2DevIoctr>(new V4L2ISPDevIoctr(m_cam_fd_capture, V4L2_BUF_TYPE_VIDEO_CAPTURE, V4L2_MEMORY_DMABUF));
    mMp = shared_ptr<CamHwItf::PathBase>(new Path(this, mMpDev.get(), CamHwItf::MP));

    if (mISPDevInfo) {
      if (mISPDevInfo->isp_dev_node_nums < 4) {
        ALOGE("%s: erro isp video node num %d", __func__, mISPDevInfo->isp_dev_node_nums);
        ret =  false;
        goto err;
      } else {
        //number 3 is dma dev
        sprintf(dev_name, "/dev/video%d", mISPDevInfo->video_nodes[3].video_index);
        dev_name_p = dev_name;
      }
    } else
      dev_name_p = CAMERA_DMA_DEV_NAME;

#ifdef CAMERAHAL_VIDEODEV_NONBLOCK
    m_cam_fd_dma = open(dev_name_p, O_RDWR | O_NONBLOCK | O_CLOEXEC);
#else
    m_cam_fd_dma = open(dev_name_p, O_RDWR);
#endif
    //LOGV("%s :m_cam_fd_dma %d \n", __func__, m_cam_fd_dma);
    if (m_cam_fd_dma < 0) {
      LOGE("ERR(%s):Cannot open %s (error : %s)\n", __func__, dev_name_p, strerror(errno));
      printf("%s:%d\n", __func__, __LINE__);
      ret =  false;
      goto err;
    }

    mDmaPathDev = shared_ptr<V4L2DevIoctr>(new V4L2DevIoctr(m_cam_fd_dma, V4L2_BUF_TYPE_VIDEO_OUTPUT, V4L2_MEMORY_USERPTR));
    mDMAPath = shared_ptr<CamHwItf::PathBase>(new Path(this, mDmaPathDev.get(), CamHwItf::DMA));

    mISPDev = shared_ptr<CamIsp10CtrItf>(new CamIsp10CtrItf(this, m_cam_fd_overlay));
    if (mSpDev->queryCap(V4L2_CAP_VIDEO_CAPTURE) < 0) {
      ret = false;
      goto err;
    }
    if (mMpDev->queryCap(V4L2_CAP_VIDEO_CAPTURE) < 0) {
      ret = false;
      goto err;
    }

    //set input
    {
      struct v4l2_input input;

      input.index = 0;
      while (mSpDev->enumInput(&input) == 0) {
        if (input.index == inputId)
          break;
        input.index++;
      }

      if (inputId != input.index) {
        ALOGE("%s: error input camera id %d", __func__, inputId);
        ret = false;
        goto err;
      }
      sprintf(dev_name, "/dev/v4l-subdev%d", inputId);
      dev_name_p = dev_name;
      m_cam_fd_imgsrc = open(dev_name, O_RDWR);
      if (m_cam_fd_imgsrc < 0) {
        ALOGE("%s: open imgsrc subdev(%s) failed!\n",
              __func__,
              dev_name);
      } else
        ALOGD("%s: open imgsrc subdev(%s) success!\n",
              __func__,
              dev_name);
      mSpDev->setInput(inputId);
    }

    default_iq = false;
    strcpy(camera_module.module_name, "(null)");
    strcpy(camera_module.len_name, "(null)");
    strcpy(camera_module.fov_h, "(null)");
    strcpy(camera_module.fov_v, "(null)");
    strcpy(camera_module.focus_distance, "(null)");
    strcpy(camera_module.focal_length, "(null)");
    camera_module.facing = -1;
    camera_module.orientation = -1;
    camera_module.iq_mirror = false;
    camera_module.iq_flip = false;
    camera_module.flash_support = -1;
    memset(mIqPath, 0x00, sizeof(mIqPath));

    if (mSpDev->getCameraModuleInfo(&camera_module) == 0) {
      if (!strcmp(camera_module.module_name, "(null)") &&
          !strcmp(camera_module.len_name, "(null)")) {
        ALOGW("Camera module name and len name haven't configured in kernel dts file,"
              "so used default tuning file!");
        default_iq = true;
      } else {
        sprintf(mIqPath, "%scam_default.xml", CAMERA_IQ_FIRST_DIR);
        while (0 != access(mIqPath, R_OK)) {

          if (strcmp(camera_module.module_name, "(null)")) {
            if (mApiReboot.iq_path[0] != 0) {
              sprintf(mIqPath, "%s%s_%s.xml", mApiReboot.iq_path,
                    camera_module.sensor_name,
                    camera_module.module_name);
              if (0 == access(mIqPath, R_OK))
                break;
            }

            sprintf(mIqPath, "%s%s_%s.xml", CAMERA_IQ_FIRST_DIR,
                    camera_module.sensor_name,
                    camera_module.module_name);
            if (0 == access(mIqPath, R_OK))
              break;

            sprintf(mIqPath, "%s%s_%s.xml", CAMERA_IQ_SECOND_DIR,
                    camera_module.sensor_name,
                    camera_module.module_name);
            if (0 == access(mIqPath, R_OK))
              break;
          }

          if (strcmp(camera_module.len_name, "(null)")) {
            if (mApiReboot.iq_path[0] != 0) {
              sprintf(mIqPath, "%s%s_%s.xml", mApiReboot.iq_path,
                    camera_module.sensor_name,
                    camera_module.len_name);
              if (0 == access(mIqPath, R_OK))
                break;
            }

            sprintf(mIqPath, "%s%s_%s.xml", CAMERA_IQ_FIRST_DIR,
                    camera_module.sensor_name,
                    camera_module.len_name);
            if (0 == access(mIqPath, R_OK))
              break;

            sprintf(mIqPath, "%s%s_%s.xml", CAMERA_IQ_SECOND_DIR,
                    camera_module.sensor_name,
                    camera_module.len_name);
            if (0 == access(mIqPath, R_OK))
              break;
          }

          sprintf(mIqPath, "%scam_default.xml", CAMERA_IQ_SECOND_DIR);
          if (0 == access(mIqPath, R_OK))
            break;

          break; // no any IQ file
        }

        if (0 == access(mIqPath, R_OK)) {
          ALOGW("IQ file use from %s.", mIqPath);
        } else {
          ALOGW("Can not find any IQ file!!! The camera may be able to work, but the picture probably be wrong!");
        }
      }

    }

    mAfSupport = (bool_t)camera_module.af_support;
    mInputId = inputId;
    mCapThdRun = false;
	initApiItf(mISPDev.get());
    m_flag_init = 1;
    if (ret == false) {
err:
      deInitHw();
      return ret;
    }
  }
  return ret;
}
void CamIsp10DevHwItf::deInitHw() {
  //LOGV("%s :", __func__);

  //ALOGD("%s: E", __func__);
  if (m_flag_init) {

    if (m_cam_fd_capture > -1) {
      mMp.reset();
      mMpDev.reset();
      close(m_cam_fd_capture);
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

    if (m_cam_fd_imgsrc > -1) {
      close(m_cam_fd_imgsrc);
      m_cam_fd_imgsrc = -1;
    }
    mISPDev.reset();
    m_flag_init = 0;

    //ALOGD("%s: x", __func__);
  }
}


bool CamIsp10DevHwItf::configureISPModules(const void* config) {
  return mISPDev->configureISP(config);
}

int CamIsp10DevHwItf::configIsp(
    struct isp_supplemental_sensor_mode_data* sensor,
    struct sensor_config_info_s* sensor_config,
    bool enable) {
  frm_info_t frm_info;

  osMutexLock(&mApiLock);
  mMp->getFmtInfo(&frm_info);
  if (enable) {
    char  dev_name[15];
    const char* dev_name_p;

    //if (sensor_config->sensor_fmt[0] < CSI2_DT_RAW8) {
    //  ALOGD("%s: sensor format 0x%x, don't need 3A control", __func__, sensor_config->sensor_fmt[0]);
    //  osMutexUnlock(&mApiLock);
    // return 0;
    //}
    if (sensor) {
      configIsp_l(sensor);
    }
    if (mISPDevInfo) {
      if (mISPDevInfo->isp_dev_node_nums < 4) {
        ALOGE("%s: erro isp video node num %d", __func__, mISPDevInfo->isp_dev_node_nums);
        return false;
      } else {
        //number 1 is isp dev
        sprintf(dev_name, "/dev/video%d", mISPDevInfo->video_nodes[1].video_index);
        dev_name_p = dev_name;
      }
    } else
      dev_name_p = CAMERA_ISP_DEV_NAME;
    mISPDev->init(mIqPath, dev_name_p);
    if (frm_info.frmFmt < HAL_FRMAE_FMT_SBGGR12)
      mISPDev->start(true);
    else
      mISPDev->start(false);
  } else {
    mISPDev->stop();
    mISPDev->deInit();
  }
  osMutexUnlock(&mApiLock);

  return 0;
}

int CamIsp10DevHwItf::setExposure(unsigned int vts, unsigned int exposure, unsigned int gain, unsigned int gain_percent) {
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

int CamIsp10DevHwItf::setAutoAdjustFps(bool auto_adjust_fps){
  int ret;
  struct v4l2_control ctrl;

  ctrl.id = RK_V4L2_CID_AUTO_FPS;
  ctrl.value = auto_adjust_fps;

  if((m_fps.numerator != 0 || m_fps.denominator != 0) && auto_adjust_fps)
  {
	LOGE("(%s) !!!!!!!!Warning: hal set fixfps(%d/%d), but IQ xml set auto_adjust_fps(%d). AE must have problem !!!!!!!!\n",
         __func__, m_fps.denominator, m_fps.numerator, auto_adjust_fps);
  }
  
  ret = ioctl(m_cam_fd_overlay, VIDIOC_S_CTRL, &ctrl);

  if (ret < 0) 
  {
    LOGE("ERR(%s): oyyf set AE seting auto adjust fps enable(%d)to sensor config failed! ret:%d err: %s\n",
         __func__,
         auto_adjust_fps,ret,strerror(errno));
  } 

  return ret;
}

//capture
bool CamIsp10DevHwItf::capLoop() {
  bool ret = true;
  if (mCapResult.result != HAL_ISP_CAP_RUNNING)
    return false;

#ifdef SUPPORT_ION
  shared_ptr<IonCameraBufferAllocator> bufAlloc(new IonCameraBufferAllocator());
#endif
#ifdef SUPPORT_DMABUF
  shared_ptr<DMABufCameraBufferAllocator> bufAlloc(new DMABufCameraBufferAllocator());
#endif

  CameraIspTunning* tunningInstance = CameraIspTunning::createInstance(this, &mCapReq);
  if (tunningInstance) {
    frm_info_t frmFmt;
    if (tunningInstance->prepare(frmFmt, 4, bufAlloc)) {
      if (tunningInstance->start()) {
        tunningInstance->stop();//the stop will be waiting for all task over signal
      }
    }
  }
  mCapResult.result = HAL_ISP_CAP_FINISH;
  return ret;
}

int CamIsp10DevHwItf::startCap(struct HAL_ISP_Cap_Req_s& req) {
  if (mCapThdRun == true)
    return -1;

  mCapReq = req;
  mCapResult.cap_id = mCapReq.cap_id;
  mCapResult.result = HAL_ISP_CAP_RUNNING;
  mCapThdRun = true;
  mCapThread = shared_ptr<CamThread>(new capThread(this));
  if (mCapThread->run("CameraCapTask")){
    ALOGE("%s: run CameraCapTask error, %s!", __func__, strerror(errno));
    mCapResult.result = HAL_ISP_CAP_FINISH;
    mCapThdRun = false;
    return -1;
  }
  return 0;
}

int CamIsp10DevHwItf::getCapRes(struct HAL_ISP_Cap_Result_s& result) {
  result = mCapResult;

  if (mCapThdRun == true && mCapResult.result == HAL_ISP_CAP_FINISH) {
    if (mCapThread.get())
      mCapThread->requestExitAndWait();
    mCapThread.reset();
    mCapThdRun = false;
    deInitHw();
    initHw(mInputId);
  }
  return 0;
}

int CamIsp10DevHwItf::reInitHW(struct HAL_ISP_Reboot_Req_s& req) {
  mApiReboot.reboot = req.reboot;
  strcpy((char *)mApiReboot.iq_path, (char *)req.iq_path);
  if (mApiReboot.reboot == 1) {
    deInitHw();
    initHw(mInputId);
    mApiReboot.reboot = 0;
  }
}

void CamIsp10DevHwItf::transDrvMetaDataToHal
(
    const void* drvMeta,
    struct HAL_Buffer_MetaData* halMeta
) {
  /* IS raw format */
  if ((V4L2DevIoctr::V4l2FmtToHalFmt(mMpDev->getCurFmt())
       >= HAL_FRMAE_FMT_SBGGR12) &&
      (V4L2DevIoctr::V4l2FmtToHalFmt(mMpDev->getCurFmt())
       <= HAL_FRMAE_FMT_SRGGB8)) {
    struct v4l2_buffer_metadata_s* v4l2Meta =
        (struct v4l2_buffer_metadata_s*)drvMeta;
    struct cifisp_isp_metadata* ispMetaData =
        (struct cifisp_isp_metadata*)v4l2Meta->isp;
    if (ispMetaData)
      mMpDev->getSensorModeData(&(ispMetaData->meas_stat.sensor_mode));
  }

  mISPDev->transDrvMetaDataToHal(drvMeta, halMeta);
  mAEMeanLuma = halMeta->MeanLuma;
  mAEMaxGainRange = halMeta->maxGainRange; 
}

