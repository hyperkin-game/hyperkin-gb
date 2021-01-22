#include "volwidget.h"

#include <QHBoxLayout>

#ifdef DEVICE_EVB
int volume_icon_size = 70;
int volume_slider_width = 160;
int volume_slider_height = 20;
#else
int volume_icon_size = 40;
int volume_slider_width = 120;
int volume_slider_height = 20;
#endif

VolWidget::VolWidget(QWidget *parent) : BaseWidget(parent)
  ,isMute(false)
{
    init();

    connect(m_volSlider, SIGNAL(valueChanged(int)), this, SLOT(slot_onSliderValueChanged(int)));
    connect(m_btnIcon, SIGNAL(clicked(bool)), this, SLOT(slot_onIconClick()));
}

void VolWidget::init()
{
    QHBoxLayout *layout = new QHBoxLayout;

    m_volSlider = new BaseSlider(Qt::Horizontal,this);
    m_volSlider->setFixedSize(volume_slider_width, volume_slider_height);

    m_btnIcon = new FlatButton(this);
    m_btnIcon->setFixedSize(volume_icon_size, volume_icon_size);
    m_btnIcon->setBackgroundImage(":/image/music/btn_volume_on.png");

    layout->addWidget(m_btnIcon, 0, Qt::AlignRight | Qt::AlignVCenter);
    layout->addWidget(m_volSlider, 0, Qt::AlignLeft | Qt::AlignVCenter);
    layout->setMargin(0);
    layout->setSpacing(10);

    setLayout(layout);
}

void VolWidget::updateSlider(int value)
{
    m_volSlider->setValue(value);
}

void VolWidget::updateIconBySliderValue(int value)
{
    if (value == 0) {
        isMute = true;
        m_btnIcon->setBackgroundImage(":/image/music/btn_volume_off.png");
    } else {
        isMute = false;
        m_btnIcon->setBackgroundImage(":/image/music/btn_volume_on.png");
    }
}

void VolWidget::slot_onSliderValueChanged(int value)
{
    updateIconBySliderValue(value);
    emit sig_valueChanged(value);
}

void VolWidget::slot_onIconClick()
{
    if (isMute) {
        m_volSlider->setValue(valueBeforeMute);
    } else {
        valueBeforeMute = m_volSlider->value();
        m_volSlider->setValue(0);
    }
}
