#ifndef _CAM_HW_ITF_H_
#define _CAM_HW_ITF_H_

#include <memory>
#include <vector>
#include <list>
#include "cam_types.h"
#include "CameraBuffer.h"
#include "CamThread.h"
#include "CamIspCtrItf.h"
#include "camapi_interface.h"
//#include "V4L2DevIoctr.h"

using namespace std;

class NewCameraBufferReadyNotifier;
class V4L2DevIoctr;

enum RK_CAM_ATTACHED_TO_e {
  RK_CAM_ATTACHED_TO_INVALID,
  RK_CAM_ATTACHED_TO_ISP,
  RK_CAM_ATTACHED_TO_CIF,
  RK_CAM_ATTACHED_TO_USB
};

struct rk_cam_video_input_infos {
  int index;
  char name[30];
  void* dev;
  RK_CAM_ATTACHED_TO_e type;
};

#define RK_ENUM_VIDEO_NUM_MAX 10

#define RK_CAM_ATTRACED_INUPUT_MAX 3
struct rk_cam_video_node {
  char card[30]; //
  int video_index;
  int input_nums;
  struct rk_cam_video_input_infos input[RK_CAM_ATTRACED_INUPUT_MAX];
};

#define ISP_DEV_VIDEO_NODES_NUM_MAX 5
struct rk_isp_dev_info {
  int isp_dev_node_nums;
  struct rk_cam_video_node video_nodes[ISP_DEV_VIDEO_NODES_NUM_MAX];
};

#define CIF_DEV_VIDEO_NODES_NUM_MAX 4
struct rk_cif_dev_info {
  int cif_index;
  struct rk_cam_video_node video_node;
};

struct rk_cif_dev_infos {
  int cif_dev_node_nums;
  struct rk_cif_dev_info cif_devs[CIF_DEV_VIDEO_NODES_NUM_MAX];
};

#define USB_CAM_DEV_VIDEO_NODES_NUM_MAX 2
struct rk_usb_cam_dev_infos {
  int usb_dev_node_nums;
  struct rk_cam_video_node video_nodes[USB_CAM_DEV_VIDEO_NODES_NUM_MAX];
};

#define RK_SUPPORTED_CAMERA_NUM_MAX 10
struct rk_cams_dev_info {
  int num_camers;
  struct rk_cam_video_input_infos*  cam[RK_SUPPORTED_CAMERA_NUM_MAX];
  struct rk_isp_dev_info isp_dev;
  struct rk_cif_dev_infos cif_devs;
  struct rk_usb_cam_dev_infos usb_devs;
};

class CamHwItf : public CamApiItf, virtual public enable_shared_from_this<CamHwItf> {
 public:
  enum PATHID {
    MP,
    SP,
    DMA,
    Y12
  };
  enum Input {
    INP_CSI0,
    INP_CSI1,
    INP_CPI,
    INP_DMA,
    INP_DMA_IE,
    INP_DMA_SP,
  };

  static int getCameraInfos(struct rk_cams_dev_info* cam_infos);

  CamHwItf(void);
  virtual ~CamHwItf(void) {};
  //data path
  class PathBase: public ICameraBufferOwener, public enable_shared_from_this<PathBase> {
    friend class CamHwItf;
   public:
    enum State {
      UNINITIALIZED,
      PREPARED,
      STREAMING
    };

    virtual bool prepare(
        frm_info_t& frmFmt,
        unsigned int numBuffers,
        CameraBufferAllocator& allocator,
        bool cached,
        unsigned int minNumBuffersQueued = 1) = 0;

    virtual bool prepare(
        frm_info_t& frmFmt,
        list<shared_ptr<BufferBase> >& bufPool,
        unsigned int numBuffers,
        unsigned int minNumBuffersQueued = 1) = 0;

    virtual void addBufferNotifier(NewCameraBufferReadyNotifier* bufferReadyNotifier) = 0;
    virtual bool removeBufferNotifer(NewCameraBufferReadyNotifier* bufferReadyNotifier) = 0;
    virtual bool setInput(Input input) const;
    virtual void releaseBuffers(void) = 0;
    virtual bool start(void) = 0;
    virtual void stop(void) = 0;
    virtual unsigned int getMinNumUndequeueableBuffers(void) const;
    virtual unsigned int getNumQueuedBuffers(void) const;
    virtual bool releaseBufToOwener(weak_ptr<BufferBase> camBuf) = 0;
    virtual ~PathBase(void);
   protected:
    PathBase(CamHwItf* camHw, V4L2DevIoctr* camDev, PATHID pathID, unsigned long dequeueTimeout = 1000);

