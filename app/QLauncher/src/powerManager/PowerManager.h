/*   ----------------------------------------------------------------------
Copyright (C) 2014-2017 Fuzhou Rockchip Electronics Co., Ltd

     Sec Class: Rockchip Confidential

V1.0 Dayao Ji <jdy@rock-chips.com>
---------------------------------------------------------------------- */

#ifndef __POWER_MANAGER_H_94387838993__
#define __POWER_MANAGER_H_94387838993__

int rk_put_system_suspend(void);
int rk_active_system_resumed(void);
int rk_get_wakelock(const char* id);
int rk_release_wakelock(const char* id);
int rk_get_screen_status(void);
int rk_set_screen_status(int status);
int rk_screen_on(void);
int rk_screen_off(void);


#endif
