#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <libxml/tree.h>
#include "PowerManager.h"

static NL_LIST_HEAD(g_pm_zone_list);
static NL_LIST_HEAD(g_pm_sensor_list);
static NL_LIST_HEAD(g_pm_cdev_list);

struct pm_zone_cdev_map z_c_map[ZONE_CDEV_MAP_MAX] = {0};

static void pm_set_cpufreq_max(int freq)
{
	int fd, ret;
	char buf[32]={0};

	fd =  open(PM_CPU_FREQ_MAX, O_RDWR);
	if(fd < 0) {
		printf("[PowerManager] open PM_CPU_FREQ_MAX fail\n");
		return;
	}
	memset(buf, 0x0, sizeof(buf));
	sprintf(buf, "%d", freq);
	ret = write(fd, buf, strlen(buf));
	if(ret <= 0)
		printf("[PowerManager] write Error (%s),ret=%d,fd=%d \n", strerror(errno), ret, fd);
	close(fd);
}

static int pm_cpu_max_freq_control(int state, struct pm_cooling_dev_t *cdev)
{
	char cpath[256]={0};
	char *str;
	static int freq[PM_THRESHOLD_MAX]={0};
	static int get_freq = 0;
	int i, j, k;
	int ret, fd = 0;
	int level, arg;
	char buf[256]={0}, cp[16]={0};

	printf("[PowerManager] pm_cpu_max_freq_control \n");
	if (!strcmp(cdev->throttle_path, "auto"))
		str = PM_CPU_CRTL_PATH;
	else
		str = cdev->throttle_path;

	if (get_freq == 0) {
		memset(cpath, 0x0, sizeof(cpath));
		sprintf(cpath, "%s/scaling_available_frequencies", str);
		fd =  open(cpath, O_RDONLY);
		if(fd < 0) {
			printf("[PowerManager] open %s fail, err:%s\n", cpath, strerror(errno));
			return -1;
		}
		memset(buf, 0, sizeof(buf));
		ret = read(fd, buf, sizeof(buf) - 1);
		if(ret <= 0)
			printf("[PowerManager] read Error (%s),ret=%d,fd=%d \n",
			       strerror(errno), ret, fd);
		close(fd);

		j = 0;
		memset(cp, 0x0, sizeof(cp));
		for (i = 0; i < strlen(buf); i++) {
			if (isdigit(buf[i])) {
				cp[j] = buf[i];
				j++;
			} else {
				cp[j] = '\0';
				j = 0;
			}
			if (j == 0) {
				arg = atoi(cp);
				if (arg > 0) {
					for (k = PM_THRESHOLD_MAX - 1; k > 0; k--) {
						freq[k] = freq[k - 1];
					}
					freq[0] = arg;
				}
			}
		}
		for (i = 0; i < PM_THRESHOLD_MAX - 1; i++) {
			if (freq[i + 1] == 0)
				freq[i + 1] = freq[i];
		}

		get_freq = 1;
	}
	if (state <= 0)
		return 0;
	level = state - 1;
	if (cdev->throttle_enable)
		arg = cdev->throttle[level];
	else
		arg = freq[level];
	arg = freq[level];

	for(i=0; i<PM_THRESHOLD_MAX; i++)
	{
		printf("[PowerManager] check freg[%d]=%d %d\n", i, freq[i]);
	}
	printf("[PowerManager] set cpufreq max %d\n", arg);
	pm_set_cpufreq_max(arg);

	return 0;
}

static int pm_get_sensor_temp(struct pm_thermal_sonsor_t *sensor)
{
	char path[1024]={0};
	int fd = 0;
	char buf[32]={0};
	int ret;

	if (!sensor || !sensor->path || !sensor->temp)
		return -1;

	memset(path, 0x0, sizeof(path));
	sprintf(path, "%s/%s", sensor->path, sensor->temp);
	fd =  open(path, O_RDONLY);
	if(fd < 0) {
		printf("[PowerManager] open %s fail\n", path);
		return -1;
	}
	memset(buf, 0, sizeof(buf));
	ret = read(fd, buf, sizeof(buf) - 1);
	if(ret <= 0)
		printf("[PowerManager] read Error (%s),ret=%d,fd=%d \n", strerror(errno), ret, fd);

	close(fd);
	return atoi(buf);
}

