#ifndef APPLICATIONMANAGER
#define APPLICATIONMANAGER
#include <QProcess>
#include <QAbstractListModel>
#include "ueventthread.h"
#include "mediaMonitor/MediaMonitor.h"

class ApplicationManager : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QStringList sections READ sections NOTIFY sectionsChanged)

public:
    explicit ApplicationManager(QObject *parent = 0);

    enum Roles {
        NameRole = Qt::UserRole +1,
        PackageNameRole,
        CategoryRole
    };

    int rowCount(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role) const;

    Q_INVOKABLE void launchApplication(const QString &application,const QString &pkgName,const QString &ui_name,const QString &argv,const QString &exitCallback);
    Q_INVOKABLE void registerBroadcast();
    Q_INVOKABLE int indexOfSection(const QString &section);
    Q_INVOKABLE void openApplicationInfo(const QString &pkgName);
    Q_INVOKABLE void addApplicationToGrid(const QString &name, const QString &pkgName,const QString &ui_name, const QString &argv,const QString &icon,const QString &exitCallback);
    Q_INVOKABLE void emitAddApplication(const QString &name, const QString &pkgName,const QString &ui_name,const QString &argv,const QString &icon,const QString &exitCallback);
    Q_INVOKABLE void emitRemoveApplication(const QString &pkgName);

    QStringList sections() const;

public slots:
    void retrievePackages();
    void addApplication(const QString &name, const QString &pkgName,const QString &ui_name,const QString &argv,const QString &icon,const QString &exitCallback);
    void removeApplication(const QString &pkgName);
    void processExitCallback();
private slots:
    void slot_onReverseTriggerStateChanged(bool triggered);
protected:
    QHash<int, QByteArray> roleNames() const;

signals:
    void newApplicationDetected(const QString &name, const QString &pkgName,const QString &ui_name, const QString &argv,const QString &icon,const QString &exitCallback);
    void addedApplicationToGrid(const QString &name, const QString &pkgName,const QString &ui_name, const QString &argv,const QString &icon,const QString &exitCallback);
    void removedApplication(const QString &pkgName);
    void sectionsChanged();
    void launcherApplicationState(bool state);

private:
    QObjectList mApplications;
    QMap<QString, QObject> mApps;
    QStringList mSections;
    QList<int> mSectionsPositions;

    void init();
    void initConnection();
private:
    QProcess* pro;
    /* Uevent thread for listening car-reverse event.
       Note: it will start cvbsView application If getted gpio state with 'on'
       from uevent message and close cvbsView application with gpio state 'over' */
    UeventThread *m_ueventThread;
    QProcess *cvbsViewPro;

    MediaMonitor m_mediaMonitor;
private slots:
    void processFinished(int, QProcess::ExitStatus);
    void processError(QProcess::ProcessError);
    void cvbsViewProcessError(QProcess::ProcessError);

    void enableChildProcess();
    void disableChildProcess();
    int  getOomAdj(Q_PID pid);
    bool setOomAdj(Q_PID pid,int oom_adj);
};

#endif
