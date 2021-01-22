#ifndef _CAM_CIF_DEV_HW_ITF_IMC_H_
#define _CAM_CIF_DEV_HW_ITF_IMC_H_
#include "CamHwItf.h"

using namespace std;


#define CAMERAHAL_VIDEODEV_NONBLOCK

class CamCifDevHwItf: public CamHwItf {
 public:
  CamCifDevHwItf(struct rk_cif_dev_info* cif_dev = NULL);
  virtual ~CamCifDevHwItf(void);
  //if it is a Cif device
  virtual shared_ptr<CamHwItf::PathBase> getPath(enum CamHwItf::PATHID id);
  virtual bool initHw(int inputId);
  virtual void deInitHw();

  class Path: public CamHwItf::PathBase {
    friend class CamCifDevHwItf;
   public:
    virtual bool prepare(
        frm_info_t& frmFmt,
        unsigned int numBuffers,
        CameraBufferAllocator& allocator,
        bool cached,
        unsigned int minNumBuffersQueued = 1);

    virtual bool prepare(
        frm_info_t& frmFmt,
        list<shared_ptr<BufferBase> >& bufPool,
        unsigned int numBuffers,
        unsigned int minNumBuffersQueued = 1);

    virtual void addBufferNotifier(NewCameraBufferReadyNotifier* bufferReadyNotifier);
    virtual bool removeBufferNotifer(NewCameraBufferReadyNotifier* bufferReadyNotifier);
    virtual void releaseBuffers(void);
    virtual bool start(void);
    virtual void stop(void);
    virtual bool releaseBufToOwener(weak_ptr<BufferBase> camBuf);
    Path(V4L2DevIoctr* camDev, PATHID pathID, unsigned long dequeueTimeout = 1000);
    virtual ~Path(void);
   private:
  };
 private:
  struct rk_cif_dev_info* mCifDev;
};

#endif

