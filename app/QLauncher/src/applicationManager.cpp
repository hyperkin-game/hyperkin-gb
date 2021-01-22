#include "applicationManager.h"
#include "application.h"

#include <QDebug>
#include <QSettings>
#include <QApplication>
#include "appscanner.h"
#include "appi18n.h"
#include "applicationmodel.h"

#define MSG_DISABLE_APPLICATION "APP_DISABLE\n"
#define MSG_ENABLE_APPLICATION "APP_ENABLE\n"

ApplicationManager::ApplicationManager(QObject *parent) :
    QAbstractListModel(parent)
{
    init();
    initConnection();
}

void ApplicationManager::init()
{
    m_ueventThread = new UeventThread();
    m_ueventThread->start();

    FileData *fileData=(FileData*)malloc(sizeof(FileData));
    m_mediaMonitor.addFile(fileData);
    m_mediaMonitor.listen();
    connect(m_ueventThread,SIGNAL(ueventDeviceChange(int)),&m_mediaMonitor,SLOT(relisten(int)));
    // Define for control life cycle of cvbsView Application
    cvbsViewPro = new QProcess(this);
#ifdef PLATFORM_WAYLAND
    cvbsViewPro->setProgram("/usr/local/cvbsView/cvbsView -platform wayland");
#else
    cvbsViewPro->setProgram("/usr/local/cvbsView/cvbsView");
#endif

    pro = new QProcess(this);
}

void ApplicationManager::initConnection()
{
    connect(this, SIGNAL(newApplicationDetected(QString, QString,QString,QString,QString,QString)), this, SLOT(addApplication(QString,QString,QString,QString,QString,QString)));
    connect(this, SIGNAL(removedApplication(QString)), this, SLOT(removeApplication(QString)));

    connect(pro, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
    connect(pro, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));

    connect(m_ueventThread,SIGNAL(reverseTriggerStateChanged(bool)),this,SLOT(slot_onReverseTriggerStateChanged(bool)));
    connect(cvbsViewPro,SIGNAL(finished(int)),this,SLOT(enableChildProcess()));
    connect(cvbsViewPro,SIGNAL(error(QProcess::ProcessError)),this,SLOT(cvbsViewProcessError(QProcess::ProcessError)));
}

void ApplicationManager::slot_onReverseTriggerStateChanged(bool triggered)
{
    if (triggered && cvbsViewPro->state() == QProcess::NotRunning) {
        qDebug("Start cvbsView application.");
        cvbsViewPro->start();
        disableChildProcess();

    } else if (!triggered && cvbsViewPro->state() == QProcess::Running) {
        qDebug("Close cvbsView application.");
        cvbsViewPro->terminate();
        cvbsViewPro->waitForFinished();
        enableChildProcess();
    }
}

void ApplicationManager::enableChildProcess()
{
    if (pro->state() == QProcess::Running) {
        pro->write(MSG_ENABLE_APPLICATION);
    } else {
        emit launcherApplicationState(false);
    }
}

void ApplicationManager::disableChildProcess()
{
    if (pro->state() == QProcess::Running) {
        pro->write(MSG_DISABLE_APPLICATION);
    } else {
        emit launcherApplicationState(true);
    }
}

int ApplicationManager::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return mApplications.count();
}

QVariant ApplicationManager::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= mApplications.count())
        return QVariant();

    switch (role) {
    case NameRole:
        return qobject_cast<Application*>(mApplications.at(index.row()))->name();
        break;
    case PackageNameRole:
        return qobject_cast<Application*>(mApplications.at(index.row()))->pkgName();
        break;
    }
    return QVariant();
}

