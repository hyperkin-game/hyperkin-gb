#include "V4L2DevIoctr.h"
//#include <linux/rk-isp10-ioctl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
//#include <error.h>
#include "cam_ia_api/cam_ia10_trace.h"

using namespace std;

V4L2DevIoctr::V4L2DevIoctr(int devFp, const unsigned int v4l2BufType, const unsigned int v4l2MemType):
  mDevFp(devFp), mV4L2BufferType((v4l2_buf_type)v4l2BufType), mV4L2MemType((v4l2_memory)v4l2MemType) {
  osMutexInit(&mBufferQueueLock);
}
V4L2DevIoctr::~V4L2DevIoctr(void) {
  osMutexDestroy(&mBufferQueueLock);
}
int V4L2DevIoctr::queueBuffer(shared_ptr<BufferBase>& buffer) {
  struct v4l2_buffer v4l2Buf;
  int ret;

  //ALOGD("%s: index %d, addr 0x%p, capacity %d", __func__, buffer->getIndex(), buffer->getVirtAddr(), buffer->getCapacity());

  memset(&v4l2Buf, 0, sizeof(v4l2Buf));
  v4l2Buf.type = mV4L2BufferType;
  v4l2Buf.memory = mV4L2MemType;
  v4l2Buf.index = buffer->getIndex();
  if (mV4L2MemType == V4L2_MEMORY_DMABUF) 
    v4l2Buf.m.fd = buffer->getFd();
  else
    v4l2Buf.m.userptr = reinterpret_cast<unsigned long>(buffer->getVirtAddr());
  v4l2Buf.length = buffer->getCapacity();

  ret = ioctl(mDevFp, VIDIOC_QBUF, &v4l2Buf);
  if (ret < 0) {
    ALOGE("%s: QBUF failed; index %d type %d memory %d (error %d)", __func__, v4l2Buf.index, v4l2Buf.type, v4l2Buf.memory, ret);
    return ret;
  }

  osMutexLock(&mBufferQueueLock);
  mBufferQueue.push_back(buffer);
  osMutexUnlock(&mBufferQueueLock);
  return ret;
}
int V4L2DevIoctr::dequeueBuffer(shared_ptr<BufferBase>& buffer, unsigned long timeout_ms) {
  struct v4l2_buffer v4l2Buf;
  int ret;
  int retrycount = 3; /* xuhf@rock-chips.com: v1.0.0x28 */
  struct pollfd pfd;

  pfd.fd = mDevFp;
  if (V4L2_BUF_TYPE_VIDEO_OUTPUT == mV4L2BufferType)
    pfd.events = POLLOUT | POLLERR;
  else
    pfd.events = POLLIN | POLLERR;
  // ALOGV("%s: %s timeout %ldms", __func__, string(), timeout_ms);

  memset(&v4l2Buf, 0, sizeof(v4l2Buf));
  while (retrycount > 0) {
    ret = poll(&pfd, 1, timeout_ms);
    if (ret < 0) {
      ALOGE("%s: polling error (error %d)", __func__, ret);
      return ret;
    } else if (!ret) {
      ALOGW("%s: no data in %ld millisecs", __func__, timeout_ms);
      return -ETIMEDOUT;
    }

    v4l2Buf.type = mV4L2BufferType;
    v4l2Buf.memory = mV4L2MemType;

    ret = ioctl(mDevFp, VIDIOC_DQBUF, &v4l2Buf);
    if (ret < 0) {
      retrycount--;
      ALOGE("%s:DQBUF failed (error %d)", __func__, ret);
    } else
      break;
  }

  if (ret < 0)
    return ret;

  {
    osMutexLock(&mBufferQueueLock);
    for (list<weak_ptr<BufferBase> >::iterator i = mBufferQueue.begin(); i != mBufferQueue.end(); i++) {
      shared_ptr<BufferBase> tmpSpBuf;
      if (!(*i).expired())
        tmpSpBuf = (*i).lock();
      if (tmpSpBuf.get() && (tmpSpBuf->getIndex() == v4l2Buf.index)) {
        tmpSpBuf->setDataSize(v4l2Buf.bytesused);
        tmpSpBuf->setTimestamp(&v4l2Buf.timestamp);
        //(*i)->setTimestamps(&v4l2Buf_t.flash, &v4l2Buf_t.frame);
        buffer = tmpSpBuf;

        mBufferQueue.erase(i);
        //ALOGV("%s: %s index %d", __func__, string(), v4l2Buf.index);
        osMutexUnlock(&mBufferQueueLock);
        return 0;
      }
    }
    osMutexUnlock(&mBufferQueueLock);
  }

  ALOGE("%s: Could not find queued buffer with index %d", __func__, ret);
  return -EINVAL;
}

