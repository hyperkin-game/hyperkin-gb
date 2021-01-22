#ifndef BASEPUSHBUTTON_H
#define BASEPUSHBUTTON_H

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QEvent>
#include <QList>
#include <QMouseEvent>
#include <QTimer>
#include <QSlider>
#include <QPainter>
#include <QRect>

class RotatableButton : public QPushButton
{
    Q_OBJECT
public:
    RotatableButton(const QString &icon, QWidget *parent);

    bool isAnimated () const;

public slots:
    void startAnimation();
    void stopAnimation();

protected:
    virtual void timerEvent(QTimerEvent * event);
    virtual void paintEvent(QPaintEvent * event);

private:
    unsigned int angle_;
    int timerId_;
    int delay_;
    bool displayedWhenStopped_;
    QColor color_;
    short progress_;
    QPixmap currentPix_;
};

class FlatButton : public QPushButton
{
    Q_OBJECT
public:
    FlatButton(QWidget *parent = 0);
    FlatButton(const QString& str ,QWidget *parent = 0);

    void setBackgroundImage(const QString &imageRess);

private:
    // it stands for the button current is be long pressed.
    bool longPressedFlag;
    // used for identify long press event.
    QTimer *m_timer;

private slots:
    void slot_timerTimeout();

protected:
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);

signals:
    void longPressedEvent();
};

class FourStateButton : public QPushButton
{
    Q_OBJECT
public:
    FourStateButton(QString pix_listurl, QWidget *parent);

protected:
    void paintEvent(QPaintEvent*);
    void enterEvent(QEvent*);
    void leaveEvent(QEvent*);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e) override;

private:
    QList<QPixmap> m_pixlist;
    int m_index;
    bool m_enter;
};

class VolButton : public QPushButton
{
    Q_OBJECT
public:
    VolButton(const QString& normal, QWidget *parent=0);
    void setParentSlider(QSlider* slider)
    {
        m_partnerslider=slider;
    }
protected:
    void enterEvent(QEvent*);
    void leaveEvent(QEvent*);
    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent *e)override;
    void mouseReleaseEvent(QMouseEvent *e)override;
private:
    bool m_isenter;
    int m_savevol;
    int m_isvolempty;
    int m_index;
    QList<QPixmap> m_listnormal;
    QList<QPixmap> m_listhover;
    QList<QPixmap> m_listpressed;

    QSlider *m_partnerslider;

public slots:
    void setButtonPixmap(int);

signals:
    void setMute(int);
};

class StackButton : public QPushButton
{
    Q_OBJECT
public:
    explicit StackButton(const QString& pixnormal, const QString& pixhover,
                         const QString& pixsel, QWidget *parent);
    void setselected(bool = true);

protected:
    void mousePressEvent(QMouseEvent *e);
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    void paintEvent(QPaintEvent *);

private:
    int m_index;
    bool m_enter;
    bool m_pressed;
    QPixmap m_pixnormal;
    QPixmap m_pixhover;
    QPixmap m_pixselected;
};

#endif // BASEPUSHBUTTON_H