void ApplicationManager::launchApplication(const QString &application, const QString &pkgName,
                                           const QString &ui_name, const QString &argv,
                                           const QString &exitCallback)
{
    QStringList arguments;
#ifdef PLATFORM_WAYLAND
    qDebug() << "launchApplication(PLATFORM_WAYLAND):application=" << application<<",argv="<<argv;

    //arguments <<"-platform"<<"wayland"<<"-plugin"<<"EvdevTouch"<<"-plugin"<<"EvdevKeyboard";
    pro->start("/usr/local/"+application+"/"+pkgName,arguments);
#else
    qDebug() << "launchApplication:application=" << application << ",argv=" << argv;

    //QStringList arguments;
#ifdef DEVICE_EVB
    arguments << "-plugin" << "EvdevTouch" << "-plugin" << "EvdevKeyboard";
#endif
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if (application.compare("video") == 0 || application.compare("camera") == 0) {
        //env.insert("GST_DEBUG", "kmssink:5");//show video fps
    } else {
        //env.insert("QT_EGLFSPLATFORM_USE_GST_VIDEOSINK", "");
        env.insert("QT_GSTREAMER_WINDOW_VIDEOSINK", " "); // remove  environment variable
    }

    if (!exitCallback.isEmpty()) {
        env.insert("EXIT_CALLBACK", exitCallback);
        env.insert("APP_DIR", application);
    }

    pro->setProcessEnvironment(env);
    if (false) {
        foreach (QString str,env.toStringList())
            qDebug() <<str;

        pro->setStandardOutputFile("/tmp/" + application + "_out.log", QIODevice::Truncate);
        pro->setStandardErrorFile("/tmp/" + application + "_error.log", QIODevice::Truncate);
    }

    if (!argv.isEmpty()) {
        arguments = argv.split(" ");
        if (application.compare("cvbsView") == 0) {
            cvbsViewPro->start("/usr/local/" + application + "/" + pkgName, arguments);
        } else {
            pro->start("/usr/local/" + application + "/" + pkgName, arguments);
        }
    } else {
#ifdef DEVICE_EVB
        if (application.compare("cvbsView") == 0) {
            cvbsViewPro->start("/usr/local/" + application + "/" + pkgName, arguments);
        } else {
            pro->start("/usr/local/" + application + "/" + pkgName, arguments);
        }
#else
        if (application.compare("cvbsView") == 0) {
            cvbsViewPro->start("/usr/local/" + application + "/" + pkgName);
        } else {
            pro->start("/usr/local/" + application + "/" + pkgName);
        }
#endif
    }

    setOomAdj(pro->pid(), 2);
    //getOomAdj(pro->pid());
    emit launcherApplicationState(true);
#endif
}

QHash<int, QByteArray> ApplicationManager::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[PackageNameRole] = "pkgName";
    return roles;
}

void ApplicationManager::retrievePackages()
{
    beginResetModel();

    AppScanner *appScanner=new AppScanner();

    QList<Application*> *apps= appScanner->scan("/usr/local");

    //qDebug() << "apps->size():" << apps->size();
    for(int i=0; i< apps->size();i++){
        Application* app= apps->at(i);
        // qDebug() << "apps(" << i<<") app->mName="<<app->name()<<",pkgName="<<app->pkgName()<<"app->app_icon="<<app->icon();
        mApplications.append(new Application(app->name(), app->pkgName(),app->ui_name(),app->argv(),app->icon(),app->exitCallback()));
        emit addedApplicationToGrid(app->name(), app->pkgName(),app->ui_name(),app->argv(),app->icon(),app->exitCallback());
        emit ApplicationModel::s_model->onApplicationInsert(app);
    }
    delete appScanner;
    endResetModel();
    emit sectionsChanged();
}

void ApplicationManager::registerBroadcast()
{
    retrievePackages();
}

int ApplicationManager::indexOfSection(const QString &section)
{
    return mSectionsPositions.at(mSections.indexOf(section));
}

void ApplicationManager::openApplicationInfo(const QString &pkgName)
{

    Q_UNUSED(pkgName)
}

void ApplicationManager::addApplicationToGrid(const QString &name, const QString &pkgName,const QString &ui_name,const QString &argv,const QString &icon,const QString &exitCallback)
{
    emit addedApplicationToGrid(name, pkgName,ui_name,argv,icon,exitCallback);
}

QStringList ApplicationManager::sections() const
{
    return mSections;
}

void ApplicationManager::addApplication(const QString &name, const QString &pkgName,const QString &ui_name,const QString &argv,const QString &icon,const QString &exitCallback)
{
    int i;
    for (i = 0; i < mApplications.length(); ++i) {
        Application *application = qobject_cast<Application*>(mApplications.at(i));
        if (name.compare(application->name()) < 0)
            break;
    }

    qDebug() << Q_FUNC_INFO << "NAME:" << name << " PACKAGE:" << pkgName;
    beginInsertRows(QModelIndex(), i, i);
    Application *application = new Application(name, pkgName,ui_name,argv,icon,exitCallback);
    mApplications.insert(i, application);
    endInsertRows();
}

