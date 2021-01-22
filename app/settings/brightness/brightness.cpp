#include "brightness.h"
#include "ui_brightness.h"
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QtConcurrent/QtConcurrent>

Brightness::Brightness(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Brightness)
{
    ui->setupUi(this);

#ifdef DEVICE_EVB
    QFile brightnessPath("/sys/devices/platform/backlight/backlight/backlight/brightness");
#else
    QFile brightnessPath("/sys/class/backlight/rk28_bl/brightness");
#endif
    brightnessPath.open(QFile::ReadOnly | QIODevice::Truncate);
    QByteArray readAll= brightnessPath.readAll();
    QString brightnessString(readAll);
    int brightnessInt = brightnessString.toInt();
    qDebug() << "read brightness from sys:" << brightnessInt;
    saveBrightness(brightnessInt);

    ui->m_BrightnessDownPushButton->setIconSize(QSize(60, 60));
    ui->m_BrightnessDownPushButton->setIcon(QIcon(QPixmap(":/image/setting/bightness-down.png")));
    ui->m_BrightnessUpPushButton->setIconSize(QSize(60, 60));
    ui->m_BrightnessUpPushButton->setIcon(QIcon(QPixmap(":/image/setting/bightness-up.png")));

    ui->m_BrightnessHorizontalSlider->setValue(brightnessInt);
    ui->m_BrightnessHorizontalSlider->setStyleSheet("  \
                                                    QSlider{\
                                                        border-color: #bcbcbc;\
                                                    }\
                                                    QSlider::groove:horizontal {\
                                                        border: 1px solid #999999;\
                                                        height: 20px;\
                                                        margin: 0px 0;\
                                                        left: 5px; right: 5px;\
                                                    }\
                                                    QSlider::handle:horizontal {\
                                                        border: 0px ;\
                                                        border-image:url(:/image/setting/slider.png);\
                                                        width: 60px;\
                                                        margin: -30px -13px -30px -13px;\
                                                    }\
                                                    QSlider::add-page:horizontal{\
                                                        background: qlineargradient(spread:pad, x1:0, y1:1, x2:0, y2:0, stop:0 #bcbcbc, stop:0.25 #bcbcbc, stop:0.5 #bcbcbc, stop:1 #bcbcbc);\
                                                    }\
                                                    QSlider::sub-page:horizontal{\
                                                        background: qlineargradient(spread:pad, x1:0, y1:1, x2:0, y2:0, stop:0 #439cf3, stop:0.25 #439cf3, stop:0.5 #439cf3, stop:1 #439cf3);\
                                                    }\
                                                    ");
}

Brightness::~Brightness()
{
    delete ui;
}

void Brightness::on_m_BrightnessHorizontalSlider_sliderMoved(int position)
{
    QtConcurrent::run(onBrightnessChange,position);
}

void Brightness::on_m_BrightnessHorizontalSlider_valueChanged(int value)
{
    QtConcurrent::run(onBrightnessChange,value);
}

void Brightness::on_m_BrightnessHorizontalSlider_actionTriggered(int)
{
}

void Brightness::onBrightnessChange(int brightness)
{
    qDebug() <<  "brightness:" << brightness;
#ifdef DEVICE_EVB
    QString cmd= QString("echo %1 > %2").arg(brightness).arg("/sys/devices/platform/backlight/backlight/backlight/brightness");
#else
    QString cmd= QString("echo %1 > %2").arg(brightness).arg("/sys/class/backlight/rk28_bl/brightness");
#endif

    system(cmd.toLatin1().data());
}

void Brightness::saveBrightness(int brightness){
    QDir settingsDir("/data/");

    if (settingsDir.exists())
        brightnessFile = new QFile("/data/brightness");
    else
        brightnessFile = new QFile("/etc/brightness");

    if (brightnessFile->open(QFile::WriteOnly | QIODevice::Truncate)) {
        QTextStream out(brightnessFile);
        out << brightness;
    }
}

void Brightness::on_m_BrightnessDownPushButton_clicked()
{
    ui->m_BrightnessHorizontalSlider->setValue(ui->m_BrightnessHorizontalSlider->value() - ui->m_BrightnessHorizontalSlider->pageStep());
}

void Brightness::on_m_BrightnessUpPushButton_clicked()
{
    ui->m_BrightnessHorizontalSlider->setValue(ui->m_BrightnessHorizontalSlider->value() + ui->m_BrightnessHorizontalSlider->pageStep());
}

void Brightness::on_m_BrightnessHorizontalSlider_sliderReleased()
{
    saveBrightness(ui->m_BrightnessHorizontalSlider->value());
}
