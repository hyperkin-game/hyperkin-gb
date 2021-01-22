#ifndef BRIGHTNESS_H
#define BRIGHTNESS_H

#include <QWidget>
#include <QFile>
#include <QDir>

namespace Ui {
class Brightness;
}

class Brightness : public QWidget
{
    Q_OBJECT

public:
    explicit Brightness(QWidget *parent = 0);
    ~Brightness();

private:
    static void onBrightnessChange(int brightness);
    void saveBrightness(int brightness);

private slots:
    void on_m_BrightnessHorizontalSlider_sliderMoved(int position);
    void on_m_BrightnessHorizontalSlider_actionTriggered(int action);
    void on_m_BrightnessHorizontalSlider_valueChanged(int value);
    void on_m_BrightnessDownPushButton_clicked();
    void on_m_BrightnessUpPushButton_clicked();
    void on_m_BrightnessHorizontalSlider_sliderReleased();

private:
    Ui::Brightness *ui;
    QFile *brightnessFile;
};

#endif // BRIGHTNESS_H
