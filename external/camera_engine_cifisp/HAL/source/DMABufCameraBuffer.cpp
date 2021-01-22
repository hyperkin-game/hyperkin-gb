
#include "DMABufCameraBuffer.h"
#include "cam_ia_api/cam_ia10_trace.h"
#include "drm.h"
#include "xf86drm.h"
#include "libdrm_macros.h"

using namespace std;

DMABufCameraBuffer::DMABufCameraBuffer(
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
    weak_ptr<ICameraBufferOwener> bufOwener):
    CameraBuffer(allocator, bufOwener), mHandle(handle), mShareFd(sharefd),
    mPhy(phy), mVaddr(vaddr), mWidth(width), mHeight(height), mBufferSize(size),
    mCamPixFmt(camPixFmt), mStride(stride)
{
    //ALOGV("%s: sharefd %d, vaddr %p, camPixFmt %s, stride %d", __func__, sharefd, vaddr, camPixFmt, stride);

}

DMABufCameraBuffer::~DMABufCameraBuffer(void)
{
	if (!mAllocator.expired()) {
	
	  shared_ptr<CameraBufferAllocator> spAlloc = mAllocator.lock();

	  if (spAlloc.get()) {
		//cout<<"line "<<__LINE__<<" func:"<<__func__<<"\n";
		spAlloc->free(this);
		}
	}
}

void* DMABufCameraBuffer::getHandle(void) const
{
    return (void*)mHandle;
}
void* DMABufCameraBuffer::getPhyAddr(void) const
{
    return (void*)mPhy;
}

void* DMABufCameraBuffer::getVirtAddr(void) const
{
    return mVaddr;
}

const char* DMABufCameraBuffer::getFormat(void)
{
    return mCamPixFmt;
}

unsigned int DMABufCameraBuffer::getWidth(void)
{
    return mWidth;
}

unsigned int DMABufCameraBuffer::getHeight(void)
{
    return mHeight;
}

size_t DMABufCameraBuffer::getDataSize(void) const
{
    return mDataSize;
}

void DMABufCameraBuffer::setDataSize(size_t size)
{
    mDataSize = size;
}

size_t DMABufCameraBuffer::getCapacity(void) const
{
    return mBufferSize;
}

unsigned int DMABufCameraBuffer::getStride(void) const
{
    return mStride;
}

bool DMABufCameraBuffer::lock(unsigned int usage)
{
    UNUSED_PARAM(usage);
    //ALOGV("%s", __func__);
    return true;
}

bool DMABufCameraBuffer::unlock(unsigned int usage)
{
    UNUSED_PARAM(usage);
    //ALOGV("%s", __func__);

    return true;
}

DMABufCameraBufferAllocator::DMABufCameraBufferAllocator(void)
{
    //ALOGD("%s", __func__);
    /*
    mDMABufClient = ion_open();
    if (mDMABufClient < 0)
    {
        ALOGE("open /dev/ion failed!\n");
        mError = true;
    }*/
}

DMABufCameraBufferAllocator::~DMABufCameraBufferAllocator(void)
{
    //ALOGD("%s", __func__);
    /*
    ion_close(mDMABufClient);
    if (mNumBuffersAllocated > 0)
    {
        ALOGE("%s: memory leak; %d camera buffers have not been freed", __func__, mNumBuffersAllocated);
    }*/
}

