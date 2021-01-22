#ifndef MIDDLEWIDGETS_H
#define MIDDLEWIDGETS_H

#include "basewidget.h"
#include "musiclistwidget.h"
#include "lyricwidget.h"

/**
 * Stand for the middle part of music ui.It contains a music list(left)
 * and a lyrics interface(right).
 */
class MiddleWidgets : public BaseWidget
{
    Q_OBJECT
public:
    explicit MiddleWidgets(QWidget *parent = 0);
    ~MiddleWidgets();

    MusicListWidget* getListWidget()
    {
        return m_listWid;
    }

    LyricWidget* getLyricWidget()
    {
        return m_lyricWid;
    }

private:
    MusicListWidget *m_listWid;
    LyricWidget *m_lyricWid;

    void initLayout();
};

#endif // MIDDLEWIDGETS_H
