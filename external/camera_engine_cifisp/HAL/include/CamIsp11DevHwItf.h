#ifndef _CAM_ISP11_DEV_HW_ITF_IMC_H_
#define _CAM_ISP11_DEV_HW_ITF_IMC_H_
#include "CamHwItf.h"
#include "CamIspCtrItf.h"
using namespace std;

#define CAMERA_DEVICE_NAME              "/dev/video"
#define CAMERA_CAPTURE_DEV_NAME   "/dev/video2"
#define CAMERA_OVERLAY_DEV_NAME   "/dev/video0"
#define CAMERA_DMA_DEV_NAME   "/dev/video3"
#define CAMERA_ISP_DEV_NAME   "/dev/video1"
#define CAMERA_IQ_FIRST_DIR     "/tmp/"
#define CAMERA_IQ_SECOND_DIR  "/etc/cam_iq/"

#define CAMERAHAL_VIDEODEV_NONBLOCK

class CamIsp11CtrItf;

class CamIsp11DevHwItf: public CamHwItf {
 public:
  CamIsp11DevHwItf(struct rk_isp_dev_info* isp_dev_info = NULL);
  virtual ~CamIsp11DevHwItf(void);
  //derived interfaces from CamHwItf
  virtual shared_ptr<CamHwItf::PathBase> getPath(enum CamHwItf::PATHID id);
  virtual bool initHw(int inputId);
  virtual void deInitHw();

  //ISP dev  inerfaces
  virtual int setExposure(unsigned int vts, unsigned int exposure, unsigned int gain, unsigned int gain_percent);
  virtual int setAutoAdjustFps(bool auto_adjust_fps);
  virtual bool configureISPModules(const void* config);
  //capture
  virtual int startCap(struct HAL_ISP_Cap_Req_s& req);
  virtual int getCapRes(struct HAL_ISP_Cap_Result_s& result);
  //IQPath
  virtual int reInitHW(struct HAL_ISP_Reboot_Req_s& req);

  class Path: public CamHwItf::PathBase {
    friend class CamIsp11DevHwItf;
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
    Path(CamIsp11DevHwItf* camIsp, V4L2DevIoctr* camDev, PATHID pathID, unsigned long dequeueTimeout = 1000);
    virtual ~Path(void);

   private:
    CamIsp11DevHwItf* mCamIsp;

  };

 private:
  virtual void transDrvMetaDataToHal(const void* drvMeta, struct HAL_Buffer_MetaData* halMeta);
  virtual int configIsp(struct isp_supplemental_sensor_mode_data* sensor_mode_data,
      struct sensor_config_info_s* sensor_config, bool enable);

  unsigned int mExposureSequence;

  shared_ptr<CamIsp11CtrItf> mISPDev;
  CamIspCtrItf::Configuration mIspCfg;
  osMutex mApiLock;
  struct rk_isp_dev_info* mISPDevInfo;
  signed char mISPBrightness;
  float mISPContrast;
  float mISPSaturation;
  float mISPHue;
  char mIqPath[64];

  class capThread;
  friend class capThread;
  class capThread : public CamThread {
   public:
    capThread(CamIsp11DevHwItf* owner): mOwner(owner) {};
    virtual bool threadLoop(void) { return mOwner->capLoop();}
   private:
    CamIsp11DevHwItf* mOwner;
  };
  shared_ptr<CamThread> mCapThread;
  bool capLoop();
  bool mCapThdRun;
};

#endif

