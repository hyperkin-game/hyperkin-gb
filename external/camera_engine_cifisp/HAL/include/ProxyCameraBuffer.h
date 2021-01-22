/*
Proxy buffers don't alloc memory actually,which just manage
memory allocated outside.for example, USB camera driver allocates
buffers,and these buffers are managed by ProxyCameraBuffer
*/

#ifndef _PROXY_CAMERA_BUFFER_H
#define _PROXY_CAMERA_BUFFER_H

#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "CameraBuffer.h"

class ProxyCameraBufferAllocator;

class ProxyCameraBuffer : public CameraBuffer {
  friend class ProxyCameraBufferAllocator;
 public:
  virtual void* getHandle(void) const;
  virtual void* getPhyAddr(void) const;

  virtual void* getVirtAddr(void) const;
  virtual void setVirtAddr(void*);

  virtual const char* getFormat(void);

  virtual unsigned int getWidth(void);

  virtual unsigned int getHeight(void);

  virtual size_t getDataSize(void) const;

  virtual void setDataSize(size_t size);

  virtual size_t getCapacity(void) const;
  virtual void setCapacity(size_t size);

  virtual unsigned int getStride(void) const;

  virtual bool lock(unsigned int usage = CameraBuffer::READ);

  virtual bool unlock(unsigned int usage = CameraBuffer::READ);

  virtual ~ProxyCameraBuffer();

 protected:
  ProxyCameraBuffer(
      void* vaddr,
      const char* camPixFmt,
      unsigned int width,
      unsigned int height,
      int stride,
      size_t size,
      weak_ptr<CameraBufferAllocator>,
      weak_ptr<ICameraBufferOwener> bufOwener);

  void* mVaddr;
  unsigned int mWidth;
  unsigned int mHeight;
  size_t mBufferSize;
  const char* mCamPixFmt;
  unsigned int mStride;
  size_t mDataSize;
};

class ProxyCameraBufferAllocator : public CameraBufferAllocator {
  friend class ProxyCameraBuffer;
 public:
  ProxyCameraBufferAllocator(void);

  ~ProxyCameraBufferAllocator(void);

  virtual shared_ptr<CameraBuffer> alloc(
      const char* camPixFmt,
      unsigned int width,
      unsigned int height,
      unsigned int usage,
      weak_ptr<ICameraBufferOwener> bufOwener);

  virtual void free(CameraBuffer* buffer);

 private:

};

#endif
