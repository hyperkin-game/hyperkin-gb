#ifndef _POWER_MANAGER_9383893_H__
#define _POWER_MANAGER_9383893_H__

#include <netlink/list.h>


#define POWER_STATE_MEM	"mem"
#define POWER_STATE_ON	"off"
#define POWER_STATE	"/sys/power/autosleep"

#define BACKLIGHT_BRIGHTNESS	"/sys/devices/platform/backlight/backlight/backlight/brightness"
#define DISP_DEV "/dev/fb0"


#define THERMAL_SENSOR_CONFIG	"/etc/thermal_sensor_config.xml"
#define THERMAL_THROTTLE_CONFIG	"/etc/thermal_throttle_config.xml"

#define PM_CPU_CRTL_PATH	"/sys/devices/system/cpu/cpu0/cpufreq"
#define PM_CPU_FREQ_MAX		"/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq"
#define PM_KERNEL_THERMAL	"/sys/module/rockchip_pm/parameters/policy"

#define PM_THRESHOLD_MAX	10

#define MAX_EPOLL_EVENTS	40
#define UEVENT_MSG_LEN		2048
#define ZONE_CDEV_MAP_MAX	32


#define FBIOBLANK		0x4611
#define FB_BLANK_UNBLANK	0
#define FB_BLANK_POWERDOWN	4

#define DISP_CMD_LCD_ON		0x140
#define DISP_CMD_LCD_OFF	0x141


#define pm_list_for_each_reverse_entry(pos, head, member)			\
	for (pos = nl_list_entry((head)->prev, typeof(*pos), member);	\
		&(pos)->member != (head); 	\
		(pos) = nl_list_entry((pos)->member.prev, typeof(*(pos)), member))

enum {
	SCREEN_OFF = 0,
	SCREEN_ON
};

enum sleep_in_priority {
	SLEEP_IN_ULOW = 0,
	SLEEP_IN_LOW,
	SLEEP_IN_MED,
	SLEEP_IN_HIGH,
	SLEEP_IN_UHIGH
};

struct pm_reg_t {
	char module_name[32];
	int flag;
	int (*get_state_cb)(void*);
	void* get_state_cb_arg;
	int (*sleep_in_cb)(void*);
	void* sleep_in_cb_arg;
	int (*sleep_out_cb)(void*, int);
	void* sleep_out_cb_arg;
	struct nl_list_head list;
};

struct pm_thermal_sonsor_t {
	char *name;
	char *path;
	char *temp;
	char *htemp;
	char *ltemp;
	char *uevent_path;
	struct nl_list_head list;
};

struct pm_cooling_dev_t {
	char *name;
	int id;
	char *throttle_path;
	char *class;
	int (*class_cb)(int, struct pm_cooling_dev_t*);
	int throttle_enable;
	int throttle[PM_THRESHOLD_MAX];
	int state;
	struct nl_list_head list;
};

struct pm_zone_t {
	char *name;
	int id;
	char *sensor_name;
	struct pm_thermal_sonsor_t *sensor;
	int threshold[PM_THRESHOLD_MAX];
	int enable;
	int critical_shutdown;
	int old_state;
	int state;
	struct nl_list_head list;
};

struct pm_zone_cdev_map {
	int zoneid;
	int cooldev;
	int state;
};

struct pm_uevent_t {
	pthread_t ueventd_thread;
	int epoll_fd;
        int uevent_fd;
};

int pm_init(void);
int pm_register(char *module_name, enum sleep_in_priority slp_in_priority,
		int (*get_state_cb)(void*), void* get_state_cb_arg,
		int (*sleep_in_cb)(void*), void* sleep_in_cb_arg,
		int (*sleep_out_cb)(void*, int), void* sleep_out_cb_arg);
int pm_unregister(char *module_name);
int screenOn(void);
int screenOff(void);
int pm_sleep(void);
int pm_wakeup(void);
void pm_reboot(void);
void pm_poweroff(void);
int system_suspend_immediately(void);
int get_screen_status(void);


#endif
