#include "hotspotmainwidget.h"

#include <QHBoxLayout>
#include <QMessageBox>

#include "constant.h"
#include "wpaserviceutil.h"
#include "retranslatemanager.h"

SwitchHead::SwitchHead(QWidget *parent) : BaseWidget(parent)
{
    setBackgroundColor(92, 89, 89);

    QHBoxLayout *layout = new QHBoxLayout;

    m_switchText = new QLabel(str_hotspot_title, this);
    setWidgetFontBold(m_switchText);

    m_switchButton = new SwitchButton(this);

#ifdef DEVICE_EVB
    m_switchButton->setFixedSize(100, 40);
#else
    m_switchButton->setFixedSize(50, 23);
#endif

    layout->addWidget(m_switchText);
    layout->addStretch(0);
    layout->addWidget(m_switchButton);
    setContentsMargins(20, 0, 20, 0);
    setLayout(layout);

    connect(m_switchButton, SIGNAL(checkedChanged(bool)), this, SIGNAL(sig_checkedChanged(bool)));
}

void SwitchHead::setTitle(const QString &title)
{
    m_switchText->setText(title);
}

void SwitchHead::setChecked(bool checked)
{
    m_switchButton->setChecked(checked);
}

HotspotMainWidget::HotspotMainWidget(QWidget *parent) : BaseWidget(parent)
  , m_thread(0)
{
    setBackgroundColor(33, 36, 43);
    initLayout();

    connect(m_header, SIGNAL(sig_checkedChanged(bool)), this, SLOT(slot_switchStateChanged(bool)));
    connect(mainWindow, SIGNAL(retranslateUi()), this, SLOT(retranslateUi()));
}

void HotspotMainWidget::initLayout()
{
    QVBoxLayout *mainlayout = new QVBoxLayout;

    m_header = new SwitchHead(this);

    m_nameItem = new LineEditItem(this);
    m_nameItem->setItem(str_hotspot_name, "HOTSPOT_TEST");

    m_pskItem= new LineEditItem(this);
    m_pskItem->setItem(str_hotspot_password, "987654321");

    QFrame *bottomLine = new QFrame(this);
    bottomLine->setFixedHeight(1);
    bottomLine->setStyleSheet("QFrame{border:1px solid rgb(0,0,0);}");
    bottomLine->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

    mainlayout->addSpacing(50);
    mainlayout->addWidget(m_header);
    mainlayout->addWidget(m_nameItem);
    mainlayout->addWidget(bottomLine);
    mainlayout->addWidget(m_pskItem);
    mainlayout->addStretch(0);
    mainlayout->setSpacing(0);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addStretch(1);
    layout->addLayout(mainlayout, 6);
    layout->addStretch(1);

    setLayout(layout);
}

void HotspotMainWidget::retranslateUi()
{
    m_nameItem->setItemName(str_hotspot_name);
    m_pskItem->setItemName(str_hotspot_password);
    m_header->setTitle(str_hotspot_title);
}

void HotspotMainWidget::slot_switchStateChanged(bool checked)
{
    if (m_thread && m_thread->isRunning()) {
        m_header->setChecked(!checked);
        return;
    }

    // check if the data is reasonable
    if (checked && (m_nameItem->getValue() == NULL || m_pskItem->getValue().length() < 8)) {
        QMessageBox::warning(mainWindow, str_hotspot_warning, str_hotspot_format_error,
                             QMessageBox::Ok);
        m_header->setChecked(false);
        return;
    }

    if (m_thread) {
        delete m_thread;
        m_thread = 0;
    }

    creat_hostapd_file(m_nameItem->getValue().toLocal8Bit().constData(), m_pskItem->getValue().toLocal8Bit().constData());
    if (checked && is_supplicant_running()) {
        // close wifi first.
        WPAManager::getInstance()->closeWPAConnection();
        wifi_stop_supplicant();
    }

    m_thread = new HotspotThread(this, checked);
    m_thread->start();
}

void HotspotMainWidget::showEvent(QShowEvent *event)
{
    BaseWidget::showEvent(event);

    m_header->setFocus();
    m_header->setChecked(is_hostapd_running());
}

HotspotThread::HotspotThread(QObject *parent, bool isStartHotspot) : QThread(parent)
{
    this->isStartHotspot = isStartHotspot;
}

HotspotThread::~HotspotThread()
{
    requestInterruption();
    quit();
    wait();
}

void HotspotThread::run()
{
    if (isStartHotspot)
        wifi_start_hostapd();
    else
        wifi_stop_hostapd();
}
