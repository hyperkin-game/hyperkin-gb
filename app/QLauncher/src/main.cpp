#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QSettings>
#include <QThread>
#include <QDebug>
#include "launcher.h"
#include "applicationManager.h"
#include "displayConfig.h"
#include "applicationmodel.h"

static QObject *application_manager(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    return new ApplicationManager();
}

static QObject *display_config(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    return new DisplayConfig();
}

static QObject *launcher_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    return new Launcher();
}

int main(int argc, char *argv[])
{
    QScopedPointer<QApplication> app(new QApplication(argc, argv));
    QCoreApplication::setApplicationName("QLauncher");
    QCoreApplication::setOrganizationName("Rockchip");
    QCoreApplication::setOrganizationDomain("www.rock-chips.com");

    QQmlApplicationEngine engine;

    ApplicationModel app_model;
    ApplicationModel::s_model=&app_model;
    engine.rootContext()->setContextProperty("app_model",&app_model);

    QObject::connect(&engine, SIGNAL(quit()), QCoreApplication::instance(), SLOT(quit()));

    qmlRegisterSingletonType<ApplicationManager>("com.cai.qlauncher", 1, 0, "ApplicationManager", application_manager);
    qmlRegisterSingletonType<DisplayConfig>("com.cai.qlauncher", 1, 0, "DisplayConfig", display_config);
    qmlRegisterSingletonType<Launcher>("com.cai.qlauncher", 1, 0, "Launcher", launcher_provider);
    engine.addImportPath("qrc:/qml/qml");

    engine.load(QUrl("qrc:/qml/qml/main.qml"));

    return app->exec();
}