void V4L2DevIoctr::enumFormat(vector<unsigned int>& fmtVec) {
  struct v4l2_fmtdesc fmtdesc;
  bool found = false;

  fmtdesc.type = mV4L2BufferType;
  fmtdesc.index = 0;

  while (ioctl(mDevFp, VIDIOC_ENUM_FMT, &fmtdesc) == 0) {
    fmtVec.push_back(fmtdesc.pixelformat);
    fmtdesc.index++;
  }
}

void V4L2DevIoctr::enumFrmSize(unsigned int fmt, vector<frm_size_t>& frmSizeVec) {
  struct v4l2_frmsizeenum fsize;
  fsize.index = 0;
  fsize.pixel_format = fmt;
  frm_size_t frmSize;
  while (ioctl(mDevFp, VIDIOC_ENUM_FRAMESIZES, &fsize) == 0) {
    if (fsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
      frmSize.width = fsize.discrete.width;
      frmSize.height = fsize.discrete.height;
      frmSizeVec.push_back(frmSize);
    }
    fsize.index++;
  }
}

void V4L2DevIoctr::enumFrmFps(unsigned int fmt, unsigned int width, unsigned int height, vector<HAL_FPS_INFO_t>& fpsVec) {
  struct v4l2_frmivalenum fival;
  fival.index = 0;
  fival.pixel_format = fmt;
  fival.width = width;
  fival.height = height;
  HAL_FPS_INFO_t fps;
  while (ioctl(mDevFp, VIDIOC_ENUM_FRAMEINTERVALS, &fival) == 0) {
    if (fival.type == V4L2_FRMIVAL_TYPE_DISCRETE) {
      fps.denominator = fival.discrete.denominator;
      fps.numerator = fival.discrete.numerator;
      fpsVec.push_back(fps);
    } else if (fival.type == V4L2_FRMIVAL_TYPE_CONTINUOUS) {
      break;
    } else if (fival.type == V4L2_FRMIVAL_TYPE_STEPWISE) {
      break;
    }
    fival.index++;
  }
}

int V4L2DevIoctr::setFormat(unsigned int& v4l2PixFmt, unsigned int& v4l2ColorSpace, unsigned int& width, unsigned int& height, unsigned int stride) const {
  struct v4l2_format v4l2Fmt;
  int ret;

  if ((int)v4l2PixFmt < 0)
    return (int)v4l2PixFmt;

  /* make sure stride is at least equal to width */
  if (stride < width)
    stride = width;

  memset(&v4l2Fmt, 0, sizeof(v4l2Fmt));
  v4l2Fmt.type = mV4L2BufferType;
  v4l2Fmt.fmt.pix.width = width;
  v4l2Fmt.fmt.pix.height = height;
  v4l2Fmt.fmt.pix.bytesperline = bytesPerLine(v4l2PixFmt, stride);
  v4l2Fmt.fmt.pix.pixelformat = v4l2PixFmt;
  v4l2Fmt.fmt.pix.colorspace = v4l2ColorSpace;
  v4l2Fmt.fmt.pix.sizeimage = (width * height * bpp(v4l2PixFmt)) / 8;

  ret = ioctl(mDevFp, VIDIOC_S_FMT, &v4l2Fmt);
  if (ret < 0) {
    ALOGE("%s: S_FMT failed (error %d)", __func__, ret);
    return ret;
  }

  v4l2PixFmt = v4l2Fmt.fmt.pix.pixelformat;
  width = v4l2Fmt.fmt.pix.width;
  height = v4l2Fmt.fmt.pix.height;

  mFmt = v4l2Fmt.fmt.pix.pixelformat;
  mWidth = v4l2Fmt.fmt.pix.width;
  mHeight = v4l2Fmt.fmt.pix.height;
  mStride = v4l2Fmt.fmt.pix.width;

  return 0;
}

int V4L2DevIoctr::tryFormat(unsigned int& v4l2PixFmt, unsigned int& width, unsigned int& height) {
  struct v4l2_format v4l2Fmt;
  unsigned int stride = width;
  int ret = -1;

  if ((int)v4l2PixFmt < 0)
    return (int)v4l2PixFmt;

  memset(&v4l2Fmt, 0, sizeof(v4l2Fmt));
  v4l2Fmt.type = mV4L2BufferType;
  v4l2Fmt.fmt.pix.width = width;
  v4l2Fmt.fmt.pix.height = height;
  v4l2Fmt.fmt.pix.bytesperline = bytesPerLine(v4l2PixFmt, stride);
  v4l2Fmt.fmt.pix.pixelformat = v4l2PixFmt;
  if (v4l2PixFmt == V4L2_PIX_FMT_JPEG)
    v4l2Fmt.fmt.pix.colorspace = V4L2_COLORSPACE_JPEG;
  v4l2Fmt.fmt.pix.sizeimage = (width * height * bpp(v4l2PixFmt)) / 8;

  ret = ioctl(mDevFp, VIDIOC_TRY_FMT, &v4l2Fmt);
  if (ret < 0) {
    ALOGE("%s:TRY_FMT failed (error %d)", __func__, ret);
    return ret;
  } else {
    //if ((v4l2Fmt.fmt.pix.width == width ) && ( v4l2Fmt.fmt.pix.height == height))
    // driver will return the best matched format
    width = v4l2Fmt.fmt.pix.width;
    height = v4l2Fmt.fmt.pix.height;
    v4l2PixFmt = v4l2Fmt.fmt.pix.pixelformat;
    ret = 0;
  }
  return ret;
}

