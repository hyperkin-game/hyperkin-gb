#ifndef _CAM_USB_DEV_HW_ITF_IMC_H_
#define _CAM_USB_DEV_HW_ITF_IMC_H_
#include "CamHwItf.h"
#include "ProxyCameraBuffer.h"

using namespace std;


#define CAMERAHAL_VIDEODEV_NONBLOCK

class CamUSBDevHwItf: public CamHwItf {
 public:
  CamUSBDevHwItf(struct rk_cam_video_node* usb_video_node = NULL);
  virtual ~CamUSBDevHwItf(void);
  class Path: public CamHwItf::PathBase {
    friend class CamUSBDevHwItf;
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
    shared_ptr<ProxyCameraBufferAllocator> mUSBBufAllocator;
  };
  //if it is a USB device
  virtual shared_ptr<CamHwItf::PathBase> getPath(enum CamHwItf::PATHID id);
  virtual bool initHw(int inputId);
  virtual void deInitHw();

  //3A controls:AE,AWB,AF
  //AE
  virtual int getSupportedAeModes(vector<enum HAL_AE_OPERATION_MODE>& aeModes);
  virtual int setAeMode(enum HAL_AE_OPERATION_MODE aeMode);
  virtual int getAeMode(enum HAL_AE_OPERATION_MODE& aeMode);
  virtual int setAeBias(int aeBias);
  virtual int getAeBias(int& curAeBias);
  virtual int getSupportedBiasRange(HAL_RANGES_t& range);
  virtual int setAbsExp(int exposure);
  virtual int getAbsExp(int& curExposure);
  virtual int getSupportedExpMeterModes(vector<enum HAL_AE_METERING_MODE> modes);
  virtual int setExposureMeterMode(enum HAL_AE_METERING_MODE aeMeterMode);
  virtual int getExposureMeterMode(enum HAL_AE_METERING_MODE& aeMeterMode);
  //fps,anti banding contols,related to ae control
  virtual int setFps(HAL_FPS_INFO_t fps);
  //for mode HAL_AE_OPERATION_MODE_AUTO,
  virtual int enableAe(bool aeEnable);
  virtual int setAntiBandMode(enum HAL_AE_FLK_MODE flkMode);
  virtual int getSupportedAntiBandModes(vector<enum HAL_AE_FLK_MODE>& flkModes);

  //AWB
  virtual int getSupportedWbModes(vector<HAL_WB_MODE>& modes);
  virtual int setWhiteBalance(HAL_WB_MODE wbMode);
  virtual int getWhiteBalanceMode(HAL_WB_MODE& wbMode);
  //for mode HAL_WB_AUTO,
  virtual int enableAwb(bool awbEnable);
  //AF
  virtual int setFocusPos(int position);
  virtual int getFocusPos(int& position);
  virtual int getSupportedFocusModes(vector<enum HAL_AF_MODE> fcModes);
  virtual int setFocusMode(enum HAL_AF_MODE fcMode);
  virtual int getFocusMode(enum HAL_AF_MODE& fcMode);
  virtual int getAfStatus(enum HAL_AF_STATUS& afStatus);
  //for af algorithm ?
  virtual int enableAf(bool afEnable);
  //single AF
  virtual int trigggerAf(bool trigger);
  //3A lock
  virtual int getSupported3ALocks(vector<enum HAL_3A_LOCKS>& locks);
  virtual int set3ALocks(int locks);
  virtual int get3ALocks(int& curLocks);
  //fmts:format , size , fps
  virtual bool enumSensorFmts(vector<frm_info_t>& frmInfos);
  virtual bool enumSupportedFmts(vector<RK_FRMAE_FORMAT>& frmFmts);
  virtual bool enumSupportedSizes(RK_FRMAE_FORMAT frmFmt, vector<frm_size_t>& frmSizes);
  virtual bool enumSupportedFps(RK_FRMAE_FORMAT frmFmt, frm_size_t frmSize, vector<HAL_FPS_INFO_t>& fpsVec);
  virtual int  tryFormat(frm_info_t inFmt, frm_info_t& outFmt);
  //flash control
  virtual int getSupportedFlashModes(vector<enum HAL_FLASH_MODE>& flModes);
  virtual int setFlashLightMode(enum HAL_FLASH_MODE flMode, int intensity, int timeout);
  virtual int getFlashLightMode(enum HAL_FLASH_MODE& flMode);
  //virtual int getFlashStrobeStatus(bool &flStatus);
  //virtual int enableFlashCharge(bool enable);

  //miscellaneous controls:
  //color effect,brightness,contrast,hue,saturation,ISO,scene mode,zoom
  //color effect
  virtual int getSupportedImgEffects(vector<enum HAL_IAMGE_EFFECT>& imgEffs);
  virtual int setImageEffect(enum HAL_IAMGE_EFFECT image_effect);
  virtual int getImageEffect(enum HAL_IAMGE_EFFECT& image_effect);
  //scene
  virtual int getSupportedSceneModes(vector<enum HAL_SCENE_MODE>& sceneModes);
  virtual int setSceneMode(enum HAL_SCENE_MODE sceneMode);
  virtual int getSceneMode(enum HAL_SCENE_MODE& sceneMode);
  //ISO
  virtual int getSupportedISOModes(vector<enum HAL_ISO_MODE>& isoModes);
  virtual int setISOMode(enum HAL_ISO_MODE isoMode, int sens);
  virtual int getISOMode(enum HAL_ISO_MODE& isoMode);
  //zoom
  virtual int getSupportedZoomRange(HAL_RANGES_t& zoomRange);
  virtual int setZoom(int zoomVal);
  virtual int getZoom(int& zoomVal);
  //brightness
  virtual int getSupportedBtRange(HAL_RANGES_t& brightRange);
  virtual int setBrightness(int brightVal);
  virtual int getBrithtness(int& brightVal);
  //contrast
  virtual int getSupportedCtRange(HAL_RANGES_t& contrastRange);
  virtual int setContrast(int contrast);
  virtual int getContrast(int& contrast);
  //saturation
  virtual int getSupportedStRange(HAL_RANGES_t& saturationRange);
  virtual int setSaturation(int sat);
  virtual int getSaturation(int& sat);
  //hue
  virtual int getSupportedHueRange(HAL_RANGES_t& hueRange);
  virtual int setHue(int hue);
  virtual int getHue(int& hue);
  virtual int setJpegQuality(int jpeg_quality) {UNUSED_PARAM(jpeg_quality); return -1;}
  //flip
  virtual int setFlip(int flip);
  virtual int getFlip(int& flip);
  int queryBusInfo(unsigned char* busInfo);
  //ircut
  //return -1 means unsupported
  virtual int isSupportedIrCut();
  //>0 means IRCUT is working
  virtual int getIrCutState();
  virtual int setIrCutState(int state);

 private:
  struct rk_cam_video_node* mUSBVideoNode;
};

#endif