static int pm_thermal_get_state(struct pm_zone_t *zone, int temp)
{
	int i, state;

	state = 0;
	for (i = 0; i < PM_THRESHOLD_MAX; i++) {
		if (zone->threshold[i] >= temp) {
			state = i + 1;
			break;
		} else if (i > 0 &&
			   zone->threshold[i - 1] == zone->threshold[i]) {
			state = i;
			break;
		}
	}
	zone->state = state;

	return state;
}

static void pm_do_cooling_dev_handler(struct pm_cooling_dev_t *cdev, int state)
{
	if (!cdev)
		return;

	if (state == cdev->state || state >= PM_THRESHOLD_MAX || state < 1)
		return;

	if (cdev->class_cb)
		cdev->class_cb(state, cdev);
	cdev->state = state;
}

static void pm_do_cooling_dev(int zone_id, int state)
{
	struct pm_cooling_dev_t *cdev = NULL;
	int i;

	if (nl_list_empty(&g_pm_cdev_list))
		return;

	for (i = 0; i < ZONE_CDEV_MAP_MAX; i++) {
		if (z_c_map[i].zoneid != zone_id)
			continue;
		nl_list_for_each_entry(cdev, &g_pm_cdev_list, list) {
			if (z_c_map[i].state && cdev->id == z_c_map[i].cooldev)
				pm_do_cooling_dev_handler(cdev, state);
		}
		break;
	}
}

static int pm_set_trip(int trip, char *path)
{
	int fd = 0;
	int ret;
	char buf[32]={0};

	fd =  open(path, O_RDWR);
	if(fd < 0) {
		printf("open %s fail\n", path);
		return -1;
	}
	memset(buf, 0x0, sizeof(buf));
	sprintf(buf, "%d", trip);
	ret = write(fd, buf, strlen(buf));
	if(ret <= 0)
		printf("[PowerManager] write Error (%s),ret=%d,fd=%d \n", strerror(errno), ret, fd);
	close(fd);
	return (ret > 0) ? 0 : -1;
}

static void pm_set_trip_point(int trip_min, int trip_max,
			      struct pm_thermal_sonsor_t *sensor)
{
	char path[1024]={0};

	printf("[PowerManager] pm_set_trip_point trip_min=%d trip_max=%d \n", trip_min, trip_max);
	printf("[PowerManager] sensor_patn=%s htemp=%s ltemp=%s \n", sensor->path, sensor->htemp,sensor->ltemp);
	if (!sensor || !sensor->path || !sensor->htemp || !sensor->ltemp)
		return;
	if (trip_min > trip_max)
		return;

	memset(path, 0x0, sizeof(path));
	sprintf(path, "%s/%s", sensor->path, sensor->ltemp);
	pm_set_trip(trip_min, path);
	memset(path, 0x0, sizeof(path));
	sprintf(path, "%s/%s", sensor->path, sensor->htemp);
	pm_set_trip(trip_max, path);
}

static void pm_thermal_update(struct pm_zone_t *zone)
{
	int temp, state;
	int trip_min, trip_max;

	printf("[PowerManager] pm_thermal_update \n");

	if (!zone || !zone->enable)
		return;
	temp = pm_get_sensor_temp(zone->sensor);
	printf("[PowerManager] pm_thermal_update temp=%d \n", temp);
	if (temp < 0)
		return;
	state = pm_thermal_get_state(zone, temp);
	if (state > 0 && zone->old_state != state) {
		pm_do_cooling_dev(zone->id, state);
		zone->old_state = state;
		if (state == 1)
			trip_min = 0;
		else
			trip_min = zone->threshold[state - 2];
		trip_max = zone->threshold[state - 1];
		pm_set_trip_point(trip_min, trip_max, zone->sensor);
	}
}