int V4L2DevIoctr::tryFps(unsigned int v4l2PixFmt, unsigned int width, unsigned int height, HAL_FPS_INFO_t fps) {

  struct v4l2_frmivalenum fival;
  fival.index = 0;
  fival.pixel_format = v4l2PixFmt;
  fival.width = width;
  fival.height = height;
  while (ioctl(mDevFp, VIDIOC_ENUM_FRAMEINTERVALS, &fival) == 0) {
    if (fival.type == V4L2_FRMIVAL_TYPE_DISCRETE) {
      if ((fps.denominator == fival.discrete.denominator) &&
          (fps.numerator == fival.discrete.numerator))
        return 0;
    } else if (fival.type == V4L2_FRMIVAL_TYPE_CONTINUOUS) {
      break;
    } else if (fival.type == V4L2_FRMIVAL_TYPE_STEPWISE) {
      break;
    }
    fival.index++;
  }
  return -EINVAL;
}

bool V4L2DevIoctr::getDefFmt(unsigned int& fmt) {
  bool ret = false;
  struct v4l2_fmtdesc fmtdesc;

  fmtdesc.type = mV4L2BufferType;
  fmtdesc.index = 0;

  if (ioctl(mDevFp, VIDIOC_ENUM_FMT, &fmtdesc) == 0) {
    fmt = fmtdesc.pixelformat;
    ret = true;
  }

  return ret;
}

//like tryFmt
bool V4L2DevIoctr::findFmt(unsigned int fmt) {
  struct v4l2_fmtdesc fmtdesc;
  bool found = false;

  fmtdesc.type = mV4L2BufferType;
  fmtdesc.index = 0;

  while (ioctl(mDevFp, VIDIOC_ENUM_FMT, &fmtdesc) == 0) {

    if (fmtdesc.pixelformat == fmt) {
      ALOGD("passed fmt = %#x found pixel format[%d]: %s\n", fmt, fmtdesc.index, fmtdesc.description);
      found = true;
      break;
    }
    fmtdesc.index++;
  }

  if (!found) {
    ALOGE("unsupported pixel format\n");
  }
  return found;
}
int V4L2DevIoctr::requestBuffers(unsigned int numBuffers) const {
  struct v4l2_requestbuffers req;
  int ret;

  req.count = numBuffers;
  req.type = mV4L2BufferType;
  req.memory = mV4L2MemType;

  ret = ioctl(mDevFp, VIDIOC_REQBUFS, &req);
  if (ret < 0) {
    ALOGE("%s:REQBUFS failed(error %d)", __func__, ret);
    return ret;
  }

  mReqBufCnt = req.count;
  return 0;

}
int V4L2DevIoctr::streamOn(void) {
  int ret;

  //ALOGV("%s: %s", __func__, string());

  ret = ioctl(mDevFp, VIDIOC_STREAMON, &mV4L2BufferType);
  if (ret < 0) {
    ALOGE("%s:STREAMON failed (error %d)", __func__, ret);
    return ret;
  }

  mStreaming = true;
  return 0;

}
int V4L2DevIoctr::streamOff(void) {
  int ret;

  //ALOGV("%s: %s", __func__, string());

  ret = ioctl(mDevFp, VIDIOC_STREAMOFF, &mV4L2BufferType);

  osMutexLock(&mBufferQueueLock);
  mBufferQueue.clear();
  osMutexUnlock(&mBufferQueueLock);
  mStreaming = false;
  if (ret < 0) {
    ALOGE("%s: STREAMOFF failed (error %d)", __func__, ret);
    return ret;
  }

  return 0;

}
//add for V4L2_MEMORY_MMAP memory type
bool V4L2DevIoctr::memMap(shared_ptr<BufferBase>& buffer) {
  if (V4L2_MEMORY_MMAP == mV4L2MemType) {
    struct v4l2_buffer v4l2buf;
    char* virtAddr = NULL;
    v4l2buf.flags = 0;
    v4l2buf.type = mV4L2BufferType;
    v4l2buf.memory = V4L2_MEMORY_MMAP;
    v4l2buf.index = buffer->getIndex();

    if (ioctl(mDevFp, VIDIOC_QUERYBUF, &v4l2buf) < 0) {
      ALOGE("%s(%d): VIDIOC_QUERYBUF Failed", __FUNCTION__, __LINE__);
      return false;
    }

    virtAddr = (char*)mmap(0 /* start anywhere */,
                           v4l2buf.length, PROT_READ, MAP_SHARED, mDevFp,
                           v4l2buf.m.offset);
    if (virtAddr == MAP_FAILED) {
      ALOGE("%s(%d): Unable to map buffer(length:0x%x offset:0x%x) %s(err:%d)\n", __FUNCTION__, __LINE__, v4l2buf.length, v4l2buf.m.offset, strerror(errno), errno);
      return false;
    } else {
      buffer->setVirtAddr((void*)virtAddr);
      buffer->setCapacity(v4l2buf.length);
    }
    return true;
  } else
    return true;

}
bool V4L2DevIoctr::memUnmap(shared_ptr<BufferBase>& buffer) {
  if (V4L2_MEMORY_MMAP == mV4L2MemType) {
    if (munmap(buffer->getVirtAddr(), buffer->getCapacity()) < 0) {
      ALOGE("%s(%d):  munmap failed : %s", __FUNCTION__, __LINE__, strerror(errno));
      return false;
    } else
      return true;
  } else
    return true;
}

