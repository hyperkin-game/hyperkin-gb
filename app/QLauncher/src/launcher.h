#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <QObject>

class Launcher : public QObject
{
    Q_OBJECT
public:
    explicit Launcher(QObject *parent = 0);

    Q_INVOKABLE void pickWallpaper(QString path);
    Q_INVOKABLE void emitNewIntent();
    Q_INVOKABLE void registerMethods();
    Q_INVOKABLE void minimize();
    Q_INVOKABLE void powerControl();

signals:
    void newIntentReceived();
    void minimized();

private:
    void registerNativeMethods();
private slots:
    void slot_standby();

};

#endif // LAUNCHER_H
