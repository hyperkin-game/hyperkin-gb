
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <linux/input.h>
#include "PowerManager.h"

#define KEY_POWER            116

static char dev_path[20] = "/dev/input/event2";

static void* key_event_handler(void *arg)
{
	int keys_fd;
    struct input_event t;

	printf("[PowerManager] key_event_handler enter! \n");

	keys_fd=open(dev_path, O_RDONLY);
    if(keys_fd <= 0)
    {
        printf("[PowerManager] open %s device error!\n", dev_path);
        return NULL;
    }
    while(1)
    {
        if(read(keys_fd, &t, sizeof(t)) == sizeof(t))
        {
                printf("[PowerManager] key_event_handler key %d %s\n", t.code, (t.value) ? "Pressed" : "Released");
                if( t.value==0 && t.code==KEY_POWER)
                {
                	printf("[PowerManager] key_event_handler key Power\n");
                    if(SCREEN_ON == get_screen_status())
				    {
				        printf("[PowerManager] key value is PowerKey, screen off \n");
				        screenOff();
				        system_suspend_immediately();
				    }
				    else
				    {
				        printf("[PowerManager] key value is PowerKey, screen ON \n");
				        screenOn();
				    }
                }
        }
    }
    close(keys_fd);

	printf("[PowerManager] key_event_handler exit unexpectedly ! \n");
	return NULL;
}


static void find_powerkey_devpath()
{
	int id = 0, event_name_fd;
	char event_name_path[40];
	char event_name[32];
	for(; id < 5; ++id) {
		memset(event_name_path, 0, sizeof(event_name_path));
		memset(event_name, 0, sizeof(event_name_path));
		sprintf(event_name_path, "/sys/class/input/event%d/device/name", id);
		event_name_fd = open(event_name_path, O_RDONLY);
		if(read(event_name_fd, &event_name, sizeof(event_name)) != -1 && (strstr(event_name, "rk816_pwrkey") || strstr(event_name, "rk8xx_pwrkey") || strstr(event_name, "gpio-keys") || strstr(event_name, "rk29-keypad"))) {
			memset(dev_path, 0, sizeof(dev_path));
			sprintf(dev_path, "/dev/input/event%d", id);
			close(event_name_fd);
			break;
		}
		close(event_name_fd);
	}

	printf("[PowerManager] find_powerkey_devpath dev_path:%s \n", dev_path);
}

int main(int argc, char **argv)
{
	pthread_t key_event_thread;
	int ret = 0;
	find_powerkey_devpath();
	pm_init();

	ret = pthread_create(&key_event_thread, NULL, key_event_handler, NULL);
	if (ret)
	{
		printf( "[PowerManager] create key_event_thread failed \n");
	}

    while(1)
    {
    	sleep(5);
		continue;
    }

 	printf("[PowerManager] exit unexpectedly ! \n");

	return 0;
}
