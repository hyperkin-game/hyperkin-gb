#ifndef MUISCLISTTABLE_H
#define MUISCLISTTABLE_H

#include <basetablewidget.h>

class MusicListTable : public BaseTableWidget
{
    Q_OBJECT
public:
    MusicListTable(QWidget *parent = 0);
    ~MusicListTable(){}

    void setRowTextColor(int row, const QColor &color) const;
    void insertIntoTable(const QString &name, const QString &suffix);
    void removeTableItem(int row);
    void playingItemChanged(int index);
    void clearTable();
    void setOriginState();

protected:
    void resizeEvent(QResizeEvent *event);

private:
    int playingItemIndex;
    QString playingItemSuffix;

    void init();
    void initConnection();
};

#endif // MUISCLISTTABLE_H
