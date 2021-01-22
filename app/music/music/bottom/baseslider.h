#ifndef BASESLIDER_H
#define BASESLIDER_H

#include <QSlider>
#include <QMouseEvent>

class BaseSlider : public QSlider
{
    Q_OBJECT
public:
    BaseSlider(Qt::Orientation orientation, QWidget *parent = 0);
    ~BaseSlider(){}

private:
    void init();

protected:
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);

signals:
    void sig_sliderPositionChanged(int);
};

#endif // BASESLIDER_H
