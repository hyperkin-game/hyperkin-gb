#ifndef VIDEOQUICKCONTENTWIDGET_H
#define VIDEOQUICKCONTENTWIDGET_H

#include <QQuickWidget>
#include <QTimer>

class QuickInterfaceWidget : public QQuickWidget
{
    Q_OBJECT
public:
    QuickInterfaceWidget(QWidget *parent = 0);
    ~QuickInterfaceWidget(){}

private:
    QTimer *m_timer;

    void init();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

signals:
    void contentOneClick();
    void contentDoubleClick();

private slots:
    void onOneClick();
};

#endif // VIDEOQUICKCONTENTWIDGET_H