void pm_thermal_uevent_check(char *msg)
{
	struct pm_zone_t *zone = NULL;

	//printf("[PowerManager] pm_thermal_uevent_check \n");
	if (nl_list_empty(&g_pm_zone_list))
		return;
	nl_list_for_each_entry(zone, &g_pm_zone_list, list) {
		if (zone->enable && zone->sensor &&
		    strstr(msg, zone->sensor->uevent_path)) {
			pm_thermal_update(zone);
			break;
		}
	}
}

static int pm_get_xml_int(xmlDocPtr doc, const xmlNode *list)
{
	char *str;
	int ret = -1;

	str = xmlNodeListGetString(doc, list, 1);
	if (str)
		ret = atoi(str);

	xmlFree(str);
	return ret;
}

static void pm_parse_sensor(xmlDocPtr doc, xmlNodePtr cur_parent)
{
	xmlNodePtr cur;
	struct pm_thermal_sonsor_t *sensor;

	if (xmlStrcmp(cur_parent->name, (const xmlChar *)"Sensor"))
		return;

	sensor = malloc(sizeof(struct pm_thermal_sonsor_t));
	if (!sensor) {
		printf("%s: malloc sensor faild\n", __func__);
		return;
	}
	memset(sensor, 0, sizeof(struct pm_thermal_sonsor_t));
	nl_init_list_head(&sensor->list);
	nl_list_add_tail(&sensor->list, &g_pm_sensor_list);

	cur = cur_parent->xmlChildrenNode;
	while (cur != NULL) {
		if (!xmlStrcmp(cur->name, (const xmlChar *)"SensorName"))
			sensor->name = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		if (!xmlStrcmp(cur->name, (const xmlChar *)"SensorPath"))
			sensor->path= xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		if (!xmlStrcmp(cur->name, (const xmlChar *)"InputTemp"))
			sensor->temp= xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		if (!xmlStrcmp(cur->name, (const xmlChar *)"HighTemp"))
			sensor->htemp= xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		if (!xmlStrcmp(cur->name, (const xmlChar *)"LowTemp"))
			sensor->ltemp= xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		if (!xmlStrcmp(cur->name, (const xmlChar *)"UEventDevPath"))
			sensor->uevent_path= xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);

		cur = cur->next;
	}
}

static void pm_parse_zone_threshold(xmlDocPtr doc, xmlNodePtr cur_parent,
				    struct pm_zone_t *zone)
{
	xmlNodePtr cur;

	cur = cur_parent->xmlChildrenNode;
	while (cur != NULL) {
		if (!xmlStrcmp(cur->name, (const xmlChar *)"ZoneThresholdPositionOne"))
			zone->threshold[0] = pm_get_xml_int(doc, cur->xmlChildrenNode);
		if (!xmlStrcmp(cur->name, (const xmlChar *)"ZoneThresholdPositionTwo")) {
			zone->threshold[1] = pm_get_xml_int(doc, cur->xmlChildrenNode);
			if (zone->threshold[1] == -1)
				zone->threshold[1] = zone->threshold[0];
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"ZoneThresholdPositionThree")) {
			zone->threshold[2] = pm_get_xml_int(doc, cur->xmlChildrenNode);
			if (zone->threshold[2] == -1)
				zone->threshold[2] = zone->threshold[1];
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"ZoneThresholdPositionFour")) {
			zone->threshold[3] = pm_get_xml_int(doc, cur->xmlChildrenNode);
			if (zone->threshold[3] == -1)
				zone->threshold[3] = zone->threshold[2];
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"ZoneThresholdPositionFive")) {
			zone->threshold[4] = pm_get_xml_int(doc, cur->xmlChildrenNode);
			if (zone->threshold[4] == -1)
				zone->threshold[4] = zone->threshold[3];
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"ZoneThresholdPositionSix")) {
			zone->threshold[5] = pm_get_xml_int(doc, cur->xmlChildrenNode);
			if (zone->threshold[5] == -1)
				zone->threshold[5] = zone->threshold[4];
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"ZoneThresholdPositionSeven")) {
			zone->threshold[6] = pm_get_xml_int(doc, cur->xmlChildrenNode);
			if (zone->threshold[6] == -1)
				zone->threshold[6] = zone->threshold[5];
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"ZoneThresholdPositionEight")) {
			zone->threshold[7] = pm_get_xml_int(doc, cur->xmlChildrenNode);
			if (zone->threshold[7] == -1)
				zone->threshold[7] = zone->threshold[6];
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"ZoneThresholdPositionNine")) {
			zone->threshold[8] = pm_get_xml_int(doc, cur->xmlChildrenNode);
			if (zone->threshold[8] == -1)
				zone->threshold[8] = zone->threshold[7];
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"ZoneThresholdPositionTen")) {
			zone->threshold[9] = pm_get_xml_int(doc, cur->xmlChildrenNode);
			if (zone->threshold[9] == -1)
				zone->threshold[9] = zone->threshold[8];
		}
		cur = cur->next;
	}
}

