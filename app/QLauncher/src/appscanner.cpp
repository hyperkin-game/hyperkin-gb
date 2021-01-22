#include "appscanner.h"

#include <QDir>
#include <QFile>
#include "appinforeader.h"
#include <QDebug>
#include "appi18n.h"
#include <QSettings>
#include <QTextCodec>

AppScanner::AppScanner()
{

}

QList<Application*>* AppScanner::scan(QString dir_path){
    QList<Application*> *apps=new QList<Application*>();
    QDir apps_dir(dir_path);
    if(!apps_dir.exists()){
        return apps;
    }

     QFileInfoList app_directorys=apps_dir.entryInfoList(QDir::Dirs);

     for(int i=0; i<app_directorys.size();i++){
        QFileInfo appdir= app_directorys.at(i);
        QDir appHome( appdir.absoluteFilePath());
        //qDebug()<<appdir.absoluteFilePath();
        QStringList filter;
        filter<<"*.json";
        QFileInfoList appInfoList= appHome.entryInfoList(filter);
        //qDebug()<<"fileSize:"<<appInfoList.size();
        if(appInfoList.size()>0){
            QFileInfo fileInfo= appInfoList.at(0);
            //qDebug()<<fileInfo.absoluteFilePath();
            QString suffix = fileInfo.suffix();
            //qDebug()<<suffix;

                QFile file(fileInfo.absoluteFilePath());                
                file.open(QIODevice::ReadOnly);
                auto byteArray= file.readAll();
                file.close();
                AppInfoReader reader;
                Application* app= reader.fromJsonObject(byteArray);
                app->dir=fileInfo.dir().absolutePath();
                apps->append(app);

                QString i18nfilePath = fileInfo.dir().absoluteFilePath(app->i18n());
                QFile i18nfile(i18nfilePath);
                if(i18nfile.exists()){
                    //qDebug()<<"Found i18n Setting :"<<i18nfilePath;
                    QSettings *setting=new QSettings(i18nfilePath,QSettings::IniFormat);
                    setting->setIniCodec(QTextCodec::codecForName("UTF-8"));
                    AppI18n::instance()->addApplication(app,setting);
                }
        }

     }


    return apps;
}
