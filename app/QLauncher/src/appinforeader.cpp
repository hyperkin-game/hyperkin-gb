#include "appinforeader.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QDebug>
#include "appi18n.h"

AppInfoReader::AppInfoReader()
{

}

QList<Application*>* AppInfoReader::readFromPath(QString fileName){


    QFile file(fileName);
   file.open(QIODevice::ReadOnly);
   auto byteArray= file.readAll();
   file.close();


    return loadFromJsonArray(byteArray);
}
Application* AppInfoReader::fromJsonObject(QByteArray byteArray){
    Application * appInfo=new Application();
    QJsonParseError json_error;
    QJsonDocument document=QJsonDocument::fromJson(byteArray,&json_error);
    if(json_error.error==QJsonParseError::NoError)
    {
        QJsonObject jsonObject;
        if(!document.isObject())
        {
            return appInfo;
        }
        jsonObject=document.object();
        if(jsonObject.contains("ui_name"))
        {
            QJsonValue ui_name= jsonObject.take("ui_name");
            appInfo->setUiName(ui_name.toString(""));
        }
        if(jsonObject.contains("app_name"))
        {
            QJsonValue app_name= jsonObject.take("app_name");
            appInfo->setName(app_name.toString(""));
        }

        if(jsonObject.contains("app_icon"))
        {
            QJsonValue app_icon= jsonObject.take("app_icon");
            appInfo->setIcon(app_icon.toString("0"""));
        }
        if(jsonObject.contains("app_argv"))
        {
            QJsonValue app_argv= jsonObject.take("app_argv");
            appInfo->setArgv(app_argv.toString("0"""));
        }

        if(jsonObject.contains("app_file"))
        {
            QJsonValue app_file= jsonObject.take("app_file");
            appInfo->app_file=app_file.toString("");
            appInfo->setApplicationName(app_file.toString(""));
        }
        if(jsonObject.contains("exit_callback"))
        {
            QJsonValue exit_callback= jsonObject.take("exit_callback");
            appInfo->setExitCallback(exit_callback.toString(""));
        }
        if(jsonObject.contains("i18n"))
        {
            QJsonValue i18n= jsonObject.take("i18n");
            appInfo->seti18n(i18n.toString());
        }
        return appInfo;
    }
    return appInfo;
}

QList<Application*>* AppInfoReader::loadFromJsonArray(QByteArray byteArray)
{
    QList<Application*> *list=new QList<Application*>();
    QJsonParseError json_error;
    QJsonDocument document=QJsonDocument::fromJson(byteArray,&json_error);
    if(json_error.error==QJsonParseError::NoError)
    {
        QJsonArray array;
        if(!document.isArray())
        {
            return list;
        }

        array=document.array();
        Application* appInfo;
        for(int i=0;i<array.count();i++)
        {

            QJsonObject jsonObject= array.at(i).toObject();
            appInfo=loadFromJsonObject(jsonObject);
            list->append(appInfo);
        }
    }

    return  list;
}
Application* AppInfoReader::loadFromJsonObject(QJsonObject &jsonObject){
    Application* appInfo=new Application();
    if(jsonObject.contains("ui_name"))
    {
        QJsonValue ui_name= jsonObject.take("ui_name");
        appInfo->setUiName(ui_name.toString(""));
    }
    if(jsonObject.contains("app_name"))
    {
        QJsonValue app_name= jsonObject.take("app_name");
        appInfo->setName(app_name.toString(""));
    }
    if(jsonObject.contains("app_icon"))
    {
        QJsonValue app_icon= jsonObject.take("app_icon");
        appInfo->setIcon(app_icon.toString("0"));
    }
    if(jsonObject.contains("app_argv"))
    {
        QJsonValue app_argv= jsonObject.take("app_argv");
        appInfo->setArgv(app_argv.toString("0"));
    }
    if(jsonObject.contains("app_file"))
    {
        QJsonValue app_file= jsonObject.take("app_file");
        appInfo->app_file=app_file.toString("");
    }
    if(jsonObject.contains("exit_callback"))
    {
        QJsonValue exit_callback= jsonObject.take("exit_callback");
        appInfo->setExitCallback(exit_callback.toString(""));
    }
    return appInfo;
}