static void pm_parse_sensor_attrib(xmlDocPtr doc, xmlNodePtr cur_parent,
				   struct pm_zone_t *zone)
{
	xmlNodePtr cur;

	cur = cur_parent->xmlChildrenNode;
	while (cur != NULL) {
		if (!xmlStrcmp(cur->name, (const xmlChar *)"SensorName"))
			zone->sensor_name = xmlNodeListGetString(doc,
							cur->xmlChildrenNode, 1);
		cur = cur->next;
	}
}

static void pm_parse_sensor_zone(xmlDocPtr doc, xmlNodePtr cur_parent)
{
	xmlNodePtr cur;
	struct pm_zone_t *zone;
	struct pm_thermal_sonsor_t *sensor = NULL;

	if (xmlStrcmp(cur_parent->name, (const xmlChar *)"Zone"))
		return;

	zone = malloc(sizeof(struct pm_zone_t));
	if (!zone) {
		printf("%s: malloc zone faild\n", __func__);
		return;
	}
	memset(zone, 0, sizeof(struct pm_zone_t));
	nl_init_list_head(&zone->list);
	nl_list_add_tail(&zone->list, &g_pm_zone_list);

	cur = cur_parent->xmlChildrenNode;
	while (cur != NULL) {
		if (!xmlStrcmp(cur->name, (const xmlChar *)"ZoneID"))
			zone->id= pm_get_xml_int(doc, cur->xmlChildrenNode);
		if (!xmlStrcmp(cur->name, (const xmlChar *)"ZoneName"))
			zone->name= xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		if (!xmlStrcmp(cur->name, (const xmlChar *)"ZoneThreshold"))
			pm_parse_zone_threshold(doc, cur, zone);
		if (!xmlStrcmp(cur->name, (const xmlChar *)"SensorAttrib"))
			pm_parse_sensor_attrib(doc, cur, zone);

		cur = cur->next;
	}

	if (nl_list_empty(&g_pm_sensor_list) || !zone->sensor_name)
		return;

	nl_list_for_each_entry(sensor, &g_pm_sensor_list, list) {
		if (sensor->name && !strcmp(sensor->name, zone->sensor_name)) {
			zone->sensor = sensor;
			break;
		}
	}
}

static void pm_parse_sensor_profile(xmlDocPtr doc, xmlNodePtr cur_parent)
{
	xmlNodePtr cur;
	char *name;

	if (xmlStrcmp(cur_parent->name, (const xmlChar *)"Profile"))
		return;
	cur = cur_parent->xmlChildrenNode;
	while (cur != NULL) {
		if (!xmlStrcmp(cur->name, (const xmlChar *)"Name")) {
			name =  xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
			if (name) {
				if (strcmp(name, "Performance")) {
					xmlFree(name);
					return;
				}
				xmlFree(name);
			}
		}
		pm_parse_sensor_zone(doc, cur);
		cur = cur->next;
	}
}

