#ifndef APPLICATIONMODEL_H
#define APPLICATIONMODEL_H
#include <QAbstractListModel>
#include <QVector>
#include "application.h"
class ApplicationModel : public QAbstractListModel
{
    Q_OBJECT

    struct ModelItem {
        QString name;
        QString pkgName;
        QString ui_name;
        QString argv;
        QString icon;
        QString exitCallback;
    };

    enum {name=Qt::UserRole+1, pkgName, ui_name, argv, icon,exitCallback};
public:
    explicit ApplicationModel(QObject *parent = 0);
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    QHash<int, QByteArray> roleNames() const;
    static ApplicationModel* s_model;
private:
    QVector<Application*> v;

public slots:
    void onApplicationInsert(Application*);

    void reflushUI();
};

#endif // APPLICATIONMODEL_H