shared_ptr<CameraBuffer> DMABufCameraBufferAllocator::alloc(
    const char* camPixFmt,
    unsigned int width,
    unsigned int height,
    unsigned int usage,
    weak_ptr<ICameraBufferOwener> bufOwener)
{
    int ret;
    dma_buf_user_handle_t* buffHandle = new dma_buf_user_handle_t;
    int stride;
    size_t buffer_size;
    shared_ptr<DMABufCameraBuffer> camBuff;
    unsigned int ionFlags = 0;
    unsigned int halPixFmt;
    unsigned long phy = 0;
    int type;// = ION_HEAP_TYPE_DMA_MASK;

    //ALOGV("%s: format %s %dx%d, usage 0x%08x", __func__, camPixFmt, width, height, usage);

    /*rk jpeg encoder demand 16 aligned*/
    //width = (width + 0xf) & ~0xf;
    //height = (height + 0xf) & ~0xf;

    if (usage & CameraBuffer::WRITE)
        ionFlags |= PROT_WRITE;
    if (usage & CameraBuffer::READ)
        ionFlags |= PROT_READ;
    //if (usage & CameraBuffer::CACHED)
    //    grallocFlags |= GRALLOC_USAGE_SW_READ_OFTEN; //GRALLOC_USAGE_SW_READ_OFTEN is used by RK gralloc, GRALLOC_USAGE_PRIVATE_3 is used by INTEL
    //if (usage & CameraBuffer::FULL_RANGE)
    //  grallocFlags |= GRALLOC_USAGE_PRIVATE_1;

    buffer_size = calcBufferSize(camPixFmt, (width + 0xf) & ~0xf, (height + 0xf) & ~0xf);
    stride = width;

    // alloc DMA buffer
    buffHandle->fd = open("/dev/dri/card0", O_RDWR, 0);

    struct drm_mode_create_dumb arg_create;
    int handle, size, pitch;
    memset(&arg_create, 0, sizeof(arg_create));
    arg_create.bpp = buffer_size * 8 / (width * height);
    arg_create.width = (width + 0xf) & ~0xf;
    arg_create.height = (height + 0xf) & ~0xf;
    ret = drmIoctl(buffHandle->fd, DRM_IOCTL_MODE_CREATE_DUMB, &arg_create);
    if (ret)
    {
        ALOGE("failed to create dumb buffer: %s\n", strerror(errno));
        return NULL;
    }
    buffHandle->handle = arg_create.handle;
    buffHandle->size = arg_create.size;
    buffHandle->pitch = arg_create.pitch;

    // export fd
    ret = drmPrimeHandleToFD(buffHandle->fd, buffHandle->handle, 0, &buffHandle->shareFd);
    if (ret)
    {
        ALOGE("%s: export fd failed", __func__);
        return NULL;
    }

    // map address
    struct drm_mode_map_dumb arg;
    memset(&arg, 0, sizeof(arg));
    arg.handle = buffHandle->handle;
    ret = drmIoctl(buffHandle->fd, DRM_IOCTL_MODE_MAP_DUMB, &arg);
    if (ret) {
        ALOGE("%s: drm ioctl mode map dumb failed", __func__);
        return NULL;
    }

    buffHandle->ptr = drm_mmap(0, buffHandle->size, PROT_READ | PROT_WRITE, MAP_SHARED, buffHandle->fd, arg.offset);
    if (buffHandle->ptr == MAP_FAILED) {
        ALOGE("%s: drm map failed", __func__);
        return NULL;
    }

    ALOGD("alloc(%d-%d-%d): handle %d, shared_fd %d, vaddr %p, size %d",
          width, height, buffer_size,
          buffHandle, buffHandle->shareFd, buffHandle->ptr, buffHandle->size);
    camBuff = shared_ptr<DMABufCameraBuffer> (new DMABufCameraBuffer(buffHandle, buffHandle->shareFd, phy, buffHandle->ptr, camPixFmt, width, height, stride,
              buffer_size, shared_from_this(), bufOwener));
    if (!camBuff.get())
    {
        ALOGE("%s: Out of memory", __func__);
    }
    else
    {
        mNumBuffersAllocated++;
        if (camBuff->error())
        {
        }
    }
alloc_end:
    return camBuff;
}

void DMABufCameraBufferAllocator::free(CameraBuffer* buffer)
{
    int ret;
    if (buffer)
    {
        DMABufCameraBuffer* camBuff = static_cast<DMABufCameraBuffer*>(buffer);
        //ALOGD("free: index %d, handle %d, shared_fd %d, vaddr %p, size %d",
        //buffer->getIndex(), camBuff->mHandle, camBuff->mShareFd, camBuff->mVaddr, camBuff->mBufferSize);
        munmap(camBuff->mVaddr, camBuff->mBufferSize);
        close(camBuff->mShareFd);

        struct drm_mode_destroy_dumb arg_destroy;
        memset(&arg_destroy, 0, sizeof(arg_destroy));
        dma_buf_user_handle_t *buffHandle = (dma_buf_user_handle_t*)camBuff->getHandle();
        arg_destroy.handle = buffHandle->handle;
        ret = drmIoctl(buffHandle->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &arg_destroy);
        if (ret)
            fprintf(stderr, "failed to destroy dumb buffer: %s\n", strerror(errno));

        struct drm_gem_close args_close;
        memset(&args_close, 0, sizeof(args_close));
        args_close.handle = buffHandle->handle;
        drmIoctl(buffHandle->fd, DRM_IOCTL_GEM_CLOSE, &args_close);

        close(buffHandle->fd);
        if (buffHandle != NULL)
            delete(buffHandle);

        /*
            ret = ion_free(mDMABufClient, camBuff->mHandle);
            if (ret != 0)
            {
                ALOGE("%s: ion free buffer failed (error %d)", __func__, ret);
            }
            */
        mNumBuffersAllocated--;
    }
}