static int pm_thermal_parse_sensor_cfg(void)
{
	xmlDocPtr doc;
	xmlNodePtr cur;
	xmlChar *id;

	doc = xmlParseFile(THERMAL_SENSOR_CONFIG);
	if (doc == NULL) {
		printf( "[PowerManager] Failed to parse xml file: %s\n", THERMAL_SENSOR_CONFIG);
		return -1;
	}

	cur = xmlDocGetRootElement(doc);
	if (cur == NULL) {
		printf("[PowerManager] Root is empty.\n");
		goto __err;
	}

	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		pm_parse_sensor(doc, cur);
		pm_parse_sensor_profile(doc, cur);
		cur = cur->next;
	}
	xmlFreeDoc(doc);
	return 0;

__err:
	if (doc)
		xmlFreeDoc(doc);
	return -1;
}

static void pm_parse_cdev_throttle(xmlDocPtr doc, xmlNodePtr cur_parent,
				   struct pm_cooling_dev_t *c_dev)
{
	xmlNodePtr cur;

	cur = cur_parent->xmlChildrenNode;
	while (cur != NULL) {
		if (!xmlStrcmp(cur->name, (const xmlChar *)"ThrottleNormal"))
			c_dev->throttle[0] = pm_get_xml_int(doc, cur->xmlChildrenNode);
		if (!xmlStrcmp(cur->name, (const xmlChar *)"ThrottleWarning"))
			c_dev->throttle[1] = pm_get_xml_int(doc, cur->xmlChildrenNode);
		if (!xmlStrcmp(cur->name, (const xmlChar *)"ThrottleAlert"))
			c_dev->throttle[2] = pm_get_xml_int(doc, cur->xmlChildrenNode);
		if (!xmlStrcmp(cur->name, (const xmlChar *)"ThrottleCritical"))
			c_dev->throttle[3] = pm_get_xml_int(doc, cur->xmlChildrenNode);
		if (!xmlStrcmp(cur->name, (const xmlChar *)"ThrottleCriticalFinal")) {
			c_dev->throttle[4] = pm_get_xml_int(doc, cur->xmlChildrenNode);
			c_dev->throttle[9] = c_dev->throttle[8] = c_dev->throttle[7] =
				c_dev->throttle[6] = c_dev->throttle[5] = c_dev->throttle[4];
		}
		cur = cur->next;
	}
}

static void pm_cdev_class_map(struct pm_cooling_dev_t *cdev, char *class_str)
{
	if (!class_str)
		return;
	if (!strcmp(class_str, "CPUMaxFreqControl"))
		cdev->class_cb = pm_cpu_max_freq_control;
}

static void pm_parse_cooling_dev(xmlDocPtr doc, xmlNodePtr cur_parent)
{
	xmlNodePtr cur;
	char *name;
	struct pm_cooling_dev_t *c_dev;

	if (xmlStrcmp(cur_parent->name, (const xmlChar *)"ContributingDeviceInfo"))
		return;

	c_dev = malloc(sizeof(struct pm_cooling_dev_t));
	if (!c_dev) {
		printf("[PowerManager] %s: malloc c_dev faild\n", __func__);
		return;
	}
	memset(c_dev, 0, sizeof(struct pm_cooling_dev_t));
	nl_init_list_head(&c_dev->list);
	nl_list_add_tail(&c_dev->list, &g_pm_cdev_list);

	cur = cur_parent->xmlChildrenNode;
	while (cur != NULL) {
		if (!xmlStrcmp(cur->name, (const xmlChar *)"CDeviceName"))
			c_dev->name = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		if (!xmlStrcmp(cur->name, (const xmlChar *)"CDeviceID"))
			c_dev->id = pm_get_xml_int(doc, cur->xmlChildrenNode);
		if (!xmlStrcmp(cur->name, (const xmlChar *)"CDeviceClassPath")) {
			c_dev->class = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
			pm_cdev_class_map(c_dev, c_dev->class);
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"CDeviceThrottlePath"))
			c_dev->throttle_path = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		if (!xmlStrcmp(cur->name, (const xmlChar *)"ThrottleValues")) {
			c_dev->throttle_enable = 1;
			pm_parse_cdev_throttle(doc, cur, c_dev);
		}

		cur = cur->next;
	}
}

