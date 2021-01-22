#include "imageviewer.h"

#include <QMatrix>
#include <QPoint>

#include "basewidget.h"
#include "constant.h"

ImageViewer::ImageViewer(QWidget *parent) : QLabel(parent)
  , m_showLoadTitle(false)
  , m_movie(new QMovie())
  , m_scale(0.0)
  , m_percentage(0.0)
  , m_originX(0.0)
  , m_originY(0.0)
  , m_basicX(0.0)
  , m_basicY(0.0)
  , currentStepScaleFactor(1)
  , m_loadThread(new PixmalLoadThread(this, &m_pixmap))
{
    setAttribute(Qt::WA_AcceptTouchEvents);

    grabGesture(Qt::PinchGesture);
    setAlignment(Qt::AlignCenter);

    // loading title font.
    setAlignment(Qt::AlignCenter);
    BaseWidget::setWidgetFontSize(this, font_size_big);
    BaseWidget::setWidgetFontBold(this);

    m_loadTitleShowTimer = new QTimer(this);
    m_loadTitleShowTimer->setSingleShot(true);
    connect(m_loadTitleShowTimer, SIGNAL(timeout()), this, SLOT(showLoadingTitle()));
    connect(m_loadThread, SIGNAL(finished()), this, SLOT(hideLoadingTitle()));
}

ImageViewer::~ImageViewer()
{
    delete m_movie;
}

bool ImageViewer::setPixmap(const QString &path)
{
    m_movie->stop();
    m_gifShow = false;

    if (m_loadThread->isRunning())
        return false;

    if (!m_loadTitleShowTimer->isActive())
        m_loadTitleShowTimer->start(800);

    m_loadThread->changeLoadPath(path);
    m_loadThread->start();
    return true;
}

void ImageViewer::setMoviePath(QString filePath)
{
    m_movie->stop();
    m_movie->setFileName(filePath);
    setMovie(m_movie);
    m_movie->start();

    m_gifShow = true;
    update();
}

void ImageViewer::hideLoadingTitle()
{
    m_loadTitleShowTimer->stop();

    m_showLoadTitle = false;
    setText("");
    update();
}

void ImageViewer::showLoadingTitle()
{
    m_showLoadTitle = true;
    setText(tr("image loading.."));
    update();
}

void ImageViewer::ariseScale(int rate)
{
    double old_percentage = m_percentage;
    double step = static_cast<double>(rate) / 100.0 * 5 * old_percentage;

    m_percentage += step;
    if (m_percentage < 0.01)
        m_percentage = 0.01;
    else if (m_percentage > 1.5)
        m_percentage = 1.5;
    m_scale = m_percentage * m_scale / old_percentage;

    update();
    emit percentageChanged(m_percentage);
}

void ImageViewer::showOriginalSize()
{
    double old_percentage = m_percentage;
    m_percentage = 1.0;
    m_scale = m_percentage * m_scale / old_percentage;

    update();
    emit percentageChanged(m_percentage);
}

void ImageViewer::showSuitableSize()
{
    double pixwidth = static_cast<double>(m_pixmap.width());
    double pixheight = static_cast<double>(m_pixmap.height());
    double showwidth = static_cast<double>(width());
    double showheight = static_cast<double>(height());

    double Wpercentage = showwidth / pixwidth;
    double Hpercentage = showheight / pixheight;
    m_percentage = qMin(Wpercentage, Hpercentage);
    m_suitableWidth = pixwidth * m_percentage;
    m_suitableHeight = pixheight * m_percentage;

    if (m_percentage < 0.01)
        m_percentage = 0.01;
    else if (m_percentage > 1)
        m_percentage = 1.0;

    m_scale = 1.0;

    m_basicX = showwidth / 2.0 - pixwidth * m_percentage / 2.0;
    m_originX = m_basicX;
    m_basicY = showheight / 2.0 - pixheight * m_percentage / 2.0;
    m_originY = m_basicY;

    update();
    emit percentageChanged(m_percentage);
}

void ImageViewer::zoomIn()
{
    ariseScale(1);
}

void ImageViewer::zoomOut()
{
    ariseScale(-1);
}

void ImageViewer::clockwise90()
{
    QMatrix matrix;
    matrix.rotate(90);

    m_pixmap = m_pixmap.transformed(matrix, Qt::FastTransformation);
    showSuitableSize();
}

void ImageViewer::anticlockwise90()
{
    QMatrix matrix;
    matrix.rotate(-90);

    m_pixmap = m_pixmap.transformed(matrix, Qt::FastTransformation);
    showSuitableSize();
}

QSize ImageViewer::getSize()
{
    return m_pixmap.size();
}

