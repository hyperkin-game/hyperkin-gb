#include <iostream>
#include <time.h>
#include <fcntl.h>
#ifdef RK_ISP10
#include <linux/v4l2-controls.h>
#include <media/rk-isp10-config.h>
#endif
#ifdef RK_ISP11
#include <linux/v4l2-controls.h>
#include <media/rk-isp11-config.h>
#endif
#include "../HAL/include/CameraBuffer.h"
#include "../HAL/include/CamHwItf.h"
//#include "../HAL/include/CamIsp10DevHwItf.h"
#include "../HAL/include/CamIsp11DevHwItf.h"
#ifdef SUPPORT_ION
#include "../HAL/include/IonCameraBuffer.h"
#endif

#include "../HAL/include/CamCifDevHwItf.h"


#include "cam_ia_api/cam_ia10_trace.h"
#include "../HAL/include/StrmPUBase.h"

#include "../HAL/include/CameraIspTunning.h"
#ifdef USE_RK_DISPLAY
#include "fb.h"
#endif

#ifdef USE_DRM_DISPLAY
#include "drmDsp.h"
#endif

//#define TEST_WRITE_SP_TO_FILE

using namespace std;
#define DECLAREPERFORMANCETRACK(name) \
  static int mFrameCount##name = 0;\
  static int mLastFrameCount##name = 0;\
  static long long mLastFpsTime##name = 0;\
  static float mFps##name = 0;

