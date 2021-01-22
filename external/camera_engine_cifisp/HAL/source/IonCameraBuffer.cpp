
#include "IonCameraBuffer.h"
#include "cam_ia_api/cam_ia10_trace.h"


using namespace std;

IonCameraBuffer::IonCameraBuffer(
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
    weak_ptr<ICameraBufferOwener> bufOwener):
  CameraBuffer(allocator, bufOwener), mHandle(handle), mShareFd(sharefd),
  mPhy(phy), mVaddr(vaddr), mWidth(width), mHeight(height), mBufferSize(size),
  mCamPixFmt(camPixFmt), mStride(stride) {
  //ALOGV("%s: sharefd %d, vaddr %p, camPixFmt %s, stride %d", __func__, sharefd, vaddr, camPixFmt, stride);

}

IonCameraBuffer::~IonCameraBuffer(void) {
  if (!mAllocator.expired()) {
	
	  shared_ptr<CameraBufferAllocator> spAlloc = mAllocator.lock();

	  if (spAlloc.get()) {
		cout<<"line "<<__LINE__<<" func:"<<__func__<<"\n";
		spAlloc->free(this);
		}
  }
}

void* IonCameraBuffer::getHandle(void) const {
  return (void*)&mHandle;
}
void* IonCameraBuffer::getPhyAddr(void) const {
  return (void*)mPhy;
}

void* IonCameraBuffer::getVirtAddr(void) const {
  return mVaddr;
}

const char* IonCameraBuffer::getFormat(void) {
  return mCamPixFmt;
}

unsigned int IonCameraBuffer::getWidth(void) {
  return mWidth;
}

unsigned int IonCameraBuffer::getHeight(void) {
  return mHeight;
}

size_t IonCameraBuffer::getDataSize(void) const {
  return mDataSize;
}

void IonCameraBuffer::setDataSize(size_t size) {
  mDataSize = size;
}

size_t IonCameraBuffer::getCapacity(void) const {
  return mBufferSize;
}

unsigned int IonCameraBuffer::getStride(void) const {
  return mStride;
}

bool IonCameraBuffer::lock(unsigned int usage) {
  UNUSED_PARAM(usage);
  //ALOGV("%s", __func__);
  return true;
}

bool IonCameraBuffer::unlock(unsigned int usage) {
  UNUSED_PARAM(usage);
  //ALOGV("%s", __func__);

  return true;
}

IonCameraBufferAllocator::IonCameraBufferAllocator(void) {
  //ALOGD("%s", __func__);

  mIonClient = ion_open();
  if (mIonClient < 0) {
    ALOGE("open /dev/ion failed!\n");
    mError = true;
  }
}

IonCameraBufferAllocator::~IonCameraBufferAllocator(void) {
  //ALOGD("%s", __func__);
  ion_close(mIonClient);
  if (mNumBuffersAllocated > 0) {
    ALOGE("%s: memory leak; %d camera buffers have not been freed", __func__, mNumBuffersAllocated);
  }
}

shared_ptr<CameraBuffer> IonCameraBufferAllocator::alloc(
    const char* camPixFmt,
    unsigned int width,
    unsigned int height,
    unsigned int usage,
    weak_ptr<ICameraBufferOwener> bufOwener) {
  int ret;
  ion_user_handle_t buffHandle;
  int stride;
  int sharefd;
  void* vaddr;
  size_t buffer_size;
  shared_ptr<IonCameraBuffer> camBuff;
  unsigned int ionFlags = 0;
  unsigned int halPixFmt;
  unsigned long phy = 0;
  int type = ION_HEAP_TYPE_DMA_MASK;

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
  ret = ion_alloc(mIonClient, buffer_size, 0, type, 0, &buffHandle);
  ret = ion_share(mIonClient, buffHandle, &sharefd);
  if (ret != 0) {
    ALOGE("%s: ion buffer allocation failed (error %d)", __func__, ret);
    goto alloc_end;
  }

  if (type == ION_HEAP_TYPE_DMA_MASK)
    ion_get_phys(mIonClient, buffHandle, &phy);

  //ret = ion_map(mIonClient, buffHandle, buffer_size, ionFlags, MAP_SHARED, 0, (unsigned char **)&vaddr, &map_fd);
  vaddr = mmap(NULL, buffer_size, ionFlags, MAP_SHARED, sharefd, 0);

  //ALOGD("alloc: handle %d, shared_fd %d, vaddr %p, size %d", buffHandle, sharefd, vaddr, buffer_size);
  camBuff = shared_ptr<IonCameraBuffer> (new IonCameraBuffer(buffHandle, sharefd, phy, vaddr, camPixFmt, width, height, stride,
                                                             buffer_size, shared_from_this(), bufOwener));
  if (!camBuff.get()) {
    ALOGE("%s: Out of memory", __func__);
  } else {
    mNumBuffersAllocated++;
    if (camBuff->error()) {
    }
  }
alloc_end:
  return camBuff;
}

void IonCameraBufferAllocator::free(CameraBuffer* buffer) {
  int ret;
  if (buffer) {
    IonCameraBuffer* camBuff = static_cast<IonCameraBuffer*>(buffer);
    //ALOGD("free: index %d, handle %d, shared_fd %d, vaddr %p, size %d",
    //buffer->getIndex(), camBuff->mHandle, camBuff->mShareFd, camBuff->mVaddr, camBuff->mBufferSize);
    munmap(camBuff->mVaddr, camBuff->mBufferSize);
    close(camBuff->mShareFd);
    ret = ion_free(mIonClient, camBuff->mHandle);
    if (ret != 0) {
      ALOGE("%s: ion free buffer failed (error %d)", __func__, ret);
    }
    mNumBuffersAllocated--;
  }
}