static void pm_parse_zone_cdev_map(xmlDocPtr doc, xmlNodePtr cur_parent,
				   int zone_id)
{
	xmlNodePtr cur;
	int cdev_id;
	int i;

	cur = cur_parent->xmlChildrenNode;
	while (cur != NULL) {
		if (!xmlStrcmp(cur->name, (const xmlChar *)"CoolingDevId"))
			cdev_id = pm_get_xml_int(doc, cur->xmlChildrenNode);
		cur = cur->next;
	}
	if (cdev_id == -1)
		return;
	for (i = 0; i < ZONE_CDEV_MAP_MAX; i++) {
		if (z_c_map[i].state == 0) {
			z_c_map[i].zoneid = zone_id;
			z_c_map[i].cooldev = cdev_id;
			z_c_map[i].state = 1;
			break;
		}
	}
}

static struct pm_zone_t *pm_set_zone_by_id(int zone_id)
{
	struct pm_zone_t *zone;

	if (nl_list_empty(&g_pm_zone_list))
		return NULL;
	nl_list_for_each_entry(zone, &g_pm_zone_list, list) {
		if (zone->id == zone_id)
			return zone;
	}

	return NULL;
}

static void pm_parse_zone_throttle_info(xmlDocPtr doc, xmlNodePtr cur_parent)
{
	xmlNodePtr cur;
	int zone_id;
	int critical_shutdown;
	struct pm_zone_t *zone;

	if (xmlStrcmp(cur_parent->name, (const xmlChar *)"ZoneThrottleInfo"))
		return;

	cur = cur_parent->xmlChildrenNode;
	while (cur != NULL) {
		if (!xmlStrcmp(cur->name, (const xmlChar *)"ZoneID")) {
			zone_id = pm_get_xml_int(doc, cur->xmlChildrenNode);
			zone = pm_set_zone_by_id(zone_id);
			if (zone)
				zone->enable = 1;
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"CriticalShutDown")) {
			critical_shutdown = pm_get_xml_int(doc, cur->xmlChildrenNode);
			if (zone)
				zone->critical_shutdown = 1;
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"CoolingDeviceInfo"))
			pm_parse_zone_cdev_map(doc, cur, zone_id);
		cur = cur->next;
	}
}

static void pm_parse_cdev_zone(xmlDocPtr doc, xmlNodePtr cur_parent)
{
	xmlNodePtr cur;
	char *name;

	if (xmlStrcmp(cur_parent->name, (const xmlChar *)"Profile"))
		return;

	cur = cur_parent->xmlChildrenNode;
	while (cur != NULL) {
		if (!xmlStrcmp(cur->name, (const xmlChar *)"Name")) {
			name =  xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
			if (name) {
				if (strcmp(name, "Performance")) {
					xmlFree(name);
					return;
				}
				xmlFree(name);
			}
		}
		pm_parse_zone_throttle_info(doc, cur);
		cur = cur->next;
	}
}

static int pm_thermal_parse_throttle_cfg(void)
{
	xmlDocPtr doc;
	xmlNodePtr cur;
	xmlChar *id;

	doc = xmlParseFile(THERMAL_THROTTLE_CONFIG);
	if (doc == NULL) {
		printf( "[PowerManager] Failed to parse xml file: %s\n", THERMAL_THROTTLE_CONFIG);
		return -1;
	}

	cur = xmlDocGetRootElement(doc);
	if (cur == NULL) {
		printf("[PowerManager] Root is empty.\n");
		goto __err;
	}

	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		pm_parse_cooling_dev(doc, cur);
		pm_parse_cdev_zone(doc, cur);
		cur = cur->next;
	}
	xmlFreeDoc(doc);
	return 0;

__err:
	if (doc)
		xmlFreeDoc(doc);
	return -1;
}

