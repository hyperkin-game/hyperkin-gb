#include "appi18n.h"
#include <QDir>
#include <QSettings>
#include <QDebug>
#include "applicationManager.h"

AppI18n* AppI18n::s_instance =  0;

AppI18n::AppI18n(QObject *parent) : QObject(parent)
{
}

QSettings* AppI18n::getSettings(){
    QDir settingsDir("/data/");

    if(settingsDir.exists()){
        return new QSettings("/data/i18n.ini", QSettings::IniFormat);

    }else{
        return new QSettings("/etc/i18n.ini", QSettings::IniFormat);
    }
}

QString AppI18n::getName(Application* app){

    const int langTokenLength = 2; /*FIXME: is checking two chars enough?*/
    QSettings* setting= m_map.value(app,NULL);
    QVariant defaultLang(QLocale::system().name()/*"zh_CN.UTF-8"*/);
    if(setting!=NULL){
        QSettings* tmp= getSettings();
	 QString lang= tmp->value("LANG",defaultLang).toString().leftRef(langTokenLength)+"/appName";
        delete tmp;
        return setting->value(lang).toString();
    }
}

void AppI18n::reflush(){

    const int langTokenLength = 2; /*FIXME: is checking two chars enough?*/
    QMapIterator<Application*,QSettings*> iter(m_map);
    QVariant defaultLang(QLocale::system().name()/*"zh_CN.UTF-8"*/);
    while(iter.hasNext())
    {
        iter.next();
        QSettings* tmp= getSettings();
        QString lang= tmp->value("LANG",defaultLang).toString().leftRef(langTokenLength)+"/appName";
        QString name= iter.value()->value(lang).toString();
        iter.key()->setUiName(name);
        delete tmp;
    }

    emit reflushUI();

}

void AppI18n::addApplication(Application *app,QSettings *setting){
    if(!m_map.contains(app)){
        m_map.insert(app,setting);
        QString name= getName(app);
        if(name!=""){
            app->setUiName(name);
        }

    }
}

void AppI18n::removeApplication(Application *app){
    if(m_map.contains(app)){
        m_map.remove(app);
    }
}
