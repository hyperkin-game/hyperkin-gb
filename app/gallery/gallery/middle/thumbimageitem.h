#ifndef THUMBIMAGEITEM_H
#define THUMBIMAGEITEM_H

#include <QImage>
#include <QLabel>
#include <QStackedLayout>

#include "basewidget.h"
#include "brightnessmapper.h"

class ThumbImageItem : public BaseWidget
{
    Q_OBJECT
public:
    ThumbImageItem(QString imagePath, QImage *image);

    void onItemClick();

    QImage* getImage()
    {
        return m_image;
    }

    QString getImagePath()
    {
        return m_imagePath;
    }

    bool getCheckState()
    {
        return isChecked;
    }

private:
    bool isChecked;
    bool toUpdate;
    QStackedLayout *m_stackedLayout;
    QString m_imagePath;
    QImage *m_image;
    brightnessMapper *imageMapper;

    QLabel *thumbImage;
    QLabel *m_checkImage;

    void initLayout();

protected:
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
};

#endif // THUMBIMAGEITEM_H
