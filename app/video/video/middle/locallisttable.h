#ifndef VIDEOLOCALLISTTABLE_H
#define VIDEOLOCALLISTTABLE_H

#include "basetablewidget.h"

class LocalListTable : public BaseTableWidget
{
    Q_OBJECT
public:
    LocalListTable(QWidget *parent);
    ~LocalListTable(){}

    void insertIntoTable(const QString &name, const QString &suffix);
    void removeTableItem(int row);
    void clearTable();

    void playingItemChanged(int index);
    void setOriginState();

protected:
    void resizeEvent(QResizeEvent*);

private:
    int playingItemIndex;
    QString playingItemSuffix;

    void init();
};

#endif // VIDEOLOCALLISTTABLE_H