    bool preDumpName(shared_ptr<BufferBase>& pBuffer, struct rk_dump_ctl &dump_ctl, uint32_t idx);
    bool dumpFrame(shared_ptr<BufferBase>& pBuffer, char *szFileName);
    bool dequeueFunc(void);

    V4L2DevIoctr* mCamDev;
    const PATHID mPathID;
    unsigned long mDequeueTimeout;
    State mState;
    unsigned int mNumBuffers;
    unsigned int mNumBuffersQueued;
    mutable osMutex mNumBuffersQueuedLock;
    mutable osMutex mPathLock;
    mutable osMutex mBufLock;
    unsigned int mMinNumBuffersQueued;
    unsigned int mNumBuffersUndequeueable;
    osEvent mBufferQueued;
    mutable osMutex mNotifierLock;
    list<NewCameraBufferReadyNotifier*> mBufferReadyNotifierList;
    CameraBufferAllocator* mBufferAllocator;
    list<shared_ptr<BufferBase> > mBufferPool;
    unsigned int mSkipFrames;
    int mPathRefCnt;
    CamHwItf* mCamHw;
    unsigned int mDumpNum;
    unsigned int mDumpCnt;
    frm_info_t mFmtInfo;
    char mDumpFileName[FILENAME_MAX];
    list<shared_ptr<BufferBase> > mDumpBufList;
    class DequeueThread : public CamThread {
     public:
      DequeueThread(PathBase* path): mPath(path) {};
      virtual bool threadLoop(void) {return mPath->dequeueFunc();};
     private:
      PathBase* mPath;
    };
    shared_ptr<DequeueThread> mDequeueThread;

  };
  virtual shared_ptr<PathBase> getPath(enum PATHID id) {
    UNUSED_PARAM(id);
    return shared_ptr<PathBase>(NULL);
  }
  //controls
  virtual bool initHw(int inputId) = 0;
  virtual void deInitHw() = 0;

  //ISP configure
  virtual bool configureISPModules(const void* config) { UNUSED_PARAM(config); return false;};
  virtual int setExposure(unsigned int vts, unsigned int exposure, unsigned int gain, unsigned int gain_percent);
  virtual int setFocusPos(int position);
  //return -1 means unsupported
  virtual int isSupportedIrCut();
  //>0 means IRCUT is working
  virtual int getIrCutState();
  virtual int setIrCutState(int state);
  virtual int tryFormat(frm_info_t inFmt, frm_info_t& outFmt);
  virtual int setAutoAdjustFps(bool auto_adjust_fps);
  virtual int setFlip(int flip);
  virtual int getFlip(int& flip);
 protected:
  virtual void transDrvMetaDataToHal(const void* drvMeta, struct HAL_Buffer_MetaData* halMeta);
  //device ,path
  shared_ptr<PathBase> mSp;
  shared_ptr<PathBase> mMp;
  shared_ptr<PathBase> mDMAPath;
  shared_ptr<PathBase> mY12Path;
  shared_ptr<V4L2DevIoctr> mSpDev;
  shared_ptr<V4L2DevIoctr> mMpDev;
  shared_ptr<V4L2DevIoctr> mDmaPathDev;
  shared_ptr<V4L2DevIoctr> mY12PathDev;
  int m_cam_fd_capture;
  int m_cam_fd_overlay;
  int m_cam_fd_dma;
  int m_cam_fd_y12;
  int m_cam_fd_imgsrc;
  int mInputId;
  bool m_flag_init;

};

class NewCameraBufferReadyNotifier {
 public:
  virtual bool bufferReady(weak_ptr<BufferBase> buffer, int status) = 0;
  virtual ~NewCameraBufferReadyNotifier(void) {}
  frm_info_t& getReqFmt() {return mReqFmt;}
  void setReqFmt(const frm_info_t& reqFmt) {mReqFmt = reqFmt;}
 private:
  frm_info_t mReqFmt;
};

shared_ptr<CamHwItf> getCamHwItf(struct rk_isp_dev_info* isp_dev_info = NULL);

#endif