int V4L2DevIoctr::enumInput(v4l2_input* input) {
  if (ioctl(mDevFp, VIDIOC_ENUMINPUT, input) != 0) {
    LOGE("ERR(%s):No matching index found\n", __func__);
    return -1;
  }
  LOGV("Name of input channel[%d] is %s\n", input->index, input->name);

  return 0;
}


int V4L2DevIoctr::setInput(int index) {
  struct v4l2_input input;
  int ret;

  input.index = index;

  ret = ioctl(mDevFp, VIDIOC_S_INPUT, &input);
  if (ret < 0) {
    LOGE("ERR(%s):VIDIOC_S_INPUT failed\n", __func__);
    return ret;
  }
  return ret;
}

int V4L2DevIoctr::queryCtrl(struct v4l2_queryctrl* ctr) {
  int ret = 0;
  ret = ioctl(mDevFp, VIDIOC_QUERYCTRL, ctr);

  if (ret < 0) {
    LOGV("ERR(%s): VIDIOC_QUERYCTRL(id = 0x%x) failed, ret = %d\n",
         __func__, ctr->id, ret);
    return ret;
  }

  return ret;
}

int V4L2DevIoctr::queryMenu(struct v4l2_querymenu* menu) {
  int ret = 0;
  ret = ioctl(mDevFp, VIDIOC_QUERYMENU, menu);

  if (ret < 0) {
    LOGV("ERR(%s): VIDIOC_QUERYMENU(id = 0x%x index %d) failed, ret = %d\n",
         __func__, menu->id, menu->index, ret);
    return ret;
  }

  return ret;
}
int V4L2DevIoctr::getCtrl(unsigned int id) {
  struct v4l2_control ctrl;
  int ret;

  ctrl.id = id;

  ret = ioctl(mDevFp, VIDIOC_G_CTRL, &ctrl);

  if (ret < 0) {
    LOGV("ERR(%s): VIDIOC_G_CTRL(id = 0x%x (%d)) failed, ret = %d\n",
         __func__, id, id - V4L2_CID_PRIVATE_BASE, ret);
    return ret;
  }

  return ctrl.value;
}

int V4L2DevIoctr::setCtrl(unsigned int id, unsigned int value) {
  struct v4l2_control ctrl;
  int ret;

  ctrl.id = id;
  ctrl.value = value;
  ret = ioctl(mDevFp, VIDIOC_S_CTRL, &ctrl);
  if (ret < 0) {
    LOGE("ERR(%s):VIDIOC_S_CTRL(id = %#x (%d), value = %d) failed ret = %d\n",
         __func__, id, id - V4L2_CID_PRIVATE_BASE, value, ret);
  }

  return ret;
}

