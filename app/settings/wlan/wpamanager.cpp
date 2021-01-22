#include "wpamanager.h"
#include "wpa_supplicant-2.5/src/common/wpa_ctrl.h"

#include <dirent.h>

WPAManager *WPAManager::_instance = NULL;

WPAManager* WPAManager::getInstance(QObject *parent)
{
    if (!_instance)
        _instance = new WPAManager(parent);

    return _instance;
}

WPAManager::WPAManager(QObject *parent) : QObject(parent)
{
    ctrl_conn = NULL;
    ctrl_iface = NULL;
    monitor_conn = NULL;
    msgNotifier = NULL;
    ctrl_iface_dir = strdup("/var/run/wpa_supplicant");

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(ping()));
    timer->setSingleShot(false);
    timer->start(1000);

    // Execute "scan" every 20 seconds when wpa_state in INACTIVE or COMPLETED.
    scanResultUpdateTimer = new QTimer(this);
    scanResultUpdateTimer->setInterval(20000);
    connect(scanResultUpdateTimer, SIGNAL(timeout()), SLOT(updateScanResultIfNecessary()));
    scanResultUpdateTimer->start();

    if (openCtrlConnection(ctrl_iface) < 0) {
        qDebug("Failed to open control connection to "
               "wpa_supplicant.");
    }
}

int WPAManager::openCtrlConnection(const char *ifname)
{
    m_connMutex.lock();

    char *cfile;
    int flen;

    if (ifname) {
        if (ifname != ctrl_iface) {
            free(ctrl_iface);
            ctrl_iface = strdup(ifname);
        }
    } else {
        struct dirent *dent;
        DIR *dir = opendir(ctrl_iface_dir);
        free(ctrl_iface);
        ctrl_iface = NULL;
        if (dir) {
            while ((dent = readdir(dir))) {
#ifdef _DIRENT_HAVE_D_TYPE
                /* Skip the file if it is not a socket.
                 * Also accept DT_UNKNOWN (0) in case
                 * the C library or underlying file
                 * system does not support d_type. */
                if (dent->d_type != DT_SOCK &&
                        dent->d_type != DT_UNKNOWN)
                    continue;
#endif /* _DIRENT_HAVE_D_TYPE */

                if (strcmp(dent->d_name, ".") == 0 ||
                        strcmp(dent->d_name, "..") == 0)
                    continue;
                qDebug("Selected interface '%s'",
                       dent->d_name);
                ctrl_iface = strdup(dent->d_name);
                break;
            }
            closedir(dir);
        }
    }

    if (ctrl_iface == NULL) {
        goto _exit_failed;
    }

    flen = strlen(ctrl_iface_dir) + strlen(ctrl_iface) + 2;
    cfile = (char *) malloc(flen);
    if (cfile == NULL) {
        goto _exit_failed;
    }
    snprintf(cfile, flen, "%s/%s", ctrl_iface_dir, ctrl_iface);

    if (ctrl_conn) {
        wpa_ctrl_close(ctrl_conn);
        ctrl_conn = NULL;
    }

    if (monitor_conn) {
        delete msgNotifier;
        msgNotifier = NULL;
        wpa_ctrl_detach(monitor_conn);
        wpa_ctrl_close(monitor_conn);
        monitor_conn = NULL;
    }

    ctrl_conn = wpa_ctrl_open(cfile);
    if (ctrl_conn == NULL) {
        free(cfile);
        goto _exit_failed;
    }
    monitor_conn = wpa_ctrl_open(cfile);
    free(cfile);
    if (monitor_conn == NULL) {
        wpa_ctrl_close(ctrl_conn);
        goto _exit_failed;
    }
    if (wpa_ctrl_attach(monitor_conn)) {
        qDebug("Failed to attach to wpa_supplicant");
        wpa_ctrl_close(monitor_conn);
        monitor_conn = NULL;
        wpa_ctrl_close(ctrl_conn);
        ctrl_conn = NULL;
        goto _exit_failed;
    }

    msgNotifier = new QSocketNotifier(wpa_ctrl_get_fd(monitor_conn),
                                      QSocketNotifier::Read, this);
    connect(msgNotifier, SIGNAL(activated(int)), SLOT(receiveMsgs()));

    goto _exit_normal;

_exit_failed:
    m_connMutex.unlock();
    return -1;

_exit_normal:
    m_connMutex.unlock();
    return 0;
}

static int str_match(const char *a, const char *b)
{
    return strncmp(a, b, strlen(b)) == 0;
}

