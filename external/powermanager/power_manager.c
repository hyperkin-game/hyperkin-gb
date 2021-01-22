#include <sys/cdefs.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/reboot.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <sys/poll.h>

#include <stdbool.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <linux/netlink.h>

#include "PowerManager.h"

#if 0
struct ucred {
	pid_t pid;		/* PID of sending process.	*/
	uid_t uid; 		   /* UID of sending process.  */
	gid_t gid;			 /* GID of sending process.  */
};

enum{
	SCM_CREDENTIALS = 0x02	 /* Credentials passing.  */
};
#endif


static int mState = SCREEN_ON;
static int mBacklight_level = -1;

static NL_LIST_HEAD(g_pm_reg_list);
pthread_mutex_t g_pm_reg_lock;
int pm_reg_lock_init = 0;

struct pm_uevent_t pm_uevent;

extern void pm_thermal_parse(void);
extern void pm_thermal_uevent_check(char *msg);
extern void pm_stop_kernel_thermal(void);

static void pm_reg_lock(void)
{
	if (!pm_reg_lock_init) {
		pthread_mutex_init(&g_pm_reg_lock, NULL);
		pm_reg_lock_init = 1;
	}
	pthread_mutex_lock(&g_pm_reg_lock);
}

static void pm_reg_unlock(void)
{
	pthread_mutex_unlock(&g_pm_reg_lock);
}

static int set_power_state(char *state)
{
	int fd = 0;
	int ret = -1;

	fd =  open(POWER_STATE, O_RDWR);
	if(fd < 0) {
		printf("[PowerManager] open POWER_STATE fail\n");
		return -1;
	}
	ret = write(fd, state, strlen(state));
	if (ret <= 0)
		printf("[PowerManager] write Error (%s),ret=%d,fd=%d \n", strerror(errno), ret, fd);

	close(fd);
	return (ret > 0) ? 0 : -1;
}

static void* pm_poweroff_thread(void* arg)
{
	sync();
	printf("[PowerManager] power off");
	reboot(RB_POWER_OFF);
	pthread_exit(NULL);
}

void pm_poweroff(void)
{
	pthread_t tid;

	if (pthread_create(&tid, NULL, pm_poweroff_thread, NULL) != 0)
		printf("[PowerManager] Create poweroff thread error!\n");
}

void pm_reboot(void)
{
	printf("[PowerManager] reboot");
	reboot(RB_AUTOBOOT);
}

int systemSuspend(void)
{
	return set_power_state(POWER_STATE_MEM);
}

int systemResume(void)
{
	return set_power_state(POWER_STATE_ON);
}

int system_suspend_immediately(void)
{
	system("echo mem > /sys/power/state");
	return 0;
}

static int get_def_backlight_level(void)
{
	int fd = 0;
	int ret;
	char buf[10];

	fd =  open(BACKLIGHT_BRIGHTNESS, O_RDWR);
	if(fd < 0) {
		printf("[PowerManager] open BACKLIGHT_BRIGHTNESS fail\n");
		return -1;
	}
	memset(buf, 0, sizeof(buf));
	ret = read(fd, buf, sizeof(buf) - 1);
	if(ret <= 0)
		printf("[PowerManager] read Error (%s),ret=%d,fd=%d \n", strerror(errno), ret, fd);

	close(fd);
	return atoi(buf);
}

static int set_backlight_level(int level)
{
	int fd = 0;
	int ret;
	char buf[10]={0};

	fd =  open(BACKLIGHT_BRIGHTNESS, O_RDWR);
	if(fd < 0) {
		printf("[PowerManager] open BACKLIGHT_BRIGHTNESS fail\n");
		return -1;
	}

	memset(buf, 0x0, sizeof(buf));
	sprintf(buf, "%d", level);
	ret = write(fd, buf, strlen(buf));
	if(ret <= 0)
	{
		printf("[PowerManager] write Error (%s),ret=%d,fd=%d \n", strerror(errno), ret, fd);
	}
	else
	{
		if(level != 0)
		{
			mBacklight_level = level;
		}
	}
	close(fd);
	return (ret > 0) ? 0 : -1;
}

