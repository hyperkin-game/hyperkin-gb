#ifndef _CAMERA_ISPTUNNING_H_
#define _CAMERA_ISPTUNNING_H_
#include "CamHwItf.h"
#include "StrmPUBase.h"

#ifdef RK_ISP10
#define RK_ISP_TUNNING_FILE_PATH "/etc/capcmd.xml"
#else
#define RK_ISP_TUNNING_FILE_PATH "/tmp/capcmd.xml"
#endif
#define RK_ISP_TUNNING_IMAGE_STORE_PATH "/mnt/sdcard/isptune/"

using namespace std;


typedef enum EXPOSUSE_MODE_ENUM {
  EXPOSUSE_MODE_MANUAL = 1,
  EXPOSUSE_MODE_AUTO  = 2,
  EXPOSUSE_MODE_INVALID = 3
} EXPOSUSE_MODE_e;

typedef enum WHITEBALANCE_MODE_ENUM {
  WHITEBALANCE_MODE_MANUAL = 1,
  WHITEBALANCE_MODE_AUTO  = 2,
  WHITEBALANCE_MODE_INVALID  = 3,
} WHITEBALANCE_MODE_e;

typedef struct ispTuneTaskInfo_t {
//from xml
  bool mTuneEnable;
  int mTuneWidth;
  int mTuneHeight;
  int mTuneFmt;
  int mTunePicNum;
  struct {
    EXPOSUSE_MODE_e exposuseMode;
    float    integrationTime;
    float    gain;
    float    integrationTimeStep;
    float    gainStep;
    int    minRaw;
    int    maxRaw;
    int    threshold;
    bool   aeRound;
    int    number;
  } mExpose;

  struct {
    WHITEBALANCE_MODE_e whiteBalanceMode;
    char     illumination[10];
    char     cc_matrix[15];
    char     cc_offset[10];
    char     rggb_gain[10];

  } mWhiteBalance;

  bool mWdrEnable;
  bool mCacEnable;
  bool mGammarEnable;
  bool mLscEnable;
  bool mDpccEnable;
  bool mBlsEnable;
  bool mAdpfEnable;
  bool mAvsEnable;
  bool mAfEnable;

//from ..
  bool mForceRGBOut;
  unsigned long y_addr;
  unsigned long uv_addr;
} ispTuneTaskInfo_s;


class CameraIspTunning: public StreamPUBase {
 public:

  ~CameraIspTunning();
  static CameraIspTunning* createInstance(CamHwItf* camHw);
  static CameraIspTunning* createInstance(CamHwItf* camHw, struct HAL_ISP_Cap_Req_s* cap_req);
  //from pu, the last process unit ,so do nothing
  virtual void addBufferNotifier(NewCameraBufferReadyNotifier* bufferReadyNotifier) {
    UNUSED_PARAM(bufferReadyNotifier);
    return;
  }
  virtual bool removeBufferNotifer(NewCameraBufferReadyNotifier* bufferReadyNotifier) {
    UNUSED_PARAM(bufferReadyNotifier);
    return true;
  }
  //from ICameraBufferOwener, this pu havn't alloc buffers,so do thing
  virtual bool releaseBufToOwener(weak_ptr<BufferBase> camBuf) {
    UNUSED_PARAM(camBuf);
    return true;
  }
  //need process result frame buffer
  virtual bool prepare(
      const frm_info_t& frmFmt,
      unsigned int numBuffers,
      shared_ptr<CameraBufferAllocator> allocator);
  virtual bool start();
  virtual void stop();

  virtual bool processFrame(shared_ptr<BufferBase> inBuf, shared_ptr<BufferBase> outBuf);
 private:
  class taskThread;
  friend class taskThread;
  class taskThread : public CamThread {
   public:
    taskThread(CameraIspTunning* owner): mOwner(owner) {};
    virtual bool threadLoop(void) { return mOwner->taskThLoop();}
   private:
    CameraIspTunning* mOwner;
  };

  bool taskThLoop();

  CameraIspTunning() {};
  CameraIspTunning(CamHwItf* camHw);
  static void ConvertYCbCr444combToRGBcomb
  (
      uint8_t*     pYCbCr444,
      uint32_t    PlaneSizePixel
  );
  static void StartElementHandler(void* userData, const char* name, const char** atts);
  static int ispTuneDesiredExp(long raw_ddr, int width, int height, int min_raw, int max_raw, int threshold);

  static int ispTuneStoreBufferRAW
  (
      ispTuneTaskInfo_s*    pIspTuneTaskInfo,
      FILE*                pFile,
      BufferBase*       pBuffer,
      bool              putHeader,
      bool              is16bit
  );

  static int ispTuneStoreBufferYUV422Semi
  (
      ispTuneTaskInfo_s*    pIspTuneTaskInfo,
      FILE*                pFile,
      BufferBase*       pBuffer,
      bool              putHeader
  );

  static int ispTuneStoreBuffer
  (
      ispTuneTaskInfo_s*    pIspTuneTaskInfo,
      BufferBase*       pBuffer,
      char*     szNmae,
      int      index
  );
 public:
  vector<ispTuneTaskInfo_s*> mTuneInfoVector;
 private:
  int mTuneTaskcount;
  int mCurTunIndex;
  float mCurIntegrationTime;
  float mCurGain;
  int mCurAeRoundNum;
  int mSkipFrmNum;
  frm_info_t mCurFmt;
  int mCurCapNum;
  int mCurCapIdx;
  shared_ptr<CameraBufferAllocator> mAlloc;
  osEvent mCurTaskOverEvt;
  osEvent mCurAllTaskOverEvt;
  bool mTaskThRunning;
  osMutex mTaskThMutex;
  shared_ptr<CamThread> mTaskThread;
  CamHwItf* mCamHw;

  static char mStorePath[HAL_ISP_STORE_PATH_LEN+1];
};


#endif