QString WPAManager::getConnectingSSIDFromMsg(const char *msg)
{
    QString message(msg);

    int beginIndex = message.indexOf("SSID=") + 6;
    if (beginIndex == -1)
        return QString("");

    message = message.mid(beginIndex);
    int endIndex = message.indexOf("freq=") - 2;
    if (endIndex == -1)
        return QString("");

    return message.left(endIndex);
}

QString WPAManager::getFailedSSIDFromMsg(const char *msg)
{
    QString message(msg);

    int beginIndex = message.indexOf("ssid=") + 6;
    if (beginIndex == -1)
        return QString("");

    message = message.mid(beginIndex);
    int endIndex = message.indexOf("auth_failures=") - 2;
    if (endIndex == -1)
        return QString("");

    return message.left(endIndex);
}

QString WPAManager::getConnectedBSSIDFromMsg(const char *msg)
{
    QString message(msg);

    int beginIndex = message.indexOf("Connection to") + 14;
    if (beginIndex == -1)
        return QString("");

    message = message.mid(beginIndex);
    int endIndex = message.indexOf("completed [id=") - 1;
    if (endIndex == -1)
        return QString("");

    return message.left(endIndex);
}

QString WPAManager::getDisconnetedBSSIDFromMsg(const char *msg)
{
    QString message(msg);

    int beginIndex = message.indexOf("bssid=") + 6;
    if (beginIndex == -1)
        return QString("");

    message = message.mid(beginIndex);
    int endIndex = message.indexOf("reason=") - 1;
    if (endIndex == -1)
        return QString("");

    return message.left(endIndex);
}

void WPAManager::processMsg(char *msg)
{
    char *pos = msg, *pos2;
    int priority = 2;

    if (*pos == '<') {
        /* skip priority */
        pos++;
        priority = atoi(pos);
        pos = strchr(pos, '>');
        if (pos)
            pos++;
        else
            pos = msg;
    }

    WpaMsg wm(pos, priority);

    msgs.append(wm);
    while (msgs.count() > 100)
        msgs.pop_front();

    /* Update last message with truncated version of the event */
    if (strncmp(pos, "CTRL-", 5) == 0) {
        pos2 = strchr(pos, str_match(pos, WPA_CTRL_REQ) ? ':' : ' ');
        if (pos2)
            pos2++;
        else
            pos2 = pos;
    } else
        pos2 = pos;
    QString lastmsg = pos2;
    lastmsg.truncate(40);

    if (str_match(pos, WPA_EVENT_SCAN_RESULTS)) {
        updateScanResult();
    } else if (str_match(pos, CTRL_EVENT_CONNECTING)) {
        emit sig_eventConnecting(getConnectingSSIDFromMsg(pos));
    } else if (str_match(pos, WPA_EVENT_TEMP_DISABLED)) {
        emit sig_eventConnectFail(getFailedSSIDFromMsg(pos));
        scan();
    } else if (str_match(pos, WPA_EVENT_CONNECTED)) {
        emit sig_eventConnectComplete(getConnectedBSSIDFromMsg(pos));
        scan();
    } else if (str_match(pos, WPA_EVENT_DISCONNECTED)) {
        emit sig_eventDisconnected(getDisconnetedBSSIDFromMsg(pos));
        scan();
    }
}

void WPAManager::receiveMsgs()
{
    char buf[256];
    size_t len;

    while (monitor_conn && wpa_ctrl_pending(monitor_conn) > 0) {
        len = sizeof(buf) - 1;
        if (wpa_ctrl_recv(monitor_conn, buf, &len) == 0) {
            buf[len] = '\0';
            processMsg(buf);
        }
    }
}

void WPAManager::updateScanResult()
{
    char reply[2048];
    size_t reply_len;
    int index;
    char cmd[20];
    QList<netWorkItem> netWorksList;

    index = 0;
    while (true) {
        snprintf(cmd, sizeof(cmd), "BSS %d", index++);
        if (index > 1000)
            break;

        reply_len = sizeof(reply) - 1;
        if (ctrlRequest(cmd, reply, &reply_len) < 0)
            break;
        reply[reply_len] = '\0';

        QString bss(reply);
        if (bss.isEmpty() || bss.startsWith("FAIL"))
            break;

        QString ssid, bssid, freq, signal, flags;

        QStringList lines = bss.split(QRegExp("\\n"));

        for (QStringList::Iterator it = lines.begin();it != lines.end(); it++) {
            int pos = (*it).indexOf('=') + 1;
            if (pos < 1)
                continue;

            if ((*it).startsWith("bssid="))
                bssid = (*it).mid(pos);
            else if ((*it).startsWith("freq="))
                freq = (*it).mid(pos);
            else if ((*it).startsWith("level="))
                signal = (*it).mid(pos);
            else if ((*it).startsWith("flags="))
                flags = (*it).mid(pos);
            else if ((*it).startsWith("ssid="))
                ssid = (*it).mid(pos);
        }

        netWorkItem item;
        item.ssid = ssid;
        item.bssid = bssid;
        item.frequence = freq;
        item.signal = signal;
        item.flags = flags;

        netWorksList.append(item);

        if (bssid.isEmpty())
            break;
    }

    emit sig_scanResultAvailable(netWorksList);
}

