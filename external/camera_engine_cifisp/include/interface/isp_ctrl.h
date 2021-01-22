#ifndef __ISP_CTRL__
#define __ISP_CTRL__

#include <HAL/CamIsp10CtrItf.h>

int getSensorModeData(int devFd,
    struct isp_supplemental_sensor_mode_data* data);
int setExposure(int m_cam_fd_overlay, unsigned int vts,
    unsigned int exposure, unsigned int gain, unsigned int gain_percent);
int setAutoAdjustFps(int m_cam_fd_overlay, bool auto_adjust_fps);
int setFocusPos(int m_cam_fd_overlay, unsigned int position);
#endif
