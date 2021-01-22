#ifndef THUMBIMAGEWIDGET_H
#define THUMBIMAGEWIDGET_H

#include <QImage>
#include <QLabel>

#include "basewidget.h"
#include "middlewidgets.h"
#include "thumbimageitem.h"
#include "basepushbutton.h"
#include "base/baselistwidget.h"

class MiddleWidget;

/**
 * one of three stacked widgets.
 * it show the all images Thumbnail on special search path.
 */
class ThumbImageWidget : public BaseWidget
{
    Q_OBJECT
public:
    ThumbImageWidget(QWidget *parent);

    void onImagesResInsert(QString path, QImage *thumb);
    void onImagesResRemove(QString path);

private:
    void initLayout();
    void initConnection();
    void updateImageCount();
    void updateBottomSeleteText();

private:
    BaseListWidget *m_imageListWid;
    MiddleWidget *m_middleWidgets;
    bool editMode;
    // control bottom
    BaseWidget *m_controlBottom;
    QLabel *m_imageCountText;
    QLabel *m_bottomSeleteTitle;
    QPushButton *m_btnMode;
    QPushButton *m_btnUpdate;

    QList<ThumbImageItem*> m_selectedItems;
    QMap<QString, QListWidgetItem*> m_thumbs;

signals:
    void imagesResChanged();

private slots:
    void slot_onImagesResChanged();
    void slot_onListItemClick(QListWidgetItem*);
    void slot_changeImageMode();
    void slot_updateImages();
};

#endif // THUMBIMAGEWIDGET_H
