#ifndef APPI18N_H
#define APPI18N_H

#include <QObject>
#include <QMap>
#include <QSettings>
#include <QFileSystemWatcher>

#include "application.h"

class AppI18n : public QObject
{
    Q_OBJECT
public:
    explicit AppI18n(QObject *parent = 0);
    static AppI18n*  instance(){
        if(s_instance == 0){
           s_instance=new AppI18n(0);
        }
        return s_instance;
    }
private:
    QMap<Application*,QSettings*> m_map;
    QFileSystemWatcher m_watcher;
    QSettings* m_i18nSetting;
    static AppI18n *s_instance;
    bool m_reflush;

    QSettings* getSettings();
public:
    void reflush();
    QString getName(Application* app);
    void addApplication(Application*,QSettings*);
    void removeApplication(Application*);
signals:
    void reflushUI();

};

#endif // APPI18N_H