static int pm_get_backlight_level(void)
{
	int fd = 0;
	int ret;
	char buf[10]={0};

	fd =  open(BACKLIGHT_BRIGHTNESS, O_RDWR);
	if(fd < 0) {
		printf("[PowerManager] open BACKLIGHT_BRIGHTNESS fail\n");
		return -1;
	}

	memset(buf, 0, sizeof(buf));
	ret = read(fd, buf, sizeof(buf) - 1);
	if(ret <= 0)
	{
		printf("[PowerManager] write Error (%s),ret=%d,fd=%d \n", strerror(errno), ret, fd);
	}
	close(fd);

	//mBacklight_level = atoi(buf);

	printf("[PowerManager] pm_get_backlight_level backlight=%d \n", atoi(buf));
	return atoi(buf);
}

int backlightOff(void)
{

	set_backlight_level(0);

	return 0;
}

int backlightOn(void)
{
	printf("[PowerManager] backlightOn mBacklight_level=%d \n", mBacklight_level);
	//pm_get_backlight_level();
	if (mBacklight_level != 0) {
		set_backlight_level(mBacklight_level);
	}

	return 0;
}

int get_screen_status(void)
{
	return mState;
}

int screenOff(void)
{
	int retval = 0;
	int ret = 0;
	int fd = 0;

	printf("[PowerManager]  screenOff \n");

	fd = open(DISP_DEV, O_RDWR);
	ret = 0;
	backlightOff();
	retval = ioctl(fd, FBIOBLANK, FB_BLANK_POWERDOWN);
	if (retval < 0) {
		printf("[PowerManager] fail to set screen on\n");
		ret = -1;
	}
	close(fd);

	mState = SCREEN_OFF;
	return ret;
}

int screenOn(void)
{
	int retval = 0;
	int ret = 0;
	int fd = 0;

	printf("[PowerManager]  ScreenOn \n");
	fd = open(DISP_DEV, O_RDWR);
	ret = 0;
	retval = ioctl(fd, FBIOBLANK, FB_BLANK_UNBLANK);
	if (retval < 0) {
		printf("[PowerManager] fail to set screen on\n");
		ret = -1;
	}
	close(fd);

	backlightOn();

	mState = SCREEN_ON;
	return 0;
}

int pm_power_init(void)
{
	int backlight_level, mBacklight_status;

	if (!pm_reg_lock_init) {
		pthread_mutex_init(&g_pm_reg_lock, NULL);
		pm_reg_lock_init = 1;
	}

	mBacklight_level = get_def_backlight_level();
	printf("[PowerManager] pm_power_init mBacklight_level=%d \n", mBacklight_level);

}

int pm_register(char *module_name, enum sleep_in_priority slp_in_priority,
		int (*get_state_cb)(void*), void* get_state_cb_arg,
		int (*sleep_in_cb)(void*), void* sleep_in_cb_arg,
		int (*sleep_out_cb)(void*, int), void* sleep_out_cb_arg)
{
	struct pm_reg_t *reg_node;
	struct pm_reg_t *tmp_n = NULL;

	if (!module_name || !get_state_cb || !sleep_in_cb || !sleep_out_cb) {
		printf("[PowerManager] %s, arg error\n", __func__);
		return -1;
	}
	reg_node = (struct pm_reg_t *)malloc(sizeof(struct pm_reg_t));
	if (!reg_node) {
		printf("[PowerManager] %s, malloc reg_node faild\n", __func__);
		return -1;
	}
	memset(reg_node, 0, sizeof(struct pm_reg_t));
	nl_init_list_head(&reg_node->list);
	strncpy(reg_node->module_name, module_name, 31);
	reg_node->flag |= (slp_in_priority & 0xff);
	reg_node->get_state_cb = get_state_cb;
	reg_node->get_state_cb_arg = get_state_cb_arg;
	reg_node->sleep_in_cb = sleep_in_cb;
	reg_node->sleep_in_cb_arg = sleep_in_cb_arg;
	reg_node->sleep_out_cb = sleep_out_cb;

	pm_reg_lock();
	if (nl_list_empty(&g_pm_reg_list)) {
		nl_list_add_tail(&reg_node->list, &g_pm_reg_list);
		pm_reg_unlock();
		return 0;
	}
	pm_list_for_each_reverse_entry(tmp_n, &g_pm_reg_list, list) {
		if (slp_in_priority >= (tmp_n->flag & 0xff))
			break;
	}
	if (tmp_n)
		nl_list_add_tail(&reg_node->list, &tmp_n->list);
	else
		nl_list_add_head(&reg_node->list, &g_pm_reg_list);
	pm_reg_unlock();

	return 0;
}

