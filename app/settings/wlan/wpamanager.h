#ifndef WPAMANAGER_H
#define WPAMANAGER_H

#include <QObject>
#include <QTimer>
#include <QSocketNotifier>
#include <QMutex>

#include "wpamsg.h"

#define CTRL_EVENT_CONNECTING "Trying to associate with"

enum WifiState {
    WIFI_STATE_NULL = 0,
    WIFI_STATE_SAVED,
    WIFI_STATE_AUTH_FAILED,
    WIFI_STATE_CONNECTING,
    WIFI_STATE_CONNECTED,
};

struct netWorkItem
{
    QString ssid;
    QString bssid;
    QString frequence;
    QString signal;
    QString flags;
    int networkId = -1;
    WifiState state = WIFI_STATE_NULL;
};

class WPAManager : public QObject
{
    Q_OBJECT
public:
    static WPAManager* getInstance(QObject *parent = 0);
    ~WPAManager();

    virtual int ctrlRequest(const char *cmd, char *buf, size_t *buflen);

    int openCtrlConnection(const char *ifname);
    virtual int setNetworkParam(int id, const char *field,
                                const char *value, bool quote);
    void connectNetwork(const QString &ssid, const QString &password);
    virtual void selectNetwork(const QString &sel);
    void removeNetwork(int networkId);

    QList<netWorkItem> getConfiguredNetWork();
    bool getConnectedItem(netWorkItem *connectedItem);

    void closeWPAConnection();
    void startWPAConnection();

private:
    static WPAManager *_instance;
    WPAManager(QObject *parent = 0);

    QMutex m_connMutex;

    struct wpa_ctrl *ctrl_conn;
    struct wpa_ctrl *monitor_conn;
    QSocketNotifier *msgNotifier;
    QTimer *timer;
    char *ctrl_iface;
    char *ctrl_iface_dir;
    WpaMsgList msgs;

    QTimer *scanResultUpdateTimer;

    void updateScanResult();
    QString getConnectingSSIDFromMsg(const char *msg);
    QString getFailedSSIDFromMsg(const char *msg);
    QString getConnectedBSSIDFromMsg(const char *msg);
    QString getDisconnetedBSSIDFromMsg(const char *msg);

public slots:
    virtual void updateScanResultIfNecessary();
    virtual void ping();
    virtual void processMsg(char *msg);
    virtual void receiveMsgs();
    virtual void scan();

signals:
    void sig_scanResultAvailable(QList<netWorkItem>);
    void sig_eventConnecting(QString ssid);
    void sig_eventConnectFail(QString ssid);
    void sig_eventConnectComplete(QString bssid);
    void sig_eventDisconnected(QString bssid);
};

#endif // WPAMANAGER_H
