#ifndef GALLERYMIDDLEWIDGETS_H
#define GALLERYMIDDLEWIDGETS_H

#include "basewidget.h"
#include "emptyimageswidget.h"
#include "thumbimagewidget.h"
#include "imageviewerwidget.h"

#include <QStackedLayout>

class ThumbImageWidget;
class ImageViewerWidget;

/**
 * The middle part of application.
 *
 * There is three stacked widget on it and it will change show state
 * by images state.
 * The three stacked widgets are: viewer、empty、thumb
 */
class MiddleWidget : public BaseWidget
{
    Q_OBJECT
public:
    MiddleWidget(QWidget *parent);

    bool isViewerMode();
    void leaveViewerMode();

private:
    QStackedLayout *m_stackLayout;

    EmptyImagesWidget *m_emptyImgWid;
    ThumbImageWidget *m_thumbImgWid;
    ImageViewerWidget *m_viewerWid;

    void initLayout();
    void initConnection();

signals:
    void imagesResChanged();
    void sig_imagesResInsert(QString path, QImage *img);
    void sig_imagesResRemove(QString path);
    void imageEmpty();
    void imageItemClick(QString imagePath);
    void viewerResChanged(QString filePath);

private slots:
    void slot_showEmptyImageTip();
    void slot_showImageViewer(QString imagePath);
    void slot_onImagesResChanged();
    void slot_imagesResInsert(QString path, QImage *img);
    void slot_imagesResRemove(QString path);
};

#endif // GALLERYMIDDLEWIDGETS_H
