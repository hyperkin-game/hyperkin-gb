#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "CamIqDebug.h"
#include "cam_ia_api/cam_ia10_trace.h"
#include "CamHwItf.h"


CameraIqDebug::CameraIqDebug() {
  //ALOGD("%s: E", __func__);
  osMutexInit(&mDumpCtlLock);
  mInitState = false;
  //ALOGD("%s: X", __func__);
}

CameraIqDebug::~CameraIqDebug() {
  //ALOGD("%s: E", __func__);
  stop();
  osMutexDestroy(&mDumpCtlLock);
  //ALOGD("%s: X", __func__);
}

void CameraIqDebug::start() {
  key_t key;

  //ALOGD("%s: E", __func__);
  if (mInitState == false) {
    if ((key=ftok(RK_IPC_MSG_QUEUE_ROUTE, RK_IPC_MSG_QUEUE_KEY)) == -1){
      ALOGE("%s: call ftok error, %s!", __func__, strerror(errno));
      return;
    }
    ALOGD("------key of msgque--- 0x%x !\n", key);

    if ((mQueueID=msgget(key, IPC_CREAT)) == -1){
      ALOGE("%s: call msgget error, %s!", __func__, strerror(errno));
      return;
    }

    mTaskThread = shared_ptr<CamThread>(new taskThread(this));
    if (mTaskThread->run("CameraIqDebugTask")){
      ALOGE("%s: run CameraIqDebugTask error, %s!", __func__, strerror(errno));
      return;
    }

    mInitState = true;
  }
  //ALOGD("%s: X", __func__);
}

void CameraIqDebug::stop() {
  //ALOGD("%s: E", __func__);
  if (mInitState == true) {
    if (mTaskThread.get())
      mTaskThread->requestExitAndWait();
    mTaskThread.reset();

    msgctl(mQueueID, IPC_RMID, NULL);
    mInitState = false;
  }
  //ALOGD("%s: X", __func__);
}

CameraIqDebug* CameraIqDebug::mInstance = new CameraIqDebug;
CameraIqDebug* CameraIqDebug::getInstance() {
  //ALOGD("%s: EX", __func__);
  mInstance->start();
  return mInstance;
}

bool CameraIqDebug::taskThLoop() {
  bool ret = true;
  struct rk_ipc_message ipc_msg;
  struct rk_dump_ctl dump_ctl;

  //ALOGD("%s: E", __func__);
  ipc_msg.msg_id = RK_IPC_MSG_ID_INVALID;
  if(msgrcv(mQueueID, (void *)&ipc_msg, sizeof(ipc_msg)-sizeof(long), 0, 0) < 0) {
    ALOGE("%s: call msgrcv error %s!", __func__, strerror(errno));
  }
  else if (ipc_msg.msg_id == RK_IPC_MSG_ID_DUMP) {
    //dump frame data
    ALOGD("Dump frame data");
    osMutexLock(&mDumpCtlLock);
    mDumpCtl = ipc_msg.msg_detail.dump_ctl;
    mDumpCtl.dump_enable = true;
    osMutexUnlock(&mDumpCtlLock);
  }
  //ALOGD("%s: X", __func__);
  return ret;
}

bool CameraIqDebug::getDumpCtl(struct rk_dump_ctl &dump_ctl, uint32_t pathID) {
  bool ret = true;

  //ALOGD("%s: E", __func__);
  osMutexLock(&mDumpCtlLock);
  if (mDumpCtl.dump_enable == false || mDumpCtl.dump_pathid != pathID) {
    ret = false;
    dump_ctl.dump_enable = false;
  }
  else {
    dump_ctl = mDumpCtl;
    mDumpCtl.dump_enable = false;
  }
  osMutexUnlock(&mDumpCtlLock);
  //ALOGD("%s: X", __func__);
  return ret;
}

