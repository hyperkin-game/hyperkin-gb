#ifndef GALLERYWIDGETS_H
#define GALLERYWIDGETS_H

#include "basewidget.h"
#include "top/topwidget.h"
#include "middle/middlewidgets.h"
#include "constant.h"

#include <QThread>
#include <QFileInfoList>

class LoadImageThread;

/**
 * The applicatiion contains top and middle widget and
 * each has it's own layout and connection control.
 */
class GalleryWidgets : public BaseWidget
{
    Q_OBJECT
public:
    GalleryWidgets(QWidget *parent = 0);
    ~GalleryWidgets();

    MiddleWidget* getMiddleWidget(){return m_middleWid;}
    QMap<QString, QImage*>& getImagesRes(){return m_imagesRes;}
    void processFileList(QFileInfoList fileInfoList);
    void removeImage(QString imagePath);

private:
    // Saved image resource\ Key with image path and value with image.
    QMap<QString, QImage*> m_imagesRes;

    LoadImageThread *m_loadImageThread;

    TopWidget *m_topWid;
    MiddleWidget *m_middleWid;

    void initData();
    void initConnection();
    void initLayout();

private slots:
    void slot_onReturnClicked();
    void slot_onViewerResChanged(QString);

public slots:
    void slot_onImagesResChanged();

signals:
    void loadImageComplete();
};

/**
 * Thread that used for load image and it will saved into thumb image
 * for less memery.
 */
class LoadImageThread : public QThread
{
public:
    LoadImageThread(GalleryWidgets *parentWidget, MiddleWidget *middleWid);
    ~LoadImageThread(){}

    void setFileInfoList(QFileInfoList infoList);
    void stopThread();

private:
    GalleryWidgets *m_parent;
    QFileInfoList m_infoList;
    MiddleWidget *m_middleWid;

    void compressImg(QString src,QString out);
    QString fileMD5(QString path);

protected:
    void run();
};

#endif // GALLERYWIDGETS_H