void ImageViewer::paintEvent(QPaintEvent *event)
{
    if (m_gifShow || m_showLoadTitle) {
        QLabel::paintEvent(event);
    } else {
        Q_UNUSED(event);

        double pixwidth = static_cast<double>(m_pixmap.width());
        double pixheight = static_cast<double>(m_pixmap.height());
        double showwidth = static_cast<double>(width());
        double showheight = static_cast<double>(height());

        double Wscalerate = pixwidth / showwidth;
        double Hscalerate = pixheight / showheight;
        double compare = qMax(Wscalerate, Hscalerate);

        if (compare < 1.0)
            compare = 1.0;

        QPainter painter(this);
        //Draw background
        painter.save();
        QRect backgroundRect = rect();
        QColor color(0, 0, 0);
        painter.setPen(color);
        painter.setBrush(QBrush(color));
        painter.drawRect(backgroundRect);
        painter.restore();

        //Draw Image
        QRect showRect = QRect(m_originX, m_originY, pixwidth / compare, pixheight / compare);
        painter.save();
        painter.translate(showwidth / 2.0, (showheight / 2.0));
        painter.scale(m_scale, m_scale);
        painter.translate(-(showwidth / 2.0), -(showheight / 2.0));
        painter.drawPixmap(showRect, m_pixmap);
        painter.restore();
    }
}

void ImageViewer::wheelEvent(QWheelEvent *event)
{
    QWidget::wheelEvent(event);

    int numDegrees = event->delta() / 8;
    int numSteps = numDegrees / 15;
    if (event->orientation() == Qt::Horizontal) {
        event->accept();
    } else {
        ariseScale(numSteps);
    }
}

void ImageViewer::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        m_pressPoint = event->pos();
    else if (event->button() == Qt::RightButton)
        emit rightButtonClicked();

    QWidget::mousePressEvent(event);
}

void ImageViewer::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if (event->button() == Qt::LeftButton) {
        m_basicX = m_originX;
        m_basicY = m_originY;
    }

    QWidget::mouseReleaseEvent(event);
}

void ImageViewer::mouseMoveEvent(QMouseEvent *event)
{
    if (m_pressPoint != QPoint(-1, -1)) {
        QPoint move_pos = event->pos();

        if (rect().contains(event->pos())) {
            move_pos -= m_pressPoint;
            m_originX = m_basicX + move_pos.x() / m_scale;
            m_originY = m_basicY + move_pos.y() / m_scale;
            update();
        } else {
            QPoint point;
            if (event->pos().x() < 0)
                point = QPoint( 0, event->pos().y());
            else if (event->pos().x() > rect().width()-1)
                point = QPoint( rect().width()-1, event->pos().y());
            else if (event->pos().y() < 0 )
                point = QPoint( event->pos().x(), 0);
            else if (event->pos().y() > rect().height() - 1)
                point = QPoint( event->pos().x(), rect().height()-1);
        }
    }

    QWidget::mouseMoveEvent(event);
}

bool ImageViewer::event(QEvent *event)
{
    if (event->type() == QEvent::Gesture) {
        QGestureEvent *gestureEvent = static_cast<QGestureEvent*>(event);
        if (QGesture *gesture = gestureEvent->gesture(Qt::PinchGesture)) {
            m_pressPoint = QPoint(-1, -1);
            QPinchGesture *pinchGesture = static_cast<QPinchGesture*>(gesture);
            QPinchGesture::ChangeFlags changeFlags = pinchGesture->changeFlags();
            if (changeFlags & QPinchGesture::ScaleFactorChanged) {
#ifdef DEVICE_EVB
                if (pinchGesture->totalScaleFactor() > 1.2)
                    ariseScale(1);
                else if (pinchGesture->totalScaleFactor() < 0.8)
                    ariseScale(-1);
#else
                if (pinchGesture->totalScaleFactor() > 1.6)
                    ariseScale(1);
                else if (pinchGesture->totalScaleFactor() < 0.4)
                    ariseScale(-1);
#endif
            }
        }
    }

    return QWidget::event(event);
}

PixmalLoadThread::PixmalLoadThread(ImageViewer *parent, QPixmap *pixmap) : QThread(parent)
  , m_parent(parent)
  , m_pixmap(pixmap)
{
}

PixmalLoadThread::~PixmalLoadThread()
{
    requestInterruption();
    quit();
    wait();
}

void PixmalLoadThread::changeLoadPath(const QString &path)
{
    m_loadPath = path;
}

void PixmalLoadThread::run()
{
    m_pixmap->load(m_loadPath);
    m_parent->showSuitableSize();
}
