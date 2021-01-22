#ifndef MIDDLEWIDGETRIGHT_H
#define MIDDLEWIDGETRIGHT_H

#include "abstractwheelwidget.h"
#include "lyricutil.h"

#include <QLabel>

class LyricWidget : public AbstractWheelWidget
{
    Q_OBJECT
public:
    LyricWidget(QWidget *parent = 0);
    ~LyricWidget(){}

    void setOriginState();
    void currentMediaChanged(const QString &mediaTitle, const QString &currentMedia);
    void onCurrentPositionChanged(qint64 positon);

    int itemHeight() const;
    int itemCount() const;

    virtual void paintItem(QPainter *painter, int index, QRect &rect);
    virtual void paintItemMask(QPainter *painter);

private:
    QString m_currentMedia;
    QLabel *m_lblTip;
    LyricUtil *m_lyricUtil;
    QColor m_lrcHightLight;
    float m_itemPrecent;

    void initLayout();
    void initData();
    void clearLrc();

private slots:
    void slot_timerWork();
};

#endif // MIDDLEWIDGETRIGHT_H