int V4L2DevIoctr::getSensorModeData(
    struct isp_supplemental_sensor_mode_data* data) {
  int ret = 0;

  ret = ioctl(mDevFp, RK_VIDIOC_SENSOR_MODE_DATA, data);

  if (ret < 0)
    LOGE("ERR(%s): RK_VIDIOC_SENSOR_MODE_DATA failed, err: %s \n",
         __func__, strerror(errno));

  return ret;
}
int V4L2DevIoctr::getCameraModuleInfo(struct camera_module_info_s* camera_module) {
  int ret = 0;

  ret = ioctl(mDevFp, RK_VIDIOC_CAMERA_MODULEINFO, camera_module);
  if (ret < 0) {
    LOGE("ERR(%s):  RK_VIDIOC_CAMERA_MODULEINFO(0x%x) failed, err: %s\n",
         __func__,
         RK_VIDIOC_CAMERA_MODULEINFO,
         strerror(errno));
  }

  return ret;
}
int V4L2DevIoctr::getSensorConfigInfo(struct sensor_config_info_s* sensor_config) {
  int ret = 0;

  ret = ioctl(mDevFp, RK_VIDIOC_SENSOR_CONFIGINFO, sensor_config);
  if (ret < 0) {
    LOGE("ERR(%s):  RK_VIDIOC_SENSOR_CONFIGINFO(0x%x) failed, err: %s\n",
         __func__,
         RK_VIDIOC_SENSOR_CONFIGINFO,
         strerror(errno));
  }

  return ret;
}
int V4L2DevIoctr::accessSensor(struct sensor_reg_rw_s* sensor_rw) {
  int ret = 0;

  ret = ioctl(mDevFp, RK_VIDIOC_SENSOR_REG_ACCESS, sensor_rw);
  if (ret < 0) {
    LOGE("ERR(%s):  RK_VIDIOC_SENSOR_CONFIGINFO(0x%x) failed, err: %s\n",
         __func__,
         RK_VIDIOC_SENSOR_CONFIGINFO,
         strerror(errno));
  }

  return ret;
}

//param
int V4L2DevIoctr::setStrmPara(struct v4l2_streamparm* parm) {
  int ret;
  parm->type = mV4L2BufferType;
  ret = ioctl(mDevFp, VIDIOC_S_PARM, &parm);
  if (ret < 0) {
    LOGE("ERR(%s):VIDIOC_S_PARM failed ret = %d\n",
         __func__, ret);
  }

  return ret;
}

int V4L2DevIoctr::getStrmPara(struct v4l2_streamparm* parm) {
  int ret;
  parm->type = mV4L2BufferType;
  ret = ioctl(mDevFp, VIDIOC_G_PARM, &parm);
  if (ret < 0) {
    LOGE("ERR(%s):VIDIOC_G_PARM failed ret = %d\n",
         __func__, ret);
  }

  return ret;
}

//extctrls
int V4L2DevIoctr::setExtCtrls(struct v4l2_ext_control* ctrls, unsigned int ctrClass, int ctrNum) {
  int ret = 0;

  struct v4l2_ext_controls extctrls;

  extctrls.count = ctrNum;
  extctrls.ctrl_class = ctrClass;
  extctrls.controls = ctrls;
  extctrls.reserved[0] = 0;
  extctrls.reserved[1] = 0;

  ret = ioctl(mDevFp, VIDIOC_S_EXT_CTRLS, &extctrls);

  if (ret < 0) {
    LOGE("ERR(%s) failed\n", __func__);
    return ret;
  }

  return ret;
}
//query all cap infos
int V4L2DevIoctr::queryCapInfo(struct v4l2_capability* cap) {
  int ret = 0;

  ret = ioctl(mDevFp, VIDIOC_QUERYCAP, cap);

  if (ret < 0) {
    LOGE("ERR(%s):VIDIOC_QUERYCAP failed\n", __func__);
    return -1;
  }
  return ret;
}
//query bus info
int V4L2DevIoctr::queryBusInfo(unsigned char* busInfo) {
  struct v4l2_capability cap;
  int ret = 0;
  if (queryCapInfo(&cap) == 0) {
    memcpy(busInfo, cap.bus_info, 32);
  } else
    ret = -1;
  return ret;
}
//query capbilities
int V4L2DevIoctr::queryCap(unsigned int queryCap) {
  struct v4l2_capability cap;
  int ret = 0;
  if (queryCapInfo(&cap) == 0) {
    if (!(cap.capabilities & queryCap)) {
      LOGE("ERR(%s):no capture devices\n", __func__);
      return -1;
    }
  }
  return ret;
}

int V4L2DevIoctr::enumSTD(vector<struct v4l2_standard>& stds) {
  struct v4l2_standard std;
  std.index = 0;

  while (ioctl(mDevFp, VIDIOC_ENUMSTD, &std) == 0) {
    stds.push_back(std);
    std.index++;
  }
  return 0;
}

int V4L2DevIoctr::getSTD(v4l2_std_id& std) {
  return ioctl(mDevFp, VIDIOC_G_STD, &std);
}
int V4L2DevIoctr::setSTD(v4l2_std_id std) {
  return ioctl(mDevFp, VIDIOC_S_STD, &std);
}

