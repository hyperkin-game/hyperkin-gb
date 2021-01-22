#include "basepushbutton.h"

RotatableButton::RotatableButton(const QString &icon, QWidget *parent) : QPushButton(parent),
    angle_(0),
    timerId_(-1),
    delay_(100),
    displayedWhenStopped_(false),
    color_(Qt::white)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setFocusPolicy(Qt::NoFocus);

    currentPix_ = QPixmap(icon);
}

bool RotatableButton::isAnimated () const
{
    return (timerId_ != -1);
}

void RotatableButton::startAnimation()
{
    angle_ = 0;

    if (timerId_ == -1)
        timerId_ = startTimer(delay_);
}

void RotatableButton::stopAnimation()
{
    if (timerId_ != -1)
        killTimer(timerId_);

    timerId_ = -1;
    update();
}

void RotatableButton::timerEvent(QTimerEvent * /*event*/)
{
    angle_ = (angle_ + 30) % 360;

    update();
}

void RotatableButton::paintEvent(QPaintEvent * /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    if (!displayedWhenStopped_ && !isAnimated()) {
        p.drawPixmap(rect(), currentPix_);
        return;
    }

    int width = qMin(this->width(), this->height());


    int outerRadius = (width-1) >> 1;
    int innerRadius = ((width-1) >> 1) * 0.38;

    int capsuleHeight = outerRadius - innerRadius;
    int capsuleWidth  = (width > 32 ) ? capsuleHeight *.23 : capsuleHeight * .35;
    int capsuleRadius = capsuleWidth >> 1;

    if (progress_ > 0 && progress_ < 100) {
        p.setPen(color_);
        p.drawText(rect(), Qt::AlignCenter, QString("%1%").arg(progress_));
    } else if (progress_ == 100) {
        stopAnimation();
    }

    for (int i = 0; i < 12; ++i) {
        QColor color = color_;
        color.setAlphaF(1.0f - (i / 12.0f));
        p.setPen(Qt::NoPen);
        p.setBrush(color);
        p.save();
        p.translate(rect().center());
        p.rotate(angle_ - i * 30.0f);
        p.drawRoundedRect(((-capsuleWidth) >> 1), -(innerRadius+capsuleHeight), capsuleWidth, capsuleHeight, capsuleRadius, capsuleRadius);
        p.restore();
    }
}

FlatButton::FlatButton(QWidget *parent) : QPushButton(parent)
  , longPressedFlag(false)
{
    setCursor(Qt::PointingHandCursor);
    setFlat(true);
    setStyleSheet("QPushButton{background:transparent;}");

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slot_timerTimeout()));
}

FlatButton::FlatButton(const QString& str, QWidget *parent) : QPushButton(str, parent)
  , longPressedFlag(false)
{
    setCursor(Qt::PointingHandCursor);
    setFlat(true);
    setStyleSheet("QPushButton{background:transparent;}");

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slot_timerTimeout()));
}

void FlatButton::slot_timerTimeout()
{
    m_timer->stop();
    longPressedFlag = true;
    // once longPressedFlag be setted, send signal every 500 milliseconds.
    m_timer->start(500);
    emit longPressedEvent();
}

void FlatButton::mousePressEvent(QMouseEvent *event)
{
    QPushButton::mousePressEvent(event);
    m_timer->start(1000);
}

void FlatButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (longPressedFlag) {
        event->accept();
        longPressedFlag = false;
    } else {
        QPushButton::mouseReleaseEvent(event);
    }

    m_timer->stop();
}

void FlatButton::setBackgroundImage(const QString &imageRess)
{
    QString styleStr;
    styleStr.append("border-image:url(").append(imageRess).append(")");

    setStyleSheet(styleStr);
}

VolButton::VolButton(const QString& normal,QWidget*parent) : QPushButton(parent)
{
    m_partnerslider = NULL;
    m_isenter = false;
    m_index = 0;
    m_isvolempty = 100;
    m_savevol = 100;
    setCursor(Qt::PointingHandCursor);

    QPixmap pix(normal);

    for (int i = 0; i < 5; i++)
        m_listnormal << pix.copy(i * (pix.width() / 5), 0, pix.width() / 5, pix.height());

}

void VolButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.drawPixmap((width() - m_listnormal.at(0).width()) / 2,
                 (height() - m_listnormal.at(0).height()) / 2, m_listnormal.at(m_index));
}

void VolButton::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
        QPushButton::mousePressEvent(e);
}

void VolButton::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        if (this->contentsRect().contains(mapFromGlobal(QCursor::pos()))) {
            if (m_isvolempty == 0) {
                emit setMute(m_savevol);
            } else {
                m_savevol = m_partnerslider->value();
                emit setMute(0);
            }
        }
        QPushButton::mouseReleaseEvent(e);
    }
}

void VolButton::setButtonPixmap(int value)
{
    m_isenter = false;
    if (value == 0)
        m_index = 4;
    if (value > 2 && value <= 30)
        m_index = 1;
    if (value > 30)
        m_index = 2;

    update();
    m_isvolempty = value;
}

void VolButton::enterEvent(QEvent *)
{
    m_isenter = true;
    update();
}

void VolButton::leaveEvent(QEvent *)
{
    m_isenter = false;
    update();
}

StackButton::StackButton(const QString &pixnormal, const QString &pixhover,
                         const QString &pixsel, QWidget *parent) : QPushButton(parent)
{
    m_enter = false;
    m_pressed = false;
    m_pixnormal = QPixmap(pixnormal);
    m_pixhover = QPixmap(pixhover);
    m_pixselected = QPixmap(pixsel);

    setCursor(Qt::PointingHandCursor);
    setFlat(true);
}

void StackButton::paintEvent(QPaintEvent *e)
{
    QPushButton::paintEvent(e);

    QPainter p(this);
    if (!m_enter && !m_pressed)
        p.drawPixmap((width() - m_pixnormal.width()) / 2, (height() - m_pixnormal.height()) / 2, m_pixnormal);

    if (m_enter && !m_pressed)
        p.drawPixmap((width() - m_pixhover.width()) / 2, (height() - m_pixhover.height()) / 2, m_pixhover);

    if (m_pressed)
        p.drawPixmap((width() - m_pixselected.width()) / 2, (height() - m_pixselected.height()) / 2, m_pixselected);
}

void StackButton::setselected(bool sel)
{
    m_pressed = sel;
    update();
}

void StackButton::mousePressEvent(QMouseEvent *e)
{
    QPushButton::mousePressEvent(e);

    if (e->button() == Qt::LeftButton) {
        m_pressed = true;
        update();
    }
}

void StackButton::enterEvent(QEvent *e)
{
    QPushButton::enterEvent(e);

    m_enter = true;
    update();
}

void StackButton::leaveEvent(QEvent *e)
{
    QPushButton::leaveEvent(e);

    m_enter = false;
    update();
}

FourStateButton::FourStateButton(QString pixnormal, QWidget *parent) : QPushButton(parent)
  , m_index(0)
  , m_enter(false)
{
    this->setCursor(Qt::PointingHandCursor);

    QPixmap pix(pixnormal);
    for (int i = 0; i < 4; i++)
        m_pixlist << pix.copy(i * (pix.width() / 4), 0, pix.width() / 4, pix.height());
}

void FourStateButton::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    painter.drawPixmap((width() - m_pixlist.at(m_index).width()) / 2, (height() - m_pixlist.at(m_index).height()) / 2,
                       m_pixlist.at(m_index).width(),  m_pixlist.at(m_index).height(), m_pixlist.at(m_index));
}

void FourStateButton::enterEvent(QEvent *)
{
    m_index = 1;
    m_enter = true;

    update();
}

void FourStateButton::leaveEvent(QEvent *)
{
    m_index = 0;
    m_enter = false;

    update();
}

void FourStateButton::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        QPushButton::mousePressEvent(e);
        m_index = 2;
        update();
    }
}

void FourStateButton::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_index = 1;
        update();
        QPushButton::mouseReleaseEvent(e);
    }
}