void WPAManager::ping()
{
    char buf[10];
    size_t len;

    if (ctrl_conn != NULL) {
        timer->stop();
        return;
    }

    len = sizeof(buf) - 1;
    if (ctrlRequest("PING", buf, &len) < 0) {
        //        qDebug("PING failed - trying to reconnect");
        if (openCtrlConnection(ctrl_iface) >= 0) {
            qDebug("Reconnected successfully");
            timer->stop();
        }
    }
}

void WPAManager::scan()
{
    char reply[10];
    size_t reply_len = sizeof(reply);
    ctrlRequest("SCAN", reply, &reply_len);
}

void WPAManager::updateScanResultIfNecessary()
{
    /* get wpa_state first */
    char buf[2048], *start, *end, *pos;
    size_t len;

    len = sizeof(buf) - 1;
    if (ctrl_conn == NULL || ctrlRequest("STATUS", buf, &len) < 0) {
        qDebug("Could not get status from wpa_supplicant.");
        return;
    }

    buf[len] = '\0';
    start = buf;

    while (*start) {
        bool last = false;
        end = strchr(start, '\n');
        if (end == NULL) {
            last = true;
            end = start;
            while (end[0] && end[1])
                end++;
        }
        *end = '\0';

        pos = strchr(start, '=');
        if (pos) {
            *pos++ = '\0';
            /*
             * NOTE: do so because that the wpa_supplicant doen't execute scan when wpa_state
             * in state of 'complete' or 'inative'
             */
            if (strcmp(start, "wpa_state") == 0) {
                if (strcmp(pos, "COMPLETED") == 0 || strcmp(pos, "INACTIVE")) {
                    scan();
                    return;
                }
            }
        }

        if (last)
            break;
        start = end + 1;
    }
}

int WPAManager::setNetworkParam(int id, const char *field,
                                const char *value, bool quote)
{
    char reply[10], cmd[256];
    size_t reply_len;
    snprintf(cmd, sizeof(cmd), "SET_NETWORK %d %s %s%s%s",
             id, field, quote ? "\"" : "", value, quote ? "\"" : "");
    reply_len = sizeof(reply);
    ctrlRequest(cmd, reply, &reply_len);
    return strncmp(reply, "OK", 2) == 0 ? 0 : -1;
}

void WPAManager::selectNetwork(const QString &sel)
{
    QString cmd(sel);
    char reply[10];
    size_t reply_len = sizeof(reply);

    cmd.prepend("SELECT_NETWORK ");
    ctrlRequest(cmd.toLocal8Bit().constData(), reply, &reply_len);
    scan();
}

void WPAManager::connectNetwork(const QString &ssid, const QString &password)
{
    char reply[10], cmd[256];
    size_t reply_len;
    int id;

    memset(reply, 0, sizeof(reply));
    reply_len = sizeof(reply) - 1;

    ctrlRequest("ADD_NETWORK", reply, &reply_len);
    if (reply[0] == 'F') {
        qDebug("error: failed to add network");
        return;
    }

    id = atoi(reply);

    setNetworkParam(id, "ssid", ssid.toLocal8Bit().constData(), true);
    setNetworkParam(id, "psk", password.toLocal8Bit().constData(), true);

    selectNetwork(QString::number(id, 10));

    snprintf(cmd, sizeof(cmd), "ENABLE_NETWORK %d", id);
    ctrlRequest(cmd, reply, &reply_len);

    memset(reply, 0, sizeof(reply));
    ctrlRequest("SAVE_CONFIG", reply, &reply_len);
}

void WPAManager::removeNetwork(int networkId)
{
    char reply[10];
    size_t reply_len = sizeof(reply);

    QString cmd = QString::number(networkId);
    cmd.prepend("REMOVE_NETWORK ");
    ctrlRequest(cmd.toLocal8Bit().constData(), reply, &reply_len);

    memset(reply, 0, sizeof(reply));
    ctrlRequest("SAVE_CONFIG", reply, &reply_len);
    scan();
}