int V4L2DevIoctr::queryCropCap(struct v4l2_cropcap& cropCap) {
  //cropCap.bouds: about limit
  //cropCap.defrect : default crop rect
  cropCap.type = mV4L2BufferType;
  return ioctl(mDevFp, VIDIOC_CROPCAP, &cropCap);
}
int V4L2DevIoctr::getCrop(struct v4l2_crop& crop) {
  crop.type = mV4L2BufferType;
  return ioctl(mDevFp, VIDIOC_G_CROP, &crop);
}
int V4L2DevIoctr::setCrop(struct v4l2_crop& crop) {
  crop.type = mV4L2BufferType;
  return ioctl(mDevFp, VIDIOC_S_CROP, &crop);
}


RK_FRMAE_FORMAT V4L2DevIoctr::V4l2FmtToHalFmt(unsigned int v4l2fmt) {
  switch (v4l2fmt) {
    case V4L2_PIX_FMT_NV12 :
      return HAL_FRMAE_FMT_NV12;
    case V4L2_PIX_FMT_NV21 :
      return HAL_FRMAE_FMT_NV21;
    case V4L2_PIX_FMT_YVU420 :
      return HAL_FRMAE_FMT_YVU420;
    case V4L2_PIX_FMT_RGB565 :
      return HAL_FRMAE_FMT_RGB565;
    case V4L2_PIX_FMT_RGB32  :
      return HAL_FRMAE_FMT_RGB32;
    case V4L2_PIX_FMT_YUV422P :
      return HAL_FRMAE_FMT_YUV422P;
    case V4L2_PIX_FMT_NV16 :
      return HAL_FRMAE_FMT_NV16;
    case V4L2_PIX_FMT_YUYV :
      return HAL_FRMAE_FMT_YUYV;
    case V4L2_PIX_FMT_JPEG :
      return HAL_FRMAE_FMT_JPEG;
    case V4L2_PIX_FMT_MJPEG:
      return HAL_FRMAE_FMT_MJPEG;
    case V4L2_PIX_FMT_H264:
      return HAL_FRMAE_FMT_H264;
    case V4L2_PIX_FMT_GREY:
      return HAL_FRMAE_FMT_Y8;
    case V4L2_PIX_FMT_Y10:
      return HAL_FRMAE_FMT_Y10;
    case V4L2_PIX_FMT_Y12:
      return HAL_FRMAE_FMT_Y12;
    case V4L2_PIX_FMT_SBGGR12:
      return HAL_FRMAE_FMT_SBGGR12;
    case V4L2_PIX_FMT_SGBRG12:
      return HAL_FRMAE_FMT_SGBRG12;
    case V4L2_PIX_FMT_SGRBG12:
      return HAL_FRMAE_FMT_SGRBG12;
    case V4L2_PIX_FMT_SRGGB12:
      return HAL_FRMAE_FMT_SRGGB12;
    case V4L2_PIX_FMT_SBGGR10:
      return HAL_FRMAE_FMT_SBGGR10;
    case V4L2_PIX_FMT_SGBRG10:
      return HAL_FRMAE_FMT_SGBRG10;
    case V4L2_PIX_FMT_SGRBG10:
      return HAL_FRMAE_FMT_SGRBG10;
    case V4L2_PIX_FMT_SRGGB10:
      return HAL_FRMAE_FMT_SRGGB10;
    case V4L2_PIX_FMT_SBGGR8:
      return HAL_FRMAE_FMT_SBGGR8;
    case V4L2_PIX_FMT_SGBRG8:
      return HAL_FRMAE_FMT_SGBRG8;
    case V4L2_PIX_FMT_SGRBG8:
      return HAL_FRMAE_FMT_SGRBG8;
    case V4L2_PIX_FMT_SRGGB8:
      return HAL_FRMAE_FMT_SRGGB8;
    case HAL_FRMAE_FMT_MAX :
    default :
      return HAL_FRMAE_FMT_MAX;
  }
}

