#include "quickinterfacewidget.h"
#include "constant.h"

QuickInterfaceWidget::QuickInterfaceWidget(QWidget *parent) : QQuickWidget(parent)
{
    init();

    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    // initialize timer in order to distinguish click and double click.
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onOneClick()));
}

void QuickInterfaceWidget::init()
{
    this->setMouseTracking(true);
    this->setCursor(QCursor(Qt::ArrowCursor));
    this->setAutoFillBackground(true);
    this->setWindowOpacity(1);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    this->setFocusPolicy(Qt::ClickFocus);
    this->setAcceptDrops(true);
}

void QuickInterfaceWidget::mousePressEvent(QMouseEvent*)
{
    m_timer->start(300);
}

void QuickInterfaceWidget::mouseDoubleClickEvent(QMouseEvent*)
{
    m_timer->stop();
    emit contentDoubleClick();
}

void QuickInterfaceWidget::onOneClick()
{
    m_timer->stop();
    emit contentOneClick();
}