bool WPAManager::getConnectedItem(netWorkItem *connectedItem)
{
    char buf[2048], *start, *end, *pos;
    size_t len;

    len = sizeof(buf) - 1;
    if (ctrl_conn == NULL || ctrlRequest("STATUS", buf, &len) < 0) {
        qDebug("Could not get status from wpa_supplicant.");
        return false;
    }

    buf[len] = '\0';
    start = buf;

    while (*start) {
        bool last = false;
        end = strchr(start, '\n');
        if (end == NULL) {
            last = true;
            end = start;
            while (end[0] && end[1])
                end++;
        }
        *end = '\0';

        pos = strchr(start, '=');
        if (pos) {
            *pos++ = '\0';
            if (strcmp(start, "bssid") == 0) {
                connectedItem->bssid = pos;
            } else if (strcmp(start, "ssid") == 0) {
                connectedItem->ssid = pos;
            } else if (strcmp(start, "wpa_state") == 0) {
                if (strcmp(pos, "COMPLETED") == 0)
                    connectedItem->state = WIFI_STATE_CONNECTED;
            }
        }

        if (last)
            break;
        start = end + 1;
    }

    if (connectedItem->state == WIFI_STATE_CONNECTED && connectedItem->ssid != "")
        return true;
    else
        return false;
}

QList<netWorkItem> WPAManager::getConfiguredNetWork()
{
    char buf[4096], *start, *end, *id, *ssid, *bssid, *flags;
    size_t len;
    QList<netWorkItem> list;
    netWorkItem connectedItem;
    bool isConnected = getConnectedItem(&connectedItem);

    if (ctrl_conn == NULL)
        return list;

    len = sizeof(buf) - 1;
    if (ctrlRequest("LIST_NETWORKS", buf, &len) < 0)
        return list;

    buf[len] = '\0';
    start = strchr(buf, '\n');
    if (start == NULL)
        return list;
    start++;

    while (*start) {
        bool last = false;
        end = strchr(start, '\n');
        if (end == NULL) {
            last = true;
            end = start;
            while (end[0] && end[1])
                end++;
        }
        *end = '\0';

        id = start;
        ssid = strchr(id, '\t');
        if (ssid == NULL)
            break;
        *ssid++ = '\0';
        bssid = strchr(ssid, '\t');
        if (bssid == NULL)
            break;
        *bssid++ = '\0';
        flags = strchr(bssid, '\t');
        if (flags == NULL)
            break;
        *flags++ = '\0';

        if (strstr(flags, "[DISABLED][P2P-PERSISTENT]")) {
            if (last)
                break;
            start = end + 1;
            continue;
        }

        netWorkItem item;
        item.ssid = ssid;
        item.bssid = bssid;
        item.networkId = atoi(id);

        if (isConnected && item.ssid == connectedItem.ssid)
            item.state = WIFI_STATE_CONNECTED;
        else if (!isConnected && strstr(flags, "[CURRENT]"))
            item.state = WIFI_STATE_CONNECTING;
        else
            item.state = WIFI_STATE_SAVED;

        list.append(item);

        if (last)
            break;
        start = end + 1;
    }

    return list;
}

void WPAManager::closeWPAConnection()
{
    m_connMutex.lock();

    timer->stop();
    if (monitor_conn) {
        wpa_ctrl_detach(monitor_conn);
        wpa_ctrl_close(monitor_conn);
        monitor_conn = NULL;
    }

    if (ctrl_conn) {
        wpa_ctrl_close(ctrl_conn);
        ctrl_conn = NULL;
    }

    m_connMutex.unlock();
}

void WPAManager::startWPAConnection()
{
    m_connMutex.lock();

    if (ctrl_conn == NULL)
        timer->start(1000);

    m_connMutex.unlock();
}

int WPAManager::ctrlRequest(const char *cmd, char *buf, size_t *buflen)
{
    int ret;

    if (ctrl_conn == NULL)
        return -3;
    ret = wpa_ctrl_request(ctrl_conn, cmd, strlen(cmd), buf, buflen, NULL);
    if (ret == -2)
        qDebug("'%s' command timed out.", cmd);
    else if (ret < 0)
        qDebug("'%s' command failed.", cmd);

    return ret;
}

WPAManager::~WPAManager()
{
    delete msgNotifier;

    if (monitor_conn) {
        wpa_ctrl_detach(monitor_conn);
        wpa_ctrl_close(monitor_conn);
        monitor_conn = NULL;
    }

    if (ctrl_conn) {
        wpa_ctrl_close(ctrl_conn);
        ctrl_conn = NULL;
    }

    free(ctrl_iface);
    ctrl_iface = NULL;

    free(ctrl_iface_dir);
    ctrl_iface_dir = NULL;
}