unsigned int V4L2DevIoctr::halFmtToV4l2Fmt(unsigned int halFmt) {
  switch (halFmt) {
    case HAL_FRMAE_FMT_NV12 :
      return V4L2_PIX_FMT_NV12;
    case HAL_FRMAE_FMT_NV21 :
      return V4L2_PIX_FMT_NV21;
    case HAL_FRMAE_FMT_YVU420 :
      return V4L2_PIX_FMT_YVU420;
    case HAL_FRMAE_FMT_RGB565 :
      return V4L2_PIX_FMT_RGB565;
    case HAL_FRMAE_FMT_RGB32 :
      return V4L2_PIX_FMT_RGB32;
    case HAL_FRMAE_FMT_YUV422P :
      return V4L2_PIX_FMT_YUV422P;
    case HAL_FRMAE_FMT_NV16 :
      return V4L2_PIX_FMT_NV16;
    case HAL_FRMAE_FMT_YUYV :
      return V4L2_PIX_FMT_YUYV;
    case HAL_FRMAE_FMT_JPEG :
      return V4L2_PIX_FMT_JPEG;
    case HAL_FRMAE_FMT_MJPEG :
      return V4L2_PIX_FMT_MJPEG;
    case HAL_FRMAE_FMT_H264 :
      return V4L2_PIX_FMT_H264;
    case HAL_FRMAE_FMT_Y8 :
      return V4L2_PIX_FMT_GREY;
    case HAL_FRMAE_FMT_Y10 :
      return V4L2_PIX_FMT_Y10;
    case HAL_FRMAE_FMT_Y12 :
      return V4L2_PIX_FMT_Y12;
    case HAL_FRMAE_FMT_SBGGR12 :
      return V4L2_PIX_FMT_SBGGR12;
    case HAL_FRMAE_FMT_SGBRG12 :
      return V4L2_PIX_FMT_SGBRG12;
    case HAL_FRMAE_FMT_SGRBG12 :
      return V4L2_PIX_FMT_SGRBG12;
    case HAL_FRMAE_FMT_SRGGB12 :
      return V4L2_PIX_FMT_SRGGB12;
    case HAL_FRMAE_FMT_SBGGR10 :
      return V4L2_PIX_FMT_SBGGR10;
    case HAL_FRMAE_FMT_SGBRG10 :
      return V4L2_PIX_FMT_SGBRG10;
    case HAL_FRMAE_FMT_SGRBG10 :
      return V4L2_PIX_FMT_SGRBG10;
    case HAL_FRMAE_FMT_SRGGB10 :
      return V4L2_PIX_FMT_SRGGB10;
    case HAL_FRMAE_FMT_SBGGR8 :
      return V4L2_PIX_FMT_SBGGR8;
    case HAL_FRMAE_FMT_SGBRG8 :
      return V4L2_PIX_FMT_SGBRG8;
    case HAL_FRMAE_FMT_SGRBG8 :
      return V4L2_PIX_FMT_SGRBG8;
    case HAL_FRMAE_FMT_SRGGB8 :
      return V4L2_PIX_FMT_SRGGB8;
    case HAL_FRMAE_FMT_MAX :
    default :
      return 0;
  }
}

unsigned int V4L2DevIoctr::halColorSpaceToV4l2ColorSpace(unsigned int halColorSpace) {
  switch (halColorSpace) {
    case HAL_COLORSPACE_SMPTE170M :
      return V4L2_COLORSPACE_SMPTE170M;
    case HAL_COLORSPACE_JPEG :
      return V4L2_COLORSPACE_JPEG;
    default :
      return 0;
  }
}

