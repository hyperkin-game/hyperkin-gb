#ifndef SWITCHBUTTON_H
#define SWITCHBUTTON_H

#include <QWidget>

class QTimer;

class SwitchButton: public QWidget
{
    Q_OBJECT
public:
    enum ButtonStyle {
        ButtonStyle_Rect = 0,
        ButtonStyle_CircleIn = 1,
        ButtonStyle_CircleOut = 2,
        ButtonStyle_Image = 3
    };

    SwitchButton(QWidget *parent = 0);
    ~SwitchButton();

protected:
    void mousePressEvent(QMouseEvent *);
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);
    void drawBg(QPainter *painter);
    void drawSlider(QPainter *painter);
    void drawText(QPainter *painter);
    void drawImage(QPainter *painter);

private:
    bool checked;
    ButtonStyle buttonStyle;

    QColor bgColorOff;
    QColor bgColorOn;

    QColor sliderColorOff;
    QColor sliderColorOn;

    QColor textColorOff;
    QColor textColorOn;

    QString textOff;
    QString textOn;

    QString imageOff;
    QString imageOn;

    int space;
    int rectRadius;

    int step;
    int startX;
    int endX;
    QTimer *timer;

private slots:
    void updateValue();

public:
    bool getChecked()const
    {
        return checked;
    }

    ButtonStyle getButtonStyle()const
    {
        return buttonStyle;
    }

    QColor getBgColorOff()const
    {
        return bgColorOff;
    }
    QColor getBgColorOn()const
    {
        return bgColorOn;
    }

    QColor getSliderColorOff()const
    {
        return sliderColorOff;
    }
    QColor getSliderColorOn()const
    {
        return sliderColorOn;
    }

    QColor getTextColorOff()const
    {
        return textColorOff;
    }
    QColor getTextColorOn()const
    {
        return textColorOn;
    }

    QString getTextOff()const
    {
        return textOff;
    }
    QString getTextOn()const
    {
        return textOn;
    }

    QString getImageOff()const
    {
        return imageOff;
    }
    QString getImageOn()const
    {
        return imageOn;
    }

    int getSpace()const
    {
        return space;
    }
    int getRectRadius()const
    {
        return rectRadius;
    }

public slots:
    void setChecked(bool checked);
    void setButtonStyle(ButtonStyle buttonStyle);

    void setBgColor(QColor bgColorOff, QColor bgColorOn);
    void setSliderColor(QColor sliderColorOff, QColor sliderColorOn);
    void setTextColor(QColor textColorOff, QColor textColorOn);
    void setText(QString textOff, QString textOn);
    void setImage(QString imageOff, QString imageOn);
    void setSpace(int space);
    void setRectRadius(int rectRadius);

signals:
    void checkedChanged(bool checked);
};

#endif // SWITCHBUTTON_H
