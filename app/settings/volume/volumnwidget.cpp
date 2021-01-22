#include "volumnwidget.h"
#include "ui_volumnwidget.h"
#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <stdio.h>
#include <alsa/asoundlib.h>

#include <unistd.h>
#include <fcntl.h>

#define VOLUME_BOUND 10000

typedef enum {
    AUDIO_VOLUME_SET,
    AUDIO_VOLUME_GET,
} audio_volume_action;

/*
  Drawbacks. Sets volume on both channels but gets volume on one. Can be easily adapted.
 */
int audio_volume(audio_volume_action action, long* outvol)
{
    int ret = 0;
    snd_mixer_t* handle;
    snd_mixer_elem_t* elem;
    snd_mixer_selem_id_t* sid;

    static const char* mix_name = "Volume";
    static const char* card = "default";
    static int mix_index = 0;

    snd_mixer_selem_id_alloca(&sid);

    //sets simple-mixer index and name
    snd_mixer_selem_id_set_index(sid, mix_index);
    snd_mixer_selem_id_set_name(sid, mix_name);

    if ((snd_mixer_open(&handle, 0)) < 0)
        return -1;
    if ((snd_mixer_attach(handle, card)) < 0) {
        snd_mixer_close(handle);
        return -2;
    }
    if ((snd_mixer_selem_register(handle, NULL, NULL)) < 0) {
        snd_mixer_close(handle);
        return -3;
    }
    ret = snd_mixer_load(handle);
    if (ret < 0) {
        snd_mixer_close(handle);
        return -4;
    }

    for (elem = snd_mixer_first_elem(handle); elem; elem = snd_mixer_elem_next(elem)) {
        if (snd_mixer_elem_get_type(elem) == SND_MIXER_ELEM_SIMPLE &&
                snd_mixer_selem_is_active(elem)) {
            //printf("------%s----------\n",snd_mixer_selem_get_name(elem));
            if (strcmp(snd_mixer_selem_get_name(elem), "DAC") == 0
		|| strcmp(snd_mixer_selem_get_name(elem), "DAC1") == 0
		|| strcmp(snd_mixer_selem_get_name(elem), "Playback Path") == 0
		|| strcmp(snd_mixer_selem_get_name(elem), "Capture MIC Path") == 0
		|| strcmp(snd_mixer_selem_get_name(elem), "Speaker") == 0)
            {
                break;
            }
        }
    }

    long minv, maxv;

    snd_mixer_selem_get_playback_volume_range(elem, &minv, &maxv);
    fprintf(stderr, "Volume range <%ld, %ld>\n", minv, maxv);

    if (action == AUDIO_VOLUME_GET) {
        if (snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, outvol) < 0) {
            snd_mixer_close(handle);
            return -6;
        }

        fprintf(stderr, "Get volume %ld with status %d \n", *outvol, ret);
        /* make the value bound to 100 */
        *outvol -= minv;
        maxv -= minv;
        minv = 0;
        *outvol = 100 * (*outvol) / maxv; // make the value bound from 0 to 100
    } else if (action == AUDIO_VOLUME_SET) {
        if (*outvol < 0 || *outvol > VOLUME_BOUND) // out of bounds
            return -7;
        *outvol = (*outvol * (maxv - minv) / (100)) + minv;

        if (snd_mixer_selem_set_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, *outvol) < 0) {
            snd_mixer_close(handle);
            return -8;
        }
        if (snd_mixer_selem_set_playback_volume(elem, SND_MIXER_SCHN_FRONT_RIGHT, *outvol) < 0) {
            snd_mixer_close(handle);
            return -9;
        }
        fprintf(stderr, "Set volume %ld with status %d \n", *outvol, ret);
    }

    snd_mixer_close(handle);
    return 0;
}


VolumnWidget::VolumnWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VolumnWidget)
{
    ui->setupUi(this);

    QDir settingsDir("/data/");

    if (settingsDir.exists())
        volumnFile = new QFile("/data/volumn");
    else
        volumnFile = new QFile("/etc/volumn");

    volumnFile->open(QFile::ReadOnly | QIODevice::Truncate);
    QByteArray readAll= volumnFile->readAll();
    QString volumnString(readAll);
    long volumnInt = volumnString.toInt();
    qDebug() << "read volumnInt from sys:" << volumnInt;

    ui->m_VolumnHorizontalSlider->setValue(volumnInt);

    ui->m_VolumnDownPushButton->setIconSize(QSize(60, 60));
    ui->m_VolumnDownPushButton->setIcon(QIcon(QPixmap(":/image/setting/volume-down.png")));
    ui->m_VolumnUpPushButton->setIconSize(QSize(60, 60));
    ui->m_VolumnUpPushButton->setIcon(QIcon(QPixmap(":/image/setting/volume-up.png")));

    ui->m_VolumnHorizontalSlider->setStyleSheet("  \
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

VolumnWidget::~VolumnWidget()
{
    delete ui;
}

void VolumnWidget::onVolumnChange(int volumn){
    long int vol = volumn;
    audio_volume(AUDIO_VOLUME_SET, &vol);
}

void VolumnWidget::on_m_VolumnDownPushButton_clicked()
{
    ui->m_VolumnHorizontalSlider->setValue(ui->m_VolumnHorizontalSlider->value() - ui->m_VolumnHorizontalSlider->pageStep());
}

void VolumnWidget::on_m_VolumnUpPushButton_clicked()
{
    ui->m_VolumnHorizontalSlider->setValue(ui->m_VolumnHorizontalSlider->value() + ui->m_VolumnHorizontalSlider->pageStep());
}

void VolumnWidget::saveVolumn(int volumn)
{
    QDir settingsDir("/data/");

    if (settingsDir.exists())
        volumnFile = new QFile("/data/volumn");
    else
        volumnFile = new QFile("/etc/volumn");

    if (volumnFile->open(QFile::WriteOnly | QIODevice::Truncate)) {
        QTextStream out(volumnFile);
        out << volumn;
    }
}

void VolumnWidget::on_m_VolumnHorizontalSlider_sliderMoved(int position)
{
    QtConcurrent::run(onVolumnChange, position);
}

void VolumnWidget::on_m_VolumnHorizontalSlider_valueChanged(int value)
{
    QtConcurrent::run(onVolumnChange, value);
}

void VolumnWidget::on_m_VolumnHorizontalSlider_sliderReleased()
{
    saveVolumn(ui->m_VolumnHorizontalSlider->value());
}