unsigned int V4L2DevIoctr::bytesPerLine(unsigned int v4l2PixFmt, unsigned int width) {
  unsigned int bpl = (unsigned int) - EINVAL;

  switch (v4l2PixFmt) {
    case V4L2_PIX_FMT_NV12:
    case V4L2_PIX_FMT_NV21:
    case V4L2_PIX_FMT_YVU420:
    case V4L2_PIX_FMT_NV16:
    case V4L2_PIX_FMT_YUV422P:
      /* For YcbCr (semi)planar formats the v4l2 manual says that */
      /* the bytes per line refers to the biggest plane; in this case */
      /* the Y plane, which has 1 byte per pixel */
      bpl = width;
      break;
    case V4L2_PIX_FMT_RGB565:
    case V4L2_PIX_FMT_YUYV:
      /* 2 bytes per pixel */
      bpl = width * 2;
      break;
    case V4L2_PIX_FMT_RGB32:
      /* 4 bytes per pixel */
      bpl = width * 4;
      break;
    case V4L2_PIX_FMT_JPEG:
    case V4L2_PIX_FMT_MJPEG:
    case V4L2_PIX_FMT_H264:
      /* not used */
      bpl = 0;
      break;
    case V4L2_PIX_FMT_SBGGR10:
    case V4L2_PIX_FMT_SGBRG10:
    case V4L2_PIX_FMT_SGRBG10:
    case V4L2_PIX_FMT_SRGGB10:
    case V4L2_PIX_FMT_SBGGR12:
    case V4L2_PIX_FMT_SGBRG12:
    case V4L2_PIX_FMT_SGRBG12:
    case V4L2_PIX_FMT_SRGGB12:
      bpl = width * 2;
      break;
    case V4L2_PIX_FMT_SBGGR8:
    case V4L2_PIX_FMT_SGBRG8:
    case V4L2_PIX_FMT_SGRBG8:
    case V4L2_PIX_FMT_SRGGB8:
      bpl = width;
      break;
    case V4L2_PIX_FMT_Y10:
      bpl = width * 2;
      break;
    case V4L2_PIX_FMT_Y12:
      bpl = width * 2;
      break;
    case V4L2_PIX_FMT_GREY:
      bpl = width;
      break;
    default:
      ALOGE("%s: Unsupported V4L2 pixel format %c%c%c%c\n", __func__,
            v4l2PixFmt & 0xFF, (v4l2PixFmt >> 8) & 0xFF, (v4l2PixFmt >> 16) & 0xFF, (v4l2PixFmt >> 24) & 0xFF);
      break;
  }

  return bpl;
}
unsigned int V4L2DevIoctr::bpp(unsigned int v4l2PixFmt) {
  unsigned int depth = (unsigned int) - EINVAL;

  switch (v4l2PixFmt) {
    case V4L2_PIX_FMT_NV12:
    case V4L2_PIX_FMT_NV21:
    case V4L2_PIX_FMT_YVU420:
      depth = 12;
      break;
    case V4L2_PIX_FMT_RGB565:
    case V4L2_PIX_FMT_YUYV:
    case V4L2_PIX_FMT_NV16:
    case V4L2_PIX_FMT_YUV422P:
      depth = 16;
      break;
    case V4L2_PIX_FMT_RGB32:
      depth = 32;
      break;
    case V4L2_PIX_FMT_JPEG:
    case V4L2_PIX_FMT_MJPEG:
    case V4L2_PIX_FMT_H264:
      depth = 8;
      break;
    case V4L2_PIX_FMT_SBGGR10:
    case V4L2_PIX_FMT_SGBRG10:
    case V4L2_PIX_FMT_SGRBG10:
    case V4L2_PIX_FMT_SRGGB10:
    case V4L2_PIX_FMT_SBGGR12:
    case V4L2_PIX_FMT_SGBRG12:
    case V4L2_PIX_FMT_SGRBG12:
    case V4L2_PIX_FMT_SRGGB12:
      depth = 16;
      break;
    case V4L2_PIX_FMT_SBGGR8:
    case V4L2_PIX_FMT_SGBRG8:
    case V4L2_PIX_FMT_SGRBG8:
    case V4L2_PIX_FMT_SRGGB8:
      depth = 8;
      break;
    case V4L2_PIX_FMT_Y10:
      depth = 16;
      break;
	case V4L2_PIX_FMT_Y12:
      depth = 16;
      break;
    case V4L2_PIX_FMT_GREY:
      depth = 8;
      break;
    default:
      // ALOGE("%s: Unsupported V4L2 pixel format %c%c%c%c\n", __func__,
      //       v4l2PixFmt & 0xFF, (v4l2PixFmt >> 8) & 0xFF, (v4l2PixFmt >> 16) & 0xFF, (v4l2PixFmt >> 24) & 0xFF);
      break;
  }

  return depth;
}

bool V4L2ISPDevIoctr::getBufferMetaData(int index, struct HAL_Buffer_MetaData* metaData) {
  if ((index >= 0) && (index < MAX_ISP_META_DATA_BUF_NUM)) {
    metaData->metedata_drv = mMetaDataBuf[index] ;
    return true;
  } else
    return false;
}

bool V4L2ISPDevIoctr::memMap(shared_ptr<BufferBase>& buffer) {
  char* virtAddr = NULL;
  if (mMaped == 0) {
    virtAddr = (char*)mmap(0 /* start anywhere */,
                           mReqBufCnt * CAMERA_METADATA_LEN,
                           PROT_READ | PROT_WRITE, MAP_SHARED, mDevFp,
                           0);
    if (virtAddr == MAP_FAILED) {
      ALOGE("%s(%d): Unable to map buffer(length:0x%x offset:0x%x) %s(err:%d)\n",
            __FUNCTION__, __LINE__, CAMERA_METADATA_LEN,
            0, strerror(errno), errno);
      return false;
    }
    mMaped++;
    mMetaDataAddr = virtAddr;
    mMetaDataBuf[buffer->getIndex()] = mMetaDataAddr + buffer->getIndex() * CAMERA_METADATA_LEN;
  } else {
    mMaped++;
    mMetaDataBuf[buffer->getIndex()] = mMetaDataAddr + buffer->getIndex() * CAMERA_METADATA_LEN;
  }

  return true;

}
bool V4L2ISPDevIoctr::memUnmap(shared_ptr<BufferBase>& buffer) {
  if (mMaped <= 0)
    return true;
  if (--mMaped == 0) {

    if (munmap(mMetaDataAddr, mReqBufCnt * CAMERA_METADATA_LEN) < 0) {
      ALOGE("%s(%d):  munmap failed : %s", __FUNCTION__, __LINE__, strerror(errno));
      return false;
    } else {

      mMetaDataBuf[buffer->getIndex()] = NULL;
      return true;
    }
  }
  return true;
}