int pm_unregister(char *module_name)
{
	struct pm_reg_t *tmp_n = NULL, *n;

	if (!module_name)
		return -1;

	pm_reg_lock();
	if (nl_list_empty(&g_pm_reg_list)) {
		pm_reg_unlock();
		return 0;
	}

	nl_list_for_each_entry_safe(tmp_n, n, &g_pm_reg_list, list) {
		if (!strcmp(tmp_n->module_name, module_name)) {
			nl_list_del(&tmp_n->list);
			free(tmp_n);
		}
	}
	pm_reg_unlock();

	return 0;
}

int pm_sleep(void)
{
	struct pm_reg_t *tmp_n = NULL;
	int state;

	pm_reg_lock();
	if (nl_list_empty(&g_pm_reg_list)) {
		pm_reg_unlock();
		systemSuspend();
		return 0;
	}

	pm_list_for_each_reverse_entry(tmp_n, &g_pm_reg_list, list) {
		state = (int)(tmp_n->get_state_cb(tmp_n->get_state_cb_arg));
		tmp_n->flag |= (state << 8);
		tmp_n->sleep_in_cb(tmp_n->sleep_in_cb_arg);
	}
	pm_reg_unlock();
	systemSuspend();

	return 0;
}

int pm_wakeup(void)
{
	struct pm_reg_t *tmp_n = NULL;
	int state;

	systemResume();
	pm_reg_lock();
	if (nl_list_empty(&g_pm_reg_list)) {
		pm_reg_unlock();
		return 0;
	}

	nl_list_for_each_entry(tmp_n, &g_pm_reg_list, list) {
		state = (tmp_n->flag & 0xff00) >> 8;
		tmp_n->sleep_out_cb(tmp_n->sleep_out_cb_arg, state);
		state = (int)(tmp_n->get_state_cb(tmp_n->get_state_cb_arg));
		tmp_n->flag |= (state << 8);
	}
	pm_reg_unlock();

	return 0;
}
ssize_t pm_uevent_kernel_multicast_uid_recv(int socket, void *buffer,
                                            size_t length, uid_t *user);

/**
 * Like recv(), but checks that messages actually originate from the kernel.
 */
ssize_t pm_uevent_kernel_multicast_recv(int socket, void *buffer, size_t length)
{
	uid_t user = -1;
	return pm_uevent_kernel_multicast_uid_recv(socket, buffer, length,
						   &user);
}

/**
 * Like the above, but passes a uid_t in by reference. In the event that this
 * fails due to a bad uid check, the uid_t will be set to the uid of the
 * socket's peer.
 *
 * If this method rejects a netlink message from outside the kernel, it
 * returns -1, sets errno to EIO, and sets "user" to the UID associated with the
 * message. If the peer UID cannot be determined, "user" is set to -1."
 */
ssize_t pm_uevent_kernel_multicast_uid_recv(int socket, void *buffer,
					    size_t length, uid_t *user)
{
	struct iovec iov = { buffer, length };
	struct sockaddr_nl addr;
	char control[CMSG_SPACE(sizeof(struct ucred))];
	struct msghdr hdr = {
		&addr,
		sizeof(addr),
		&iov,
		1,
		control,
		sizeof(control),
		0,
	};

	*user = -1;
	ssize_t n = recvmsg(socket, &hdr, 0);
	if (n <= 0) {
		return n;
	}

	struct cmsghdr *cmsg = CMSG_FIRSTHDR(&hdr);
	if (cmsg == NULL || cmsg->cmsg_type != SCM_CREDENTIALS) {
		/* ignoring netlink message with no sender credentials */
		goto out;
	}

	struct ucred *cred = (struct ucred *)CMSG_DATA(cmsg);
	*user = cred->uid;
	if (cred->uid != 0) {
		/* ignoring netlink message from non-root user */
		goto out;
	}

	if (addr.nl_groups == 0 || addr.nl_pid != 0) {
		/* ignoring non-kernel or unicast netlink message */
		goto out;
	}

	return n;

out:
	/* clear residual potentially malicious data */
	bzero(buffer, length);
	errno = EIO;
	return -1;
}

