#ifndef WPASERVICEUTIL_H
#define WPASERVICEUTIL_H
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <qlogging.h>
#include <QProcess>

#define DBG false

#if DBG
#define DEBUG_INFO(M, ...) qDebug("DEBUG %d: " M, __LINE__, ##__VA_ARGS__)
#else
#define DEBUG_INFO(M, ...) do {} while (0)
#endif

#define DEBUG_ERR(M, ...) qDebug("DEBUG %d: " M, __LINE__, ##__VA_ARGS__)

#ifdef DEVICE_EVB
static const char WPA_SUPPLICANT_CONF_DIR[]  = "/etc/wpa_supplicant.conf";
#else
static const char WPA_SUPPLICANT_CONF_DIR[]  = "/data/wpa_supplicant.conf";
#endif

static const char HOSTAPD_CONF_DIR[] = "/etc/hostapd.conf";
static char WLAN_PID_NAME[] = "wpa_supplicant";
static char HOSTAPD_PID_NAME[] = "hostapd";
static char UDHCPC_PID_NAME[] = "udhcpc";

inline bool console_run(const char *cmdline)
{
    qDebug("consule run cmd: %s", cmdline);
    DEBUG_INFO("cmdline = %s\n",cmdline);
#if 0
    FILE *fp = popen(cmdline, "r");
    if (!fp) {
        DEBUG_ERR("Running cmdline failed: %s\n", cmdline);
        return false;
    }
    pclose(fp);
#else
    int ret;
    ret = system(cmdline);
    if (ret < 0) {
        DEBUG_ERR("Running cmdline failed: %s\n", cmdline);
    }
#endif
    return true;
}

inline void detached_run(const char *cmdline)
{
    QProcess::startDetached(QString(cmdline));
}

inline int get_pid(char *Name)
{
    int len;
    char name[20] = {0};
    len = strlen(Name);
    strncpy(name,Name,len);
    name[len] ='\0';
    char cmdresult[256] = {0};
    char cmd[20] = {0};
    FILE *pFile = NULL;
    int  pid = 0;

    sprintf(cmd, "pidof %s", name);
    pFile = popen(cmd, "r");
    if (pFile != NULL)  {
        while (fgets(cmdresult, sizeof(cmdresult), pFile)) {
            pid = atoi(cmdresult);
            DEBUG_INFO("--- %s pid = %d ---\n",name,pid);
            break;
        }
    }
    pclose(pFile);
    return pid;
}

inline int is_supplicant_running()
{
    int ret;
    ret = get_pid(WLAN_PID_NAME);
    return ret;
}

inline int is_hostapd_running()
{
    int ret;
    ret = get_pid(HOSTAPD_PID_NAME);
    return ret;
}

inline int is_udhcpc_running()
{
    int ret;
    ret = get_pid(UDHCPC_PID_NAME);
    return ret;
}

inline int creat_supplicant_file()
{
    if (access(WPA_SUPPLICANT_CONF_DIR, F_OK) < 0) {
        FILE* fp;
        fp = fopen(WPA_SUPPLICANT_CONF_DIR, "wt+");

        if (fp != 0) {
            fputs("ctrl_interface=/var/run/wpa_supplicant\n", fp);
            fputs("update_config=1\n", fp);
            fputs("ap_scan=1\n", fp);
            fclose(fp);
            return 0;
        }
        return -1;
    }
    return 0;
}

inline int creat_hostapd_file(const char* name, const char* password)
{
    FILE* fp;
    fp = fopen(HOSTAPD_CONF_DIR, "wt+");

    if (fp != 0) {
        fputs("interface=wlan0\n", fp);
        fputs("driver=nl80211\n", fp);
        fputs("ssid=", fp);
        fputs(name, fp);
        fputs("\n", fp);
        fputs("channel=6\n", fp);
        fputs("hw_mode=g\n", fp);
        fputs("ieee80211n=1\n", fp);
        fputs("ht_capab=[SHORT-GI-20]\n", fp);
        fputs("ignore_broadcast_ssid=0\n", fp);
        fputs("auth_algs=1\n", fp);
        fputs("wpa=3\n", fp);
        fputs("wpa_passphrase=", fp);
        fputs(password, fp);
        fputs("\n", fp);
        fputs("wpa_key_mgmt=WPA-PSK\n", fp);
        fputs("wpa_pairwise=TKIP\n", fp);
        fputs("rsn_pairwise=CCMP", fp);

        fclose(fp);
        return 0;
    }

    return -1;
}

inline int wifi_start_supplicant()
{
    char cmd[256];

    if (is_supplicant_running()) {
        return 0;
    }

    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "/usr/sbin/wpa_supplicant -Dnl80211 -iwlan0 -c %s &", WPA_SUPPLICANT_CONF_DIR);
    detached_run(cmd);

    return 0;
}

inline int wifi_stop_supplicant()
{
    int pid;
    char *cmd = NULL;

    /* Check whether supplicant already stopped */
    if (!is_supplicant_running())
        return 0;

    pid = get_pid(WLAN_PID_NAME);
    asprintf(&cmd, "kill %d", pid);
    console_run(cmd);
    free(cmd);

    return 0;
}

inline int wifi_start_hostapd()
{
    char cmd[256];

    if (is_hostapd_running())
        return 0;

    console_run("ifconfig wlan0 up");
    console_run("ifconfig wlan0 192.168.100.1 netmask 255.255.255.0");
    console_run("echo 1 > /proc/sys/net/ipv4/ip_forward");
    console_run("iptables --flush");
    console_run("iptables --table nat --flush");
    console_run("iptables --delete-chain");
    console_run("iptables --table nat --delete-chain");
    console_run("iptables --table nat --append POSTROUTING --out-interface eth0 -j MASQUERADE");
    console_run("iptables --append FORWARD --in-interface wlan0 -j ACCEPT");
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd,"/usr/sbin/hostapd %s -B", HOSTAPD_CONF_DIR);
    detached_run(cmd);

    return 0;
}

inline int wifi_stop_hostapd()
{
    int pid;
    char *cmd = NULL;

    if (!is_hostapd_running())
        return 0;

    pid = get_pid(HOSTAPD_PID_NAME);
    asprintf(&cmd, "kill %d", pid);
    console_run(cmd);
    free(cmd);

    console_run("echo 0 > /proc/sys/net/ipv4/ip_forward");
    console_run("ifconfig wlan0 down");
    return 0;
}

inline void lanStateChanhe(bool state)
{
    if (state) {
        console_run("ifconfig eth0 up");
        console_run("udhcpc -i eth0");
    } else {
        console_run("ifconfig eth0 down");
    }
}

inline void get_IP_address()
{
    int pid;
    char *cmd = NULL;

    if (is_udhcpc_running()) {
        pid = get_pid(UDHCPC_PID_NAME);
        asprintf(&cmd, "kill %d", pid);
        console_run(cmd);
    }

    detached_run("udhcpc -i wlan0 &");
}

#endif // WPASERVICEUTIL_H
