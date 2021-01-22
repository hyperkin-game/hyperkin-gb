/*   ----------------------------------------------------------------------
Copyright (C) 2014-2017 Fuzhou Rockchip Electronics Co., Ltd

     Sec Class: Rockchip Confidential

V1.0 Dayao Ji <jdy@rock-chips.com>
---------------------------------------------------------------------- */


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>



#define PX3SE_WAKE_LOCK		("px3se_wakelock")
static int screen_status  = 1;

#define PX3SE_BL_FILE		("/sys/class/backlight/rk28_bl/brightness")
#define PX3SE_FB_FILE		("/dev/fb0")

#define FBIOBLANK		0x4611
#define FB_BLANK_UNBLANK	0
#define FB_BLANK_POWERDOWN	4

#define POWER_STATE_MEM	"mem"
#define POWER_STATE_ON	"off"
#define POWER_STATE	"/sys/power/autosleep"


static int rk_set_power_state(char *state)
{
	int fd = 0;
	int ret = -1;

	fd =  open(POWER_STATE, O_RDWR);
	if(fd < 0) {
		return -1;
    }
	ret = write(fd, state, strlen(state));

    close(fd);
    return (ret > 0) ? 0 : -1;
}

int rk_put_system_suspend(void)
{
	return rk_set_power_state(POWER_STATE_MEM);
}

int rk_active_system_resumed(void)
{
	return rk_set_power_state(POWER_STATE_ON);
}


int rk_get_wakelock(const char* id)
{
	int fd;

	if(NULL == id)
	{
		return -1;
	}

	fd = open( "/sys/power/wake_lock", O_RDWR);

	write(fd, id, strlen(id));

	close(fd);

	return 0;
}

int rk_release_wakelock(const char* id)
{
	int fd;

	if(NULL == id)
	{
		return -1;
	}

	fd = open( "/sys/power/wake_unlock", O_RDWR);

	write(fd, id, strlen(id));

	close(fd);

	return 0;
}


int rk_get_screen_status(void)
{
	return screen_status;
}

int rk_set_screen_status(int status)
{
	screen_status = status;

	return 0;
}

int rk_set_backlight_level(int level)
{
	int fd = 0;
	int ret;
	char buf[10];

	fd =  open(PX3SE_BL_FILE, O_RDWR);
	if(fd < 0) {
		return -1;
    }
	sprintf(buf, "%d", level);
	ret = write(fd, buf, strlen(buf));

    close(fd);
    return (ret > 0) ? 0 : -1;
}


int rk_screen_on(void)
{
	int fd = -1;

	fd = open(PX3SE_FB_FILE, O_RDWR);

	ioctl(fd, FBIOBLANK, FB_BLANK_UNBLANK);
	rk_set_backlight_level(128);
	rk_set_screen_status(1);

	close(fd);

	return 0;
}

int rk_screen_off(void)
{
	int fd = -1;

	fd = open(PX3SE_FB_FILE, O_RDWR);

	rk_set_backlight_level(0);
	ioctl(fd, FBIOBLANK, FB_BLANK_POWERDOWN);
	rk_set_screen_status(0);

	close(fd);

	return 0;
}
