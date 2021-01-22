#include "thumbimagewidget.h"
#include "thumbimageitem.h"
#include "constant.h"
#include "cmessagebox.h"

#include <QHBoxLayout>
#include <QInputDialog>

#ifdef DEVICE_EVB
int thumb_image_width = 280;
int bottom_widget_height = 130;
int button_width = 130;
int button_height = 60;
#else
int thumb_image_width = 114;
int bottom_widget_height = 70;
int button_width = 90;
int button_height = 40;
#endif

ThumbImageWidget::ThumbImageWidget(QWidget *parent) : BaseWidget(parent)
  , editMode(false)
{
    m_middleWidgets = (MiddleWidget*)parent;

    initLayout();
    initConnection();
}

void ThumbImageWidget::initLayout()
{
    QVBoxLayout *mainLyout = new QVBoxLayout;

    // layout of bottom control widgets.
    m_controlBottom = new BaseWidget(this);
    m_controlBottom->setBackgroundColor(20, 22, 28);
    m_controlBottom->setFixedHeight(bottom_widget_height);

    m_imageCountText = new QLabel(this);
    m_imageCountText->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    BaseWidget::setWidgetFontSize(m_imageCountText, font_size_big);
    m_imageCountText->adjustSize();

    m_bottomSeleteTitle = new QLabel(this);
    BaseWidget::setWidgetFontSize(m_bottomSeleteTitle, font_size_big);
    m_bottomSeleteTitle->adjustSize();

    m_btnMode = new QPushButton(tr("EditMode"), this);
    m_btnMode->setStyleSheet("QPushButton{background:rgb(36,184,71);color:white;border-radius:5px}"
                             "QPushButton::hover{background:rgb(36,184,71)}"
                             "QPushButton::pressed{background:rgb(61,76,65)}");
    m_btnUpdate = new QPushButton(tr("Refresh"),this);
    m_btnUpdate->setStyleSheet("QPushButton{background:rgb(36,184,71);color:white;border-radius:5px}"
                               "QPushButton::hover{background:rgb(36,184,71)}"
                               "QPushButton::pressed{background:rgb(61,76,65)}");

    m_btnMode->setFixedSize(button_width, button_height);
    m_btnUpdate->setFixedSize(button_width, button_height);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addSpacing(0);
    buttonLayout->addWidget(m_btnUpdate);
    buttonLayout->addWidget(m_btnMode);
    buttonLayout->setSpacing(20);
    buttonLayout->addSpacing(20);

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(m_imageCountText, 1);
    bottomLayout->addWidget(m_bottomSeleteTitle, 1);
    bottomLayout->addLayout(buttonLayout, 1);

    m_controlBottom->setLayout(bottomLayout);

    // Layout of image thumb list.
    m_imageListWid = new BaseListWidget(this);
    m_imageListWid->setIconSize(QSize(thumb_image_width, thumb_image_width));
    m_imageListWid->setSpacing(10);

    mainLyout->addWidget(m_imageListWid);
    mainLyout->addWidget(m_controlBottom);
    mainLyout->setMargin(0);
    mainLyout->setSpacing(0);

    setLayout(mainLyout);
}

void ThumbImageWidget::initConnection()
{
    connect(this, SIGNAL(imagesResChanged()), this, SLOT(slot_onImagesResChanged()));
    connect(m_btnMode, SIGNAL(clicked(bool)), this, SLOT(slot_changeImageMode()));
    connect(m_btnUpdate, SIGNAL(clicked(bool)), this, SLOT(slot_updateImages()));
    connect(m_imageListWid, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(slot_onListItemClick(QListWidgetItem*)));
}

void ThumbImageWidget::updateImageCount()
{
    m_imageCountText->setText(tr("★ Image and Preview(%1)").arg(QString::number(m_imageListWid->count())));
}

void ThumbImageWidget::updateBottomSeleteText()
{
    if (!editMode)
        m_bottomSeleteTitle->setText("");
    else
        m_bottomSeleteTitle->setText(tr("Current has %1 images seleted").arg(QString::number(m_selectedItems.size())));
}

void ThumbImageWidget::onImagesResInsert(QString path, QImage *thumb)
{
    QListWidgetItem *litsItem = new QListWidgetItem();
    litsItem->setData(Qt::DisplayRole, path);
    litsItem->setSizeHint(QSize(thumb_image_width, thumb_image_width));

    ThumbImageItem *itemWid = new ThumbImageItem(path, thumb);
    m_imageListWid->addItem(litsItem);
    m_imageListWid->setItemWidget(litsItem, itemWid);
    m_thumbs.insert(path, litsItem);

    m_imageListWid->sortItems();
    updateImageCount();
}

void ThumbImageWidget::onImagesResRemove(QString path)
{
    if (m_thumbs.keys().contains(path)) {
        QListWidgetItem *litsItem = m_thumbs[path];
        if (litsItem) {
            m_imageListWid->removeItemWidget(litsItem);
            delete litsItem;
        }

        if (m_imageListWid->count() == 0)
            emit m_middleWidgets->imageEmpty();
        else
            updateImageCount();
    }
}

void ThumbImageWidget::slot_onImagesResChanged()
{
    if (editMode)
        slot_changeImageMode();

    if (m_imageListWid->count() == 0) {
        emit m_middleWidgets->imageEmpty();
    } else {
        m_imageListWid->sortItems();
        updateImageCount();
    }
}

void ThumbImageWidget::slot_onListItemClick(QListWidgetItem *listItem)
{
    ThumbImageItem *imageItem = (ThumbImageItem*)m_imageListWid->itemWidget(listItem);

    if (editMode) {
        imageItem->onItemClick();
        if (imageItem->getCheckState())
            m_selectedItems.append(imageItem);
        else
            m_selectedItems.removeOne(imageItem);

        updateBottomSeleteText();
    } else {
        emit m_middleWidgets->imageItemClick(imageItem->getImagePath());
    }
}

void ThumbImageWidget::slot_changeImageMode()
{
    if (editMode) {
        // change to normal mode.
        for (int i = 0; i < m_selectedItems.size(); i++)
            m_selectedItems.at(i)->onItemClick();

        editMode = false;
        m_selectedItems.clear();
        m_btnUpdate->setText(tr("Refresh"));
        m_btnMode->setText(tr("EditMode"));
        updateBottomSeleteText();
    } else {
        // change to edit mode.
        editMode = true;
        m_btnUpdate->setText(tr("Delete"));
        m_btnMode->setText(tr("Cancel"));
        updateBottomSeleteText();
    }
}

void ThumbImageWidget::slot_updateImages()
{
    if (m_selectedItems.size() > 0 && editMode) {
        int result = CMessageBox::showCMessageBox(this, tr("Delete images?"), tr("Delete"), tr("Cancel"));
        if (result == CMessageBox::RESULT_CONFIRM) {
            // delete images selected
            for (int i = 0; i < m_selectedItems.size(); i++) {
                ThumbImageItem *imageItem = m_selectedItems.at(i);
                if (QFile::remove(imageItem->getImagePath())) {
                    mainWindow->getGalleryWidget()->removeImage(imageItem->getImagePath());
                    emit m_middleWidgets->sig_imagesResRemove(imageItem->getImagePath());
                }
            }
            emit m_middleWidgets->imagesResChanged();
        }
    } else {
        mainWindow->slot_updateMedia();
    }
}
