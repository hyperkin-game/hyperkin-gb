#ifndef IMAGEBROWSER_IMAGEHANDLER_H
#define IMAGEBROWSER_IMAGEHANDLER_H

#include <QLabel>
#include <QPixmap>
#include <QPainter>
#include <QPaintEvent>
#include <QWheelEvent>
#include <QResizeEvent>
#include <QGestureEvent>
#include <QPinchGesture>
#include <QMovie>
#include <QThread>

class PixmalLoadThread;

class ImageViewer : public QLabel
{
    Q_OBJECT
public:
    explicit ImageViewer(QWidget *parent = 0);
    ~ImageViewer();

    bool setPixmap(const QString &path);
    void setMoviePath(QString filePath);

    void showOriginalSize();
    void showSuitableSize();
    void zoomIn();
    void zoomOut();
    void clockwise90();
    void anticlockwise90();
    QSize getSize();

private:
    QTimer *m_loadTitleShowTimer;
    bool m_showLoadTitle;
    bool m_gifShow;
    QPixmap m_pixmap;
    QMovie *m_movie;

    double m_scale;
    double m_percentage;

    QPoint m_pressPoint;
    double m_originX;
    double m_originY;
    double m_basicX;
    double m_basicY;

    int m_suitableWidth;
    int m_suitableHeight;

    qreal currentStepScaleFactor;
    PixmalLoadThread *m_loadThread;
    void ariseScale(int rate);

signals:
    void percentageChanged(double percentage);
    void rightButtonClicked();

public slots:
    void hideLoadingTitle();
    void showLoadingTitle();

protected:
    void paintEvent(QPaintEvent *event);
    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    bool event(QEvent *event) Q_DECL_OVERRIDE;
};

class PixmalLoadThread : public QThread
{
public:
    explicit PixmalLoadThread(ImageViewer *parent, QPixmap *pixmap);
    ~PixmalLoadThread();

    void changeLoadPath(const QString &path);

protected:
    void run();

private:
    ImageViewer *m_parent;
    QPixmap *m_pixmap;
    QString m_loadPath;
};

#endif // IMAGEBROWSER_IMAGEHANDLER_H
