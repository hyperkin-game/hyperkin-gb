#ifndef IMAGEVIEWERWIDGET_H
#define IMAGEVIEWERWIDGET_H

#include "basewidget.h"
#include "middlewidgets.h"
#include "imageviewer.h"
#include "imagecontroler.h"

class MiddleWidget;

/**
 * one of three stacked widgets.
 * it show the all specific image detail and provide some other image operation.
 */
class ImageViewerWidget : public BaseWidget
{
    Q_OBJECT
public:
    ImageViewerWidget(QWidget *parent = 0);

    void updateRes(QString imagePath);

private:
    MiddleWidget *m_middleWidgets;
    QString m_imagePath;
    // imageViewer: display and operate the image
    ImageViewer *m_imageViewer;
    ImageControler *m_imageControler;

    void initLayout();
    void initConnection();

private slots:
    void slot_onImagesResChanged(bool);
    void slot_lastImage();
    void slot_nextImage();
    void slot_viewDetail();
    void slot_deleteImage();
    void slot_imageZoomOut();
    void slot_imageZoomIn();
    void slot_imageRotate();

signals:
    void imagesResChanged(bool);
};

#endif // IMAGEVIEWERWIDGET_H
