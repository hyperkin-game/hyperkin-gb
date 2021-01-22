#ifndef _CAMERA_IQDEBUG_H_
#define _CAMERA_IQDEBUG_H_

#include "CameraIspTunning.h"


#define RK_IPC_MSG_QUEUE_ROUTE "/tmp"
#define RK_IPC_MSG_QUEUE_KEY   10
#define RK_DUMP_PATH_MAX       100

using namespace std;


enum RK_IPC_MSG_TYPE {
  RK_IPC_MSG_TYPE_INVALID,
  RK_IPC_MSG_TYPE_NORMAL
};

enum RK_IPC_MSG_ID {
  RK_IPC_MSG_ID_INVALID,
  RK_IPC_MSG_ID_DUMP
};

struct rk_dump_ctl {
  bool dump_enable;
  uint32_t dump_num;
  uint32_t dump_pathid;
  char dump_path[RK_DUMP_PATH_MAX];
};

struct rk_ipc_message {
  RK_IPC_MSG_TYPE msg_type;
  RK_IPC_MSG_ID msg_id;
  union {
    struct rk_dump_ctl dump_ctl;
  } msg_detail;
};

class CameraIqDebug {
 public:
  static CameraIqDebug* getInstance();
  ~CameraIqDebug();

  bool getDumpCtl(struct rk_dump_ctl &dump_ctl, uint32_t pathID);

 protected:
  CameraIqDebug();

 private:
  class taskThread;
  friend class taskThread;
  class taskThread : public CamThread {
   public:
    taskThread(CameraIqDebug* owner): mOwner(owner) {};
    virtual bool threadLoop(void) { return mOwner->taskThLoop();}
   private:
    CameraIqDebug* mOwner;
  };

  bool taskThLoop();
  void start();
  void stop();

 private:
  int mQueueID;
  bool mInitState;
  shared_ptr<CamThread> mTaskThread;
  static CameraIqDebug* mInstance;

  mutable osMutex mDumpCtlLock;
  struct rk_dump_ctl mDumpCtl;
};


#endif

