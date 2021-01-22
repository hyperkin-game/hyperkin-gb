#include "applicationmodel.h"
#include <QDebug>

ApplicationModel* ApplicationModel::s_model = 0;
ApplicationModel::ApplicationModel(QObject *parent)
    :QAbstractListModel(parent)
{

}
void ApplicationModel::onApplicationInsert(Application * app){
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    v.push_back(app);
    connect(app,SIGNAL(uiNameChanged()),this,SLOT(reflushUI()));
    endInsertRows();
}

void ApplicationModel::reflushUI(){
     beginResetModel();
     endResetModel();
}

QVariant ApplicationModel::data(const QModelIndex &index, int role) const
{
    int i = index.row();
    if (i < 0 || i >= v.size()) {
        return QVariant();
    }

    if (role == name) {
        return v[i]->name();
    }
    if (role==pkgName) {
        return v[i]->pkgName();
    }
    if (role==ui_name) {
        return v[i]->ui_name();
    }
    if (role==argv) {
        return v[i]->argv();
    }
    if (role==icon) {
        return v[i]->icon();
    }
    if (role==exitCallback) {
        return v[i]->exitCallback();
    }
}

int ApplicationModel::rowCount(const QModelIndex &parent) const
{
    return v.size();
}

QHash<int, QByteArray> ApplicationModel::roleNames() const
{
    QHash<int, QByteArray> r =QAbstractListModel::roleNames();
    r[name]="name";
    r[pkgName]="pkgName";
    r[ui_name]="ui_name";
    r[argv]="argv";
    r[icon]="icon";
    r[exitCallback]="exitCallback";
    return r;
}
