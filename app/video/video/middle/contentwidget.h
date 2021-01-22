#ifndef VIDEOMIDDLEWIDGETLEFT_H
#define VIDEOMIDDLEWIDGETLEFT_H

#include "basewidget.h"
#include "quickinterfacewidget.h"

#include <QMediaPlayer>

class ContentWidget : public BaseWidget
{ 
    Q_OBJECT
public:
    ContentWidget(QWidget *parent);
    ~ContentWidget();

    QMediaPlayer* getMediaPlayerFormQml()
    {
        return m_player;
    }

    QuickInterfaceWidget* getSurfaceWid()
    {
        return m_surfaceWid;
    }

private:
    // QMediaPlayer will deliver to 'videoWidets' for global control,
    // so it is no actual use in here.
    QMediaPlayer *m_player;
    QuickInterfaceWidget *m_surfaceWid;

    void initLayout();
    void initConnection();

signals:
    void surfaceOneClick();
    void surfaceDoubleClick();
};

#endif // VIDEOMIDDLEWIDGETLEFT_H
