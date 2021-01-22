#ifndef _ION_CAMERA_BUFFER_H
#define _ION_CAMERA_BUFFER_H

#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <ion/ion.h>
#include "CameraBuffer.h"

class IonCameraBuffer : public CameraBuffer {
  friend class IonCameraBufferAllocator;
 public:
  virtual void* getHandle(void) const;
  virtual void* getPhyAddr(void) const;

  virtual void* getVirtAddr(void) const;

  virtual const char* getFormat(void);

  virtual unsigned int getWidth(void);

  virtual unsigned int getHeight(void);

  virtual size_t getDataSize(void) const;

  virtual void setDataSize(size_t size);

  virtual size_t getCapacity(void) const;

  virtual unsigned int getStride(void) const;

  virtual bool lock(unsigned int usage = CameraBuffer::READ);

  virtual bool unlock(unsigned int usage = CameraBuffer::READ);

  virtual ~IonCameraBuffer();

  virtual int getFd() { return mShareFd;}

 protected:
  IonCameraBuffer(
      ion_user_handle_t handle,
      int sharefd,
      unsigned long phy,
      void* vaddr,
      const char* camPixFmt,
      unsigned int width,
      unsigned int height,
      int stride,
      size_t size,
      weak_ptr<CameraBufferAllocator> allocator,
      weak_ptr<ICameraBufferOwener> bufOwener);

  ion_user_handle_t mHandle;
  int mShareFd;
  void* mVaddr;
  unsigned int mWidth;
  unsigned int mHeight;
  size_t mBufferSize;
  const char* mCamPixFmt;
  unsigned int mStride;
  size_t mDataSize;
  unsigned long mPhy;
};

class IonCameraBufferAllocator : public CameraBufferAllocator {
  friend class IonCameraBuffer;
 public:
  IonCameraBufferAllocator(void);

  ~IonCameraBufferAllocator(void);

  virtual shared_ptr<CameraBuffer> alloc(
      const char* camPixFmt,
      unsigned int width,
      unsigned int height,
      unsigned int usage,
      weak_ptr<ICameraBufferOwener> bufOwener);

  virtual void free(CameraBuffer* buffer);

 private:

  int mIonClient;
};

#endif
