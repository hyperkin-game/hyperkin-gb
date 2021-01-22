#ifndef QKEYBOARD_H
#define QKEYBOARD_H

#include "basewidget.h"

#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>

namespace Ui
{
class QKeyBoard;
}

class QKeyBoard : public BaseWidget
{
    Q_OBJECT
public:
    explicit QKeyBoard(QWidget *parent = 0);
    ~QKeyBoard();

    // singleton pattern
    static QKeyBoard* getInstance()
    {
        if (!_instance) {
            _instance = new QKeyBoard;
        }
        return _instance;
    }

    enum PanelStyle
    {
        Blue,
        Gray,
        Dev,
        LightGray,
        DarkGray,
        Black,
        Brown,
        Silvery
    };

    enum InputType
    {
        LowerCase,
        UpperCase
    };

    void globalInit(PanelStyle stype, int btnSize, int btnFontSize);

private:
    Ui::QKeyBoard *ui;
    static QKeyBoard* _instance;

    InputType cur_input_type;
    QLineEdit *cur_lineEdit;

    QPoint mousePoint;
    bool mousePressed;

    void initForm();
    void initProperty();

    void changePanelStyle(PanelStyle style);
    void changePanelStyle(QString topColor, QString bottomColor,
                          QString borderColor, QString textColor);
    void changeInputType(InputType type);
    void letterUpdate(bool isUpper);

    void showPanel();
    void hidePanel();

    void insertValue(const QString &value);

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);

private slots:
    void slot_onApplicationFocusChanged(QWidget *, QWidget *);
    void enableKeyBoard();
    void btn_clicked();
};

#endif // QKEYBOARD_H
