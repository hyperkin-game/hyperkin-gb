#include "imagedetailwidget.h"
#include "constant.h"

#include <QHBoxLayout>

#ifdef DEVICE_EVB
int detail_widget_height = 900;
int confirm_button_width = 130;
int confirm_button_hieght = 60;
int title_width = 220;
#else
int detail_widget_height = 470;
int confirm_button_width = 90;
int confirm_button_hieght = 30;
int title_width = 100;
#endif

ImageDetailWidget::ImageDetailWidget(QWidget *parent) : QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setMinimumHeight(detail_widget_height);

    setObjectName("ImageDetailWidget");
    setStyleSheet("#ImageDetailWidget{border:1.5px solid rgb(0,120,215);background:rgb(56,58,66);border-radius:5px;}"
                  "QLabel{color:white;}");

    initLayout();
    initConnection();
}

void ImageDetailWidget::initLayout()
{
    QVBoxLayout *mainLyout = new QVBoxLayout;

    // header
    QFrame *underLine = new QFrame(this);
    underLine->setFixedHeight(1);
    underLine->setStyleSheet("QFrame{border:1px solid rgb(255,255,255);}");
    underLine->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QHBoxLayout *headerLyout = new QHBoxLayout;

    QLabel *titleLabel = new QLabel(tr("Image Infomation"), this);
    BaseWidget::setWidgetFontBold(titleLabel);
    titleLabel->setAlignment(Qt::AlignLeft);

    headerLyout->addSpacing(10);
    headerLyout->addWidget(titleLabel);
    headerLyout->addStretch(0);
    headerLyout->addSpacing(10);

    // image infomation
    QVBoxLayout *infoLyout = new QVBoxLayout;

    nameItem = new ImageItem(this);
    patternItem = new ImageItem(this);
    resolutionItem = new ImageItem(this);
    locationItem = new ImageItem(this);
    sizeItem = new ImageItem(this);
    createTimeItem = new ImageItem(this);

    infoLyout->addWidget(nameItem);
    infoLyout->addWidget(patternItem);
    infoLyout->addWidget(resolutionItem);
    infoLyout->addWidget(locationItem);
    infoLyout->addWidget(sizeItem);
    infoLyout->addWidget(createTimeItem);

    // confirm cancel button
    QHBoxLayout *controlLyout = new QHBoxLayout;

    m_btnConfirm = new FlatButton(tr("Confirm"), this);
    m_btnConfirm->setStyleSheet("QPushButton{background:rgb(36,184,71);color:white;border-radius:5px}"
                                "QPushButton::hover{background:rgb(36,184,71)}"
                                "QPushButton::pressed{background:rgb(61,76,65)}");
    m_btnConfirm->setFixedSize(confirm_button_width, confirm_button_hieght);

    controlLyout->addStretch(0);
    controlLyout->addWidget(m_btnConfirm);
    controlLyout->setContentsMargins(10, 10, 20, 10);

    mainLyout->addLayout(headerLyout);
    mainLyout->addWidget(underLine);
    mainLyout->addSpacing(20);
    mainLyout->addLayout(infoLyout);
    mainLyout->addStretch(0);
    mainLyout->addLayout(controlLyout);
    mainLyout->setMargin(10);

    setLayout(mainLyout);
}

void ImageDetailWidget::initConnection()
{
    connect(m_btnConfirm, SIGNAL(clicked(bool)), this, SLOT(close()));
}

QString getImageResolution(QString imagePath)
{
    QString result;
    QImage image;

    if (image.load(imagePath))
        result.append(QString::number(image.width())).append("×").append(QString::number(image.height()))
                .append("(width×height)");

    return result;
}

QString getImageResolution(QSize size)
{
    QString result;

    result.append(QString::number(size.width())).append("x").append(QString::number(size.height()))
            .append("(width×height)");

    return result;
}

/**
 * convert size form B to other
 * @param size
 * @return
 */
QString convertFileSize(qint64 size) {
    QString unit = "B";
    qint64 curSize = size;
    double curSize2 = 0;
    if (curSize > 1024) {
        curSize /= 1024;
        unit = "KB";
        if (curSize > 1024) {
            curSize2 = curSize / 1024.00;
            unit = "MB";
            return QString::number(curSize2, 'f', 2).append(unit);
        }
    }

    return QString::number(curSize).append(unit);
}

int ImageDetailWidget::showImageDetail(QWidget *parent, QString imagePath, QSize size)
{
    ImageDetailWidget *detailWidget = new ImageDetailWidget(parent);
    QFileInfo *info = new QFileInfo(imagePath);
    if (info->exists()) {
        detailWidget->nameItem->updateItem(tr("Name"), info->baseName());
        detailWidget->patternItem->updateItem(tr("Pattern"), info->completeSuffix());
        detailWidget->resolutionItem->updateItem(tr("Resolution"), getImageResolution(size));
        detailWidget->locationItem->updateItem(tr("Location"), info->absolutePath());
        detailWidget->sizeItem->updateItem(tr("Size"), convertFileSize(info->size()));
        detailWidget->createTimeItem->updateItem(tr("CreateTime"), info->created().toString("yyyy-MM-dd hh:mm"));
    }

    return detailWidget->exec();
}

int ImageDetailWidget::exec()
{
    this->setWindowModality(Qt::WindowModal);
    this->show();

    m_eventLoop = new QEventLoop;
    m_eventLoop->exec();

    return -1;
}

void ImageDetailWidget::closeEvent(QCloseEvent *event)
{
    if (m_eventLoop != NULL)
        m_eventLoop->exit();

    event->accept();
}

ImageItem::ImageItem(QWidget *parent) : BaseWidget(parent)
{
    QHBoxLayout *lyout = new QHBoxLayout;

    titleLabel = new QLabel(this);
    titleLabel->setAlignment(Qt::AlignRight);
    titleLabel->setFixedWidth(title_width);

    textLabel = new QLabel(this);

    lyout->addWidget(titleLabel);
    lyout->addWidget(textLabel);
    lyout->addStretch(0);

    lyout->setMargin(0);
    lyout->setSpacing(10);

    setLayout(lyout);
}

void ImageItem::updateItem(QString title, QString text)
{
    titleLabel->setText(title + ":");
    textLabel->setText(text);
}