#define SHOWPERFORMACEFPS(name) \
  { \
    mFrameCount##name++;\
    if (!(mFrameCount##name & 0x1F)) { \
      struct timeval now; \
      gettimeofday(&now,NULL); \
      long long diff = now.tv_sec*1000000 + now.tv_usec - mLastFpsTime##name; \
      mFps##name = ((mFrameCount##name - mLastFrameCount##name) * float(1000*1000)) / diff; \
      mLastFpsTime##name = now.tv_sec*1000000 + now.tv_usec; \
      mLastFrameCount##name = mFrameCount##name; \
      LOGD("%s:%d Frames, %2.3f FPS",#name, mFrameCount##name, mFps##name); \
    } \
  }

#ifdef USE_RK_DISPLAY
static struct fb gFb = {0};
#endif

class CameraBufferOwenerImp : public ICameraBufferOwener {
 public:
  virtual bool releaseBufToOwener(weak_ptr<BufferBase> camBuf) { UNUSED_PARAM(camBuf); return false;}
};


enum SOFIAISPUSECASE {
  CAMMP,
  CAMSP,
  DMAMPSP,
  MPSP,
};
static struct sofiaTestPara_s {
  enum SOFIAISPUSECASE usecase;
  int camId;
  frm_info_t frmFmt;
} gSofiaTestPara = {
  .usecase = CAMSP,
  .camId = 0,
  .frmFmt = {
    .frmSize = {1920, 1080},
    .frmFmt = HAL_FRMAE_FMT_NV12,
    .colorSpace = HAL_COLORSPACE_SMPTE170M,
    .fps = 30,
  },
};

class CambufNotifierImp : public NewCameraBufferReadyNotifier {
 private:
  CamHwItf* mDev;
 public:
  CambufNotifierImp(CamHwItf* dev = NULL): mDev(dev) {}
  ~CambufNotifierImp() {}
  virtual bool bufferReady(weak_ptr<BufferBase> buffer, int status) {
    UNUSED_PARAM(status);
    DECLAREPERFORMANCETRACK(CambufNotifierImp);
    shared_ptr<BufferBase> spCamBuf = buffer.lock();
    if (spCamBuf.get()) {
      SHOWPERFORMACEFPS(CambufNotifierImp);
      /* increase used count */
      spCamBuf->incUsedCnt();

      //test get metadata
      struct HAL_Buffer_MetaData* metaData = spCamBuf->getMetaData();
      if ((metaData) && (metaData->metedata_drv)) {
        //ALOGD("get meta data success!");
#if 0
        struct v4l2_buffer_metadata_s* v4l2Meta =
            (struct v4l2_buffer_metadata_s*)metaData->metedata_drv;
        struct cifisp_isp_metadata* ispMetaData =
            (struct cifisp_isp_metadata*)v4l2Meta->isp;
        if (ispMetaData) {
          ALOGD("sensor exposure time & gain:%d,%d",
                ispMetaData->meas_stat.sensor_mode.exp_time,
                ispMetaData->meas_stat.sensor_mode.gain);
          ALOGD("frame id %d",  ispMetaData->meas_cfg.s_frame_id);
        }
#endif
        //ALOGD("exp gain & time %f %f",metaData->exp_gain, metaData->exp_time);
        //ALOGD("wb gain RGrGbB %f %f %f %f",
        //  metaData->wb_gain.gain_red, metaData->wb_gain.gain_green_r,
        //  metaData->wb_gain.gain_green_b,metaData->wb_gain.gain_blue);
        //ALOGD("flt denoise sharp %d %d",metaData->flt.denoise_level,metaData->flt.sharp_level);
      }

      static int frames_set = 0;
      frames_set++;

      if ((frames_set % 300) == 0) {
        HAL_FPS_INFO_t fps;

        fps.numerator = 1;
        fps.denominator = 10;
        ALOGD("set fps to 10 ....");
        if (mDev)
          mDev->setFps(fps);
      } else if ((frames_set % 200) == 0) {
        HAL_FPS_INFO_t fps;

        fps.numerator = 1;
        fps.denominator = 15;
        ALOGD("set fps to 15 ....");
        if (mDev)
          mDev->setFps(fps);
      }

#ifdef TEST_WRITE_SP_TO_FILE
      //write to file
      char fname[50] = {0};
      static int frames = 0;
      snprintf(fname, 30, "/tmp/yuv_%dx%d.bin", spCamBuf->getWidth(), spCamBuf->getHeight());
      frames++;
      if ((frames > 25) && (frames < 29)) {
        FILE* yuv_file =  fopen(fname, "a+");
        if (yuv_file) {
          fwrite(spCamBuf->getVirtAddr(), spCamBuf->getDataSize(), 1, yuv_file);
          ALOGD("write 0x%x bytes to file!", spCamBuf->getDataSize());
          fclose(yuv_file);
        } else
          ALOGE("open file %s error", fname);
      }
#endif
      //send to display
#ifdef USE_RK_DISPLAY
      rk_fb_config(&gFb,
                   0,  /*win id*/
                   spCamBuf->getWidth(),
                   spCamBuf->getHeight(),
                   HAL_PIXEL_FORMAT_YCrCb_NV12,
                   (short)(spCamBuf->getFd()),
                   /*(unsigned int)((spCamBuf->getPhyAddr()))*/0
                  );
#endif

#ifdef USE_DRM_DISPLAY
      drmDspFrame(spCamBuf->getWidth(),
                  spCamBuf->getHeight(),
                  (int)(spCamBuf->getVirtAddr()),
                  DRM_FORMAT_NV12);
#endif
      spCamBuf->decUsedCnt();
    }
    return true;
  }

};


class CambufMpNotifierImp : public NewCameraBufferReadyNotifier {
 public:
  CambufMpNotifierImp() {}
  ~CambufMpNotifierImp() {}
  virtual bool bufferReady(weak_ptr<BufferBase> buffer, int status) {
    UNUSED_PARAM(status);
    DECLAREPERFORMANCETRACK(CambufMpNotifierImp);
    shared_ptr<BufferBase> spCamBuf = buffer.lock();
    if (spCamBuf.get()) {
      SHOWPERFORMACEFPS(CambufMpNotifierImp);
      //ALOGD("mp got a new frame !!!!");
    }
    return true;
  }
};

static struct rk_cams_dev_info g_test_cam_infos;
#define RUNNINGTIME_MINUTE 3
#define RUNNINGTIME_MS (RUNNINGTIME_MINUTE*1000)
void testSofiaIsp(struct sofiaTestPara_s* para) {
  int i = 0;
  if (g_test_cam_infos.num_camers <= 0)
    ALOGE("%s:no camera connected!!");
  //search isp connectd cameras
  for (i = 0; i < g_test_cam_infos.num_camers; i++) {
    if (g_test_cam_infos.cam[i]->type == RK_CAM_ATTACHED_TO_ISP) {
      ALOGD("connected isp camera name %s,input id %d", g_test_cam_infos.cam[i]->name,
            g_test_cam_infos.cam[i]->index);
      break;
    }
  }

  if (i == g_test_cam_infos.num_camers)
    ALOGE("%s:no camera connected to isp !!");

  //new ISP DEV
  ALOGD("construct ISP dev........");
  //shared_ptr<CamHwItf> testIspDev =  shared_ptr<CamHwItf> \
  //      (new CamIsp11DevHwItf(&(g_test_cam_infos.isp_dev)));//getCamHwItf();
  shared_ptr<CamHwItf> testIspDev =  getCamHwItf(&(g_test_cam_infos.isp_dev));
  ALOGD("init ISP dev......");
  if (testIspDev->initHw(g_test_cam_infos.cam[i]->index) == false)
    ALOGE("isp dev init error !\n");
  else {
    frm_info_t frmFmt;
    memcpy(&frmFmt, &(gSofiaTestPara.frmFmt), sizeof(frm_info_t));
    if (para->usecase == DMAMPSP)
      goto USECASE_DMAMPSP;
    else if (para->usecase == CAMSP)
      goto USECASE_CAMSP;
    else if (para->usecase == CAMMP)
      goto USECASE_CAMMP;
    else if (para->usecase == MPSP)
      goto USECASE_CAMMPSP;
    else
      goto USECASE_END;
USECASE_DMAMPSP: {
      //DMA path test
      ALOGD("start DMA path test .......");
      shared_ptr<CamHwItf::PathBase> dmaPath = testIspDev->getPath(CamHwItf::DMA);
      if (dmaPath.get() == NULL)
        cout << "dma path doesn't exist!" << endl;
      else {
#ifdef SUPPORT_ION //alloc buffers
        shared_ptr<IonCameraBufferAllocator> bufAlloc(new IonCameraBufferAllocator());
        ALOGD("dma  inputfmt %s:%dx%d@%dfps\n", RK_HAL_FMT_STRING::hal_fmt_map_to_str(frmFmt.frmFmt),
              frmFmt.frmSize.width, frmFmt.frmSize.height, frmFmt.fps);
        list<shared_ptr<BufferBase> > bufPool;
        weak_ptr<ICameraBufferOwener> bufOwener;
        for (int i = 0; i < 4; i++) {
          shared_ptr<BufferBase> buffer;
          buffer = bufAlloc->alloc(RK_HAL_FMT_STRING::hal_fmt_map_to_str(frmFmt.frmFmt)
                                   , (frmFmt.frmSize.width), (frmFmt.frmSize.height)
                                   , 0, bufOwener);
          if (buffer.get())
            bufPool.push_back(buffer);
          else
            ALOGE("alloc buffer failed.....");

        }
        //TODO: fill buffers
        //get mp and sp,then set input as DMA
        shared_ptr<CamHwItf::PathBase> mpath = testIspDev->getPath(CamHwItf::MP);
        if (mpath.get() == NULL)
          cout << "mpath doesn't exist!" << endl;
        else {
          if (!mpath->setInput(CamHwItf::INP_DMA)) {
            ALOGE("%s: Setting input to DMA failed", __func__);
            return;
          }
        }
        shared_ptr<CamHwItf::PathBase> spath = testIspDev->getPath(CamHwItf::SP);
        if (spath.get() == NULL)
          cout << "spath doesn't exist!" << endl;
        else {
          if (!spath->setInput(CamHwItf::INP_DMA)) {
            ALOGE("%s: Setting input to DMA failed", __func__);
            return;
          }
        }
        //prepare DMA path
        if (dmaPath->prepare(frmFmt, bufPool, 4, 0)) {
          //start dma path
          NewCameraBufferReadyNotifier* mDMABufNotifer = new CambufNotifierImp();
          dmaPath->addBufferNotifier(mDMABufNotifer);
          if (dmaPath->start())
            ALOGD("dma path  start success!\n");
          //prepare sp path
          frmFmt.frmFmt = HAL_FRMAE_FMT_NV12;
          NewCameraBufferReadyNotifier* mSpBufNotifer = new CambufNotifierImp();
          //add stream PU for notifer
          //default will new a process thread and put process frame result to a new buffer
          shared_ptr<StreamPUBase>  mStreamPU(new StreamPUBase());
          if (spath->prepare(frmFmt, 4, *(bufAlloc.get()), false, 0)) {
            ALOGD("sp prepare success\n");
            ALOGD("sp add buffer notifer before start\n");
            //set buffer notifer required fmt
            mSpBufNotifer->setReqFmt(frmFmt);
            spath->addBufferNotifier(mSpBufNotifer);
            spath->addBufferNotifier(mStreamPU.get());
            mStreamPU->prepare(frmFmt, 4, bufAlloc);
            //start sp path
            if (spath->start())
              ALOGD("sp start success!\n");
            mStreamPU->start();
          }
          //prepare mp path
          NewCameraBufferReadyNotifier* mMpBufNotifer = new CambufNotifierImp();
          if (mpath->prepare(frmFmt, 4, *(bufAlloc.get()), false, 0)) {
            ALOGD("mp prepare success\n");
            ALOGD("mp add buffer notifer before start\n");
            //set buffer notifer required fmt
            mMpBufNotifer->setReqFmt(frmFmt);
            mpath->addBufferNotifier(mMpBufNotifer);
            //start DMA path and sp path
            if (mpath->start())
              ALOGD("mp start success!\n");
          }
          //osSleep(RUNNINGTIME_MS);
          while (1);
          //stop stream PU
          spath->removeBufferNotifer(mStreamPU.get());
          mStreamPU->stop();
          mStreamPU->releaseBuffers();
          //stop mp and sp path
          ALOGD("stop sp path");
          spath->removeBufferNotifer(mSpBufNotifer);
          delete mSpBufNotifer;
          spath->stop();
          spath->releaseBuffers();

          ALOGD("stop mp path");
          mpath->removeBufferNotifer(mMpBufNotifer);
          delete mMpBufNotifer;
          mpath->stop();
          mpath->releaseBuffers();
          // stop DMA path
          ALOGD("stop dma path");
          dmaPath->removeBufferNotifer(mDMABufNotifer);
          delete mDMABufNotifer;
          dmaPath->stop();
          dmaPath->releaseBuffers();
        }
        //clear buffer pool
        ALOGD("cleare DMA buffer pool");
        bufPool.clear();
#endif
        ALOGD("dma path test end ......");
        goto USECASE_END;
      }
    }
USECASE_CAMSP: {
      //sp test
      ALOGD("start sp test........");
      shared_ptr<CamHwItf::PathBase> spath = testIspDev->getPath(CamHwItf::SP);
      if (spath.get() == NULL)
        cout << "spath doesn't exist!" << endl;
      else {
#ifdef SUPPORT_ION
        shared_ptr<IonCameraBufferAllocator> bufAlloc(new IonCameraBufferAllocator());

        weak_ptr<ICameraBufferOwener> bufOwener;

        ALOGD("sp fmt %s:%dx%d@%dfps\n", RK_HAL_FMT_STRING::hal_fmt_map_to_str(frmFmt.frmFmt),
              frmFmt.frmSize.width, frmFmt.frmSize.height, frmFmt.fps);
        if (spath->prepare(frmFmt, 4, *(bufAlloc.get()), false, 0)) {
          ALOGD("sp prepare success\n");
          ALOGD("sp add buffer notifer before start\n");
          NewCameraBufferReadyNotifier* mBufNotifer = new CambufNotifierImp(testIspDev.get());
          //set buffer notifer required fmt
          mBufNotifer->setReqFmt(frmFmt);
          spath->addBufferNotifier(mBufNotifer);
          if (spath->start())
            ALOGD("sp start success!\n");
          //osSleep(RUNNINGTIME_MS);
          //while(1);
          getchar();
          ALOGD("sp remove buffer notifer before stop\n");
          spath->removeBufferNotifer(mBufNotifer);
          delete mBufNotifer;
          spath->stop();
          spath->releaseBuffers();
        } else
          ALOGE("sp prepare faild!");
#endif

      }
      ALOGD("sp test complete........");
      goto USECASE_END;
    }
USECASE_CAMMP: {
      //mp test
      ALOGD("start mp test........");
      shared_ptr<CamHwItf::PathBase> mpath = testIspDev->getPath(CamHwItf::MP);
      if (mpath.get() == NULL)
        cout << "mpath doesn't exist!" << endl;
      else {
#ifdef SUPPORT_ION
        shared_ptr<IonCameraBufferAllocator> bufAlloc(new IonCameraBufferAllocator());

        if (mpath->prepare(frmFmt, 4, *(bufAlloc.get()), false, 0)) {
          ALOGD("mp prepare success\n");
          ALOGD("mp fmt %s:%dx%d@%dfps\n", RK_HAL_FMT_STRING::hal_fmt_map_to_str(frmFmt.frmFmt),
                frmFmt.frmSize.width, frmFmt.frmSize.height, frmFmt.fps);
          NewCameraBufferReadyNotifier* mBufNotifer = new CambufMpNotifierImp();
          //set buffer notifer required fmt
          mBufNotifer->setReqFmt(frmFmt);
          mpath->addBufferNotifier(mBufNotifer);
          if (mpath->start())
            ALOGD("mp start success!\n");
          //osSleep(RUNNINGTIME_MS);
          getchar();
          mpath->removeBufferNotifer(mBufNotifer);
          delete mBufNotifer;
          mpath->stop();
          mpath->releaseBuffers();
        } else
          ALOGE("mp prepare faild!");
#endif
        ALOGD("mp test complete........");
        goto USECASE_END;
      }
    }
USECASE_CAMMPSP: {
      shared_ptr<CamHwItf::PathBase> mpath = testIspDev->getPath(CamHwItf::MP);
      shared_ptr<CamHwItf::PathBase> spath = testIspDev->getPath(CamHwItf::SP);
      if (mpath.get() && spath.get()) {
        shared_ptr<IonCameraBufferAllocator> bufAlloc(new IonCameraBufferAllocator());
        frmFmt.frmSize.width = 1920;
        frmFmt.frmSize.height = 1080;
        bool ret = mpath->prepare(frmFmt, 4, *(bufAlloc.get()), false, 0);
        if (ret == true) {
          //mpath->start();
          frmFmt.frmSize.width = 640;
          frmFmt.frmSize.height = 360;
          ret = spath->prepare(frmFmt, 4, *(bufAlloc.get()), false, 0);
          if (ret == true) {
            NewCameraBufferReadyNotifier* mSpBufNotifer = new CambufNotifierImp();
            NewCameraBufferReadyNotifier* mMpBufNotifer = new CambufMpNotifierImp();
            mSpBufNotifer->setReqFmt(frmFmt);
            spath->addBufferNotifier(mSpBufNotifer);
            //mpath->addBufferNotifier(mMpBufNotifer);
            mpath->start();
            spath->start();
            getchar();
            mpath->removeBufferNotifer(mMpBufNotifer);
            spath->removeBufferNotifer(mSpBufNotifer);
            delete mSpBufNotifer;
            delete mMpBufNotifer;
            mpath->stop();
            mpath->releaseBuffers();
            spath->stop();
            spath->releaseBuffers();
          }
        }
      }
    }
  }
USECASE_END:
  //deinit HW
  ALOGD("deinit ISP dev......");
  testIspDev->deInitHw();
  //delete isp dev
  ALOGD("destruct ISP dev......");
  testIspDev.reset();
}

static void testTunningClass() {
  int i = 0;
  if (g_test_cam_infos.num_camers <= 0)
    ALOGE("%s:no camera connected!!");
  //search isp connectd cameras
  for (i = 0; i < g_test_cam_infos.num_camers; i++) {
    if (g_test_cam_infos.cam[i]->type == RK_CAM_ATTACHED_TO_ISP) {
      ALOGD("connected isp camera name %s,input id %d", g_test_cam_infos.cam[i]->name,
            g_test_cam_infos.cam[i]->index);
      break;
    }
  }

  if (i == g_test_cam_infos.num_camers)
    ALOGE("%s:no camera connected to isp !!");

  //new ISP DEV
  ALOGD("construct ISP dev........");
  //shared_ptr<CamHwItf> testIspDev =  shared_ptr<CamHwItf> \
  //      (new CamIsp11DevHwItf(&(g_test_cam_infos.isp_dev)));//getCamHwItf();
  shared_ptr<CamHwItf> testIspDev =  getCamHwItf(&(g_test_cam_infos.isp_dev));
  ALOGD("init ISP dev......");
  if (testIspDev->initHw(g_test_cam_infos.cam[i]->index) == false)
    ALOGE("isp dev init error !\n");
#ifdef SUPPORT_ION //alloc buffers
  shared_ptr<IonCameraBufferAllocator> bufAlloc(new IonCameraBufferAllocator());
  CameraIspTunning* tunningInstance = CameraIspTunning::createInstance(testIspDev.get());
  if (tunningInstance) {
    frm_info_t frmFmt;
    if (tunningInstance->prepare(frmFmt, 4, bufAlloc)) {
      if (tunningInstance->start()) {
        //getchar to stop
        LOGD("press any key to stop tuning:");
        getchar();
        tunningInstance->stop();
      }
    }
  }


#endif
  //deinit HW
  ALOGD("deinit ISP dev......");
  testIspDev->deInitHw();
  //delete isp dev
  ALOGD("destruct ISP dev......");
  testIspDev.reset();
}

void testCifDev(int videoDevNum, int inputid) {
  int i = 0;
  if (g_test_cam_infos.num_camers <= 0)
    ALOGE("%s:no camera connected!!");
  //search cif connectd cameras
  for (i = 0; i < g_test_cam_infos.num_camers; i++) {
    if ((g_test_cam_infos.cam[i]->type == RK_CAM_ATTACHED_TO_CIF) &&
        (g_test_cam_infos.cam[i]->dev == (&(g_test_cam_infos.cif_devs.cif_devs[videoDevNum]))) &&
        (g_test_cam_infos.cam[i]->index == inputid)) {
      ALOGD("connected cif camera name %s,input id %d", g_test_cam_infos.cam[i]->name,
            g_test_cam_infos.cam[i]->index);
      break;
    }
  }

  if (i == g_test_cam_infos.num_camers)
    ALOGE("no input %d connected to cif  %d !!", inputid, videoDevNum);

  ALOGD("construct CIF dev........");
  shared_ptr<CamHwItf> testCifDev =  shared_ptr<CamHwItf>\
                                     (new CamCifDevHwItf(&(g_test_cam_infos.cif_devs.cif_devs[videoDevNum])));//getCamHwItf();
  ALOGD("init CIF dev......");
  if (testCifDev->initHw(g_test_cam_infos.cam[i]->index) == false)
    ALOGE("isp CIF init error !\n");
#ifdef SUPPORT_ION //alloc buffers
  shared_ptr<CamHwItf::PathBase> mpath = testCifDev->getPath(CamHwItf::MP);
  shared_ptr<IonCameraBufferAllocator> bufAlloc(new IonCameraBufferAllocator());
  frm_info_t frmFmt = {
    .frmSize = {720, 576},
    .frmFmt = HAL_FRMAE_FMT_NV12,
    .colorSpace = HAL_COLORSPACE_SMPTE170M,
    .fps = 30,
  };
  frm_info_t outFmt;

  testCifDev->tryFormat(frmFmt, outFmt);

  NewCameraBufferReadyNotifier* mMpBufNotifer = new CambufNotifierImp();
  mpath->addBufferNotifier(mMpBufNotifer);
  if (mpath->prepare(outFmt, 4, *(bufAlloc.get()), false, 0)) {
    if (mpath->start()) {
      //getchar to stop
      LOGD("press any key to stop tuning:");
      getchar();
      mpath->removeBufferNotifer(mMpBufNotifer);
      delete mMpBufNotifer;
      mpath->stop();
    }
  }
#endif
  //deinit HW
  ALOGD("deinit cif dev......");
  testCifDev->deInitHw();
  //delete isp dev
  ALOGD("destruct cif dev......");
  testCifDev.reset();

}

int main(int argc, const char* argv[]) {
  ALOGD("tests start !\n");
#ifdef USE_RK_DISPLAY
  rk_fb_open(&gFb);
#endif

#ifdef USE_DRM_DISPLAY
  if (initDrmDsp() < 0)
    ALOGD("DRM disp init failed !");
#endif

  //get all connected camera infos
  memset(&g_test_cam_infos, 0, sizeof(g_test_cam_infos));
  CamHwItf::getCameraInfos(&g_test_cam_infos);

  if (argc > 1) {
    int videoIndex = atoi(argv[1]);
    int inputId = 0;
    ALOGD("video index is %d", videoIndex);
    if (argc == 3)
      inputId = atoi(argv[2]);
    testCifDev(videoIndex, inputId);
  } else {
    while (1)
      testSofiaIsp(&gSofiaTestPara);
    //testTunningClass();
  }
#ifdef USE_RK_DISPLAY
  rk_fb_close(&gFb);
#endif
#ifdef USE_DRM_DISPLAY
  deInitDrmDsp();
#endif
  ALOGD("tests end !\n");
  return 0;
}

