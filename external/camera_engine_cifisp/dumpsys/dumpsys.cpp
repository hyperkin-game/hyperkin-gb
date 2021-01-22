#include <iostream>

#include "../HAL/include/CameraBuffer.h"
#include "../HAL/include/CamHwItf.h"
//#include "../HAL/include/CamIsp10DevHwItf.h"
#ifdef SUPPORT_ION
#include "../HAL/include/IonCameraBuffer.h"
#endif
#ifdef SUPPORT_DMABUF
#include "../HAL/include/DMABufCameraBuffer.h"
#endif
#include "cam_ia_api/cam_ia10_trace.h"
#include "../HAL/include/StrmPUBase.h"

#include "../HAL/include/CameraIspTunning.h"

static struct rk_cams_dev_info g_test_cam_infos;

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
  //shared_ptr<CamHwItf> testIspDev =  shared_ptr<CamHwItf>(new CamIsp10DevHwItf());//getCamHwItf();
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
        //getchar();
        tunningInstance->stop();//the stop will be waiting for all task over signal
      }
    }
  }


#endif
#ifdef SUPPORT_DMABUF //alloc buffers
  shared_ptr<DMABufCameraBufferAllocator> bufAlloc(new DMABufCameraBufferAllocator());
  CameraIspTunning* tunningInstance = CameraIspTunning::createInstance(testIspDev.get());
  if (tunningInstance) {
    frm_info_t frmFmt;
    if (tunningInstance->prepare(frmFmt, 4, bufAlloc)) {
      if (tunningInstance->start()) {
        //getchar to stop
        LOGD("press any key to stop tuning:");
        //getchar();
        tunningInstance->stop();//the stop will be waiting for all task over signal
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

int main() {
  ALOGD("dumpsys start !\n");
  //get all connected camera infos
  memset(&g_test_cam_infos, 0, sizeof(g_test_cam_infos));
  CamHwItf::getCameraInfos(&g_test_cam_infos);
  testTunningClass();
  ALOGD("dumpsys end !\n");
  return 0;
}