int pm_uevent_open_socket(int buf_sz, bool passcred)
{
	struct sockaddr_nl addr;
	int on = passcred;
	int s;

	memset(&addr, 0, sizeof(addr));
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = getpid();
	addr.nl_groups = 0xffffffff;

	s = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	if(s < 0)
		return -1;

	setsockopt(s, SOL_SOCKET, SO_RCVBUFFORCE, &buf_sz, sizeof(buf_sz));
	setsockopt(s, SOL_SOCKET, SO_PASSCRED, &on, sizeof(on));

	if(bind(s, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		close(s);
		return -1;
	}

	return s;
}

static void uevent_event(void)
{
	char msg[UEVENT_MSG_LEN + 2]={0};
	char *cp;
	int n;

	memset(msg, 0x0, sizeof(msg));
	n = pm_uevent_kernel_multicast_recv(pm_uevent.uevent_fd, msg,
					    UEVENT_MSG_LEN);
	if (n <= 0)
		return;
	if (n >= UEVENT_MSG_LEN)   /* overflow -- discard */
		return;

	msg[n] = '\0';
	msg[n+1] = '\0';
	cp = msg;

	//printf("[PowerManager]: uevent: %s\n", cp);
	while (*cp) {
		pm_thermal_uevent_check(cp);

		/* advance to after the next \0 */
		while (*cp++)
			;
	}
}

static void* ueventd_thread_func(void *arg)
{
	struct epoll_event events;
	int nevents;

	while(1) {
		nevents = epoll_wait(pm_uevent.epoll_fd, &events, 1, -1);
		if (nevents == -1) {
			if (errno == EINTR)
				continue;
			printf("[PowerManager] epoll_wait failed, be notice!\n");
			//break;
			continue;
		}
		(*(void (*)(int))events.data.ptr)(events.events);
	}

	printf("[PowerManager] ueventd_thread_func exit unexpectedly ! \n");
	return NULL;
}

static int uevent_init(void)
{
	struct epoll_event ev;
	int ret;

	pm_uevent.epoll_fd = epoll_create(MAX_EPOLL_EVENTS);
	pm_uevent.uevent_fd = pm_uevent_open_socket(64 * 1024, true);
	if (pm_uevent.epoll_fd == -1 || pm_uevent.uevent_fd < 0) {
		printf("[PowerManager] epoll_create failed(%s)\n", strerror(errno));
		return -1;
	}

	fcntl(pm_uevent.uevent_fd, F_SETFL, O_NONBLOCK);
	ev.events = EPOLLIN;
	ev.data.ptr = (void *)uevent_event;
	ret = epoll_ctl(pm_uevent.epoll_fd, EPOLL_CTL_ADD, pm_uevent.uevent_fd, &ev);
	if (ret < 0) {
		printf("[PowerManager] epoll_ctl failed(%s)\n", strerror(errno));
		return -1;
	}

	ret = pthread_create(&pm_uevent.ueventd_thread, NULL, ueventd_thread_func, NULL);
	if (ret)
		printf( "[PowerManager] create pthread failed(%s)\n", strerror(errno));

	return ret;
}

void pm_dump_register_list(void)
{
	struct pm_reg_t *tmp_n = NULL;
	int state;

	pm_reg_lock();
	if (nl_list_empty(&g_pm_reg_list)) {
		printf("[PowerManager] g_pm_reg_list is empty\n");
		pm_reg_unlock();
		return;
	}

	nl_list_for_each_entry(tmp_n, &g_pm_reg_list, list) {
		state = (tmp_n->flag & 0xff00) >> 8;
		printf("[PowerManager] module_name=%s, level=%d, state=%d\n",
		       tmp_n->module_name, tmp_n->flag&0xff, state);
	}
	pm_reg_unlock();
}

int pm_init(void)
{
	printf("[PowerManager]: pm_init\n");
	pm_power_init();
	pm_thermal_parse();
	pm_stop_kernel_thermal();
	uevent_init();

	return 0;
}