static void pm_thermal_test(void)
{
	struct pm_thermal_sonsor_t *sensor = NULL;
	struct pm_cooling_dev_t *cdev = NULL;
	struct pm_zone_t *zone = NULL;
	int i;

	if (nl_list_empty(&g_pm_sensor_list)) {
		printf("pm_thermal_test: sensor lis is NULL\n");
	} else {
		nl_list_for_each_entry(sensor, &g_pm_sensor_list, list) {
			if (sensor->name)
				printf("pm_thermal_test: sensor name = %s\n", sensor->name);
			if (sensor->path)
				printf("pm_thermal_test: sensor path = %s\n", sensor->path);
			if (sensor->uevent_path)
				printf("pm_thermal_test: sensor uevent_path = %s\n", sensor->uevent_path);
			if (sensor->temp)
				printf("pm_thermal_test: sensor temp = %s\n", sensor->temp);
			if (sensor->ltemp)
				printf("pm_thermal_test: sensor ltemp = %s\n", sensor->ltemp);
			if (sensor->htemp)
				printf("pm_thermal_test: sensor htemp = %s\n", sensor->htemp);
		}
	}

	if (nl_list_empty(&g_pm_cdev_list)) {
		printf("pm_thermal_test: g_pm_cdev_list is NULL\n");
	} else {
		nl_list_for_each_entry(cdev, &g_pm_cdev_list, list) {
			printf("pm_thermal_test: cdev id = %d\n", cdev->id);
			printf("pm_thermal_test: cdev throttle_enable = %d\n", cdev->throttle_enable);
			for (i = 0; i < PM_THRESHOLD_MAX; i++) {
				printf("pm_thermal_test:thr[%d] = %d\n", i, cdev->throttle[i]);
			}
			if (cdev->name)
				printf("pm_thermal_test: cdev name = %s\n", cdev->name);
			if (cdev->throttle_path)
				printf("pm_thermal_test: cdev throttle_path = %s\n", cdev->throttle_path);
			if (cdev->class)
				printf("pm_thermal_test: cdev class = %s\n", cdev->class);
		}
	}

	if (nl_list_empty(&g_pm_zone_list)) {
		printf("pm_thermal_test: g_pm_zone_list is NULL\n");
	} else {
		nl_list_for_each_entry(zone, &g_pm_zone_list, list) {
			printf("pm_thermal_test: zone id = %d\n", zone->id);
			printf("pm_thermal_test: zone enable = %d\n", zone->enable);
			printf("pm_thermal_test: zone critical_shutdown = %d\n", zone->critical_shutdown);
			for (i = 0; i < PM_THRESHOLD_MAX; i++) {
				printf("pm_thermal_test:threshold[%d] = %d\n", i, zone->threshold[i]);
			}
			if (zone->name)
				printf("pm_thermal_test: zone name = %s\n", zone->name);
			if (zone->sensor_name)
				printf("pm_thermal_test: zone name = %s\n", zone->sensor_name);
			if (zone->sensor)
				printf("pm_thermal_test: zone sensor = %p\n", zone->sensor);
		}
	}
	for (i = 0; i < ZONE_CDEV_MAP_MAX; i++) {
		if (z_c_map[i].state)
			printf("pm_thermal_test: z_c_map zone = %d, cdev = %d\n",
			       z_c_map[i].zoneid, z_c_map[i].cooldev);
	}
}

void pm_thermal_parse(void)
{
	pm_thermal_parse_sensor_cfg();
	pm_thermal_parse_throttle_cfg();
	/* pm_thermal_test(); */
	//pm_thermal_test();
}

void pm_stop_kernel_thermal(void)
{
	int fd, ret;
	char buf[3]={0};

	fd =  open(PM_KERNEL_THERMAL, O_RDWR);
	if(fd < 0) {
		printf("[PowerManager] open PM_KERNEL_THERMAL fail\n");
		return;
	}
	memset(buf, 0x0, sizeof(buf));
	sprintf(buf, "%d", 0);
	ret = write(fd, buf, strlen(buf));
	if(ret <= 0)
		printf("[PowerManager] write Error (%s),ret=%d,fd=%d \n", strerror(errno), ret, fd);
	close(fd);
}

