#ifndef _DMABUF_CAMERA_BUFFER_H
#define _DMABUF_CAMERA_BUFFER_H

#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "CameraBuffer.h"

typedef struct dma_buf_user_handle_s
{
    int fd;
    int shareFd;
    void *ptr;
    size_t size;
    size_t offset;
    size_t pitch;
    unsigned handle;
} dma_buf_user_handle_t;

class DMABufCameraBuffer : public CameraBuffer
{
    friend class DMABufCameraBufferAllocator;
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

    virtual ~DMABufCameraBuffer();

    virtual int getFd()
    {
        return mShareFd;
    }

protected:
    DMABufCameraBuffer(
        dma_buf_user_handle_t *handle,
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

    dma_buf_user_handle_t *mHandle;
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

class DMABufCameraBufferAllocator : public CameraBufferAllocator
{
    friend class DMABufCameraBuffer;
public:
    DMABufCameraBufferAllocator(void);

    ~DMABufCameraBufferAllocator(void);

    virtual shared_ptr<CameraBuffer> alloc(
        const char* camPixFmt,
        unsigned int width,
        unsigned int height,
        unsigned int usage,
        weak_ptr<ICameraBufferOwener> bufOwener);

    virtual void free(CameraBuffer* buffer);

private:

    int mDMABufClient;
};

#endif