void ApplicationManager::removeApplication(const QString &pkgName)
{
    qDebug() << Q_FUNC_INFO << pkgName;
    int i = 0;
    foreach (QObject *object, mApplications) {
        Application *app = qobject_cast<Application*>(object);
        if (app->pkgName() == pkgName) {
            beginRemoveRows(QModelIndex(), i, i);
            delete mApplications.takeAt(i);
            endRemoveRows();
            i--;
        }
        i++;
    }

    mSections.clear();
    mSectionsPositions.clear();

    sectionsChanged();
}

void ApplicationManager::emitAddApplication(const QString &name, const QString &pkgName,const QString &ui_name,const QString &argv,const QString &icon,const QString &exitCallback)
{
    emit newApplicationDetected(name, pkgName,ui_name,argv,icon,exitCallback);
}

void ApplicationManager::emitRemoveApplication(const QString &pkgName)
{
    emit removedApplication(pkgName);
}

void ApplicationManager::processFinished(int, QProcess::ExitStatus){
    qDebug() << "processFinished" << endl;
    emit  launcherApplicationState(false);
    processExitCallback();
    pro->close();
    AppI18n::instance()->reflush();
}

void ApplicationManager::processError(QProcess::ProcessError){
    qDebug() << "processError" << endl;
/*
#ifdef PLATFORM_WAYLAND
    emit  launcherApplicationState(false);
    processExitCallback();
    pro->close();
#else
    // stop thread before restart application.
    m_mediaMonitor.stopThread();
    m_ueventThread->stopThread();
    pro->close();
    qApp->closeAllWindows();

    QStringList arguments;
#ifdef DEVICE_EVB
    arguments <<"-plugin"<<"EvdevTouch"<<"-plugin"<<"EvdevKeyboard";
#endif
    QProcess::startDetached(qApp->applicationFilePath(), arguments);

    qApp->exit(0);
#endif
*/
}

void ApplicationManager::cvbsViewProcessError(QProcess::ProcessError)
{
    qDebug() << "cvbsView processError" << endl;
    if(pro->state() == QProcess::Running) {
        qDebug("Current program: %s",qPrintable(pro->program()));
        pro->terminate();
        pro->waitForFinished();
    }

    //qApp->closeAllWindows();
    //QStringList arguments;
    //arguments <<"-platform"<<"EGLFS";
    //QProcess::startDetached(qApp->applicationFilePath(), arguments);
    //qApp->quit();
}

void ApplicationManager::processExitCallback()
{
    qDebug() << "processExitCallback" << endl;
    if(pro->processEnvironment().contains("EXIT_CALLBACK"))
    {
        QString exit_work=pro->processEnvironment().value("EXIT_CALLBACK");
        QString pro_path=pro->processEnvironment().value("APP_DIR");
        qDebug() <<"exit_work="<<exit_work<<"pro_path="<<pro_path<<":"<<"/usr/local/"+pro_path+"/"+exit_work;
        QProcess::execute("/usr/local/"+pro_path+"/"+exit_work);
    }
}

int  ApplicationManager::getOomAdj(Q_PID pid)
{
    char path[20];
    sprintf(path,"/proc/%d/oom_adj",pid);
    //qDebug() <<"path="<<path;
    int oom=-1000;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug()<<"getOomAdj:Can't open the file!"<<endl;
        return -1000;
    }

    QTextStream in(&file);
    QString line = in.readLine();
    while (!line.isNull()) {
        line = in.readLine();
    }
    oom=line.toInt();
    qDebug()<<"getOomAdj oom_adj="<<oom;
    file.close();

    return oom;

}

bool ApplicationManager::setOomAdj(Q_PID pid,int oom_adj)
{
    char path[20];
    sprintf(path,"/proc/%d/oom_adj",pid);
    qDebug() <<"setOomAdj:write "<<QString("%1").arg(oom_adj)<<":"<<path;
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug()<<"setOomAdj:Can't open the file!"<<endl;
        return false;
    }
    QTextStream out(&file);
    out <<QString("%1").arg(oom_adj);
    file.close();
    return true;
}
