#ifndef __STREAM_PROCESS_UNIT_BASE_H__
#define __STREAM_PROCESS_UNIT_BASE_H__

#include "CamHwItf.h"

using namespace std;

class StreamPUBase
  : public enable_shared_from_this<StreamPUBase>,
    public NewCameraBufferReadyNotifier,
    public ICameraBufferOwener {
  friend class streamThread;
 public:
  enum State {
    UNINITIALIZED,
    PREPARED,
    STREAMING
  };
  StreamPUBase(const char* name = "defaultPU", bool needStrmTh = true, bool shareInbuffer = false);
  virtual ~StreamPUBase();
  virtual void addBufferNotifier(NewCameraBufferReadyNotifier* bufferReadyNotifier);
  virtual bool removeBufferNotifer(NewCameraBufferReadyNotifier* bufferReadyNotifier);

  //from ICameraBufferOwener
  virtual bool releaseBufToOwener(weak_ptr<BufferBase> camBuf);
  //from NewCameraBufferReadyNotifier
  virtual bool bufferReady(weak_ptr<BufferBase> buffer, int status);
  //need process result frame buffer
  virtual bool prepare(
      const frm_info_t& frmFmt,
      unsigned int numBuffers,
      shared_ptr<CameraBufferAllocator> allocator);
  virtual bool start();
  virtual void stop();
  virtual void releaseBuffers(void);
  virtual void invokeFakeNotify(void);

 protected:

  class streamThread : public CamThread {
   public:
    streamThread(StreamPUBase* streamPU): mStreamPU(streamPU) {};
    virtual bool threadLoop(void);
   private:
    StreamPUBase* mStreamPU;
  };
  virtual bool processFrame(shared_ptr<BufferBase> inBuf, shared_ptr<BufferBase> outBuf);
  shared_ptr<CamThread> mStrmPUThread;
  osMutex mNotifierLock;
  list<NewCameraBufferReadyNotifier*> mBufferReadyNotifierList;

  shared_ptr<CameraBufferAllocator> mBufferAllocator;
  list<shared_ptr<BufferBase> > mBufferPool;
  list<shared_ptr<BufferBase> > mInBuffers;
  list<shared_ptr<BufferBase> > mOutBuffers;
  unsigned int mAllocNumBuffers;
  osMutex mBuffersLock;
  osEvent mBuffersCond;

  State mState;
  osMutex mStateLock;
  frm_info_t mStrmFrmInfo;
  bool mNeedStrmTh;
  bool mShareInBuf;
  const char* mName;
};
#endif
