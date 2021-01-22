#ifndef VIDEOMIDDLEWIDGETRIGHT_H
#define VIDEOMIDDLEWIDGETRIGHT_H

#include "basewidget.h"
#include "listheader.h"
#include "locallisttable.h"
#include "player/medialist.h"

#include <QFileInfoList>
#include <QStackedWidget>
#include <QMediaContent>

class ListWidgets : public BaseWidget
{
    Q_OBJECT
public:
    ListWidgets(QWidget *parent = 0);
    ~ListWidgets();

public:
    void updateResUi(QFileInfoList fileList);
    void updatePlayingItemStyle(QMediaContent);
    void addVideo();
    void setOriginState();
    void deleteItem(int);

    MediaList* &getMediaList()
    {
        return m_playList;
    }

private:
    LocalListTable *m_localTable;
    ListHeader *m_listHeader;
    QStackedWidget *m_stackedWid;
    BaseTableWidget *m_netTable;

    MediaList *m_playList;

    void initData();
    void initLayout();
    void initConnection();

signals:
    void sig_localTableItemClick(int, int);
    int tableLongPressed(int);

private slots:
    void slot_switchToLocalList()
    {
        m_stackedWid->setCurrentIndex(0);
    }

    void slot_switchToNetList()
    {
        m_stackedWid->setCurrentIndex(1);
    }
};

#endif // VIDEOMIDDLEWIDGETRIGHT_H
