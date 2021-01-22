#include "qkeyboard.h"
#include "ui_qkeyboard.h"
#include "retranslatemanager.h"

#include <QTimer>
#include <QDesktopWidget>

QKeyBoard* QKeyBoard::_instance = 0;

#define Default_Button_Size 40
#define Default_Font_Size 13
#define Default_Max_Edit_Length 20

QKeyBoard::QKeyBoard(QWidget *parent) : BaseWidget(parent),
    ui(new Ui::QKeyBoard),
    cur_input_type(LowerCase),
    cur_lineEdit(0),
    mousePressed(false)
{
    ui->setupUi(this);

    initForm();
    initProperty();
    globalInit(QKeyBoard::Black, Default_Button_Size, Default_Font_Size);
}

QKeyBoard::~QKeyBoard()
{
    delete ui;
}

void QKeyBoard::globalInit(PanelStyle stype, int btnSize, int btnFontSize)
{
    changePanelStyle(stype);

    QFont btnFont(this->font().family(), btnFontSize);
    QList<QPushButton*> btns = this->findChildren<QPushButton*>();
    foreach (QPushButton *button, btns) {
        if (button->objectName() == "btnSpace" || button->objectName() == "btnType"
                || button->objectName() == "btn0" || button->objectName() == "btnClose") {
            button->setFixedSize(btnSize * 2, btnSize);
        } else {
            button->setFixedSize(btnSize, btnSize);
        }

        button->setFont(btnFont);
    }

    ui->labInfo->setFont(btnFont);
}

void QKeyBoard::initForm()
{
    this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);

    // initialize application fouce changed event for show or hide this panel.
    connect(qApp, SIGNAL(focusChanged(QWidget*, QWidget*)),
            this, SLOT(slot_onApplicationFocusChanged(QWidget*, QWidget*)));
    qApp->installEventFilter(this);

    QList<QPushButton*> btns = this->findChildren<QPushButton*>();
    foreach (QPushButton *button, btns) {
        connect(button, SIGNAL(clicked()), this, SLOT(btn_clicked()));
    }

    QTimer::singleShot(100, this, SLOT(enableKeyBoard()));
}

void QKeyBoard::initProperty()
{
    // there button mark as letter for upper or lower.
    ui->btna->setProperty("btnLetter", true);
    ui->btnb->setProperty("btnLetter", true);
    ui->btnc->setProperty("btnLetter", true);
    ui->btnd->setProperty("btnLetter", true);
    ui->btne->setProperty("btnLetter", true);
    ui->btnf->setProperty("btnLetter", true);
    ui->btng->setProperty("btnLetter", true);
    ui->btnh->setProperty("btnLetter", true);
    ui->btni->setProperty("btnLetter", true);
    ui->btnj->setProperty("btnLetter", true);
    ui->btnk->setProperty("btnLetter", true);
    ui->btnl->setProperty("btnLetter", true);
    ui->btnm->setProperty("btnLetter", true);
    ui->btnn->setProperty("btnLetter", true);
    ui->btno->setProperty("btnLetter", true);
    ui->btnp->setProperty("btnLetter", true);
    ui->btnq->setProperty("btnLetter", true);
    ui->btnr->setProperty("btnLetter", true);
    ui->btns->setProperty("btnLetter", true);
    ui->btnt->setProperty("btnLetter", true);
    ui->btnu->setProperty("btnLetter", true);
    ui->btnv->setProperty("btnLetter", true);
    ui->btnw->setProperty("btnLetter", true);
    ui->btnx->setProperty("btnLetter", true);
    ui->btny->setProperty("btnLetter", true);
    ui->btnz->setProperty("btnLetter", true);
}

void QKeyBoard::changePanelStyle(PanelStyle style)
{
    switch (style) {
    case Blue:
        changePanelStyle("#DEF0FE", "#C0DEF6", "#C0DCF2", "#386487");
        break;
    case Dev:
        changePanelStyle("#C0D3EB", "#BCCFE7", "#B4C2D7", "#324C6C");
        break;
    case Gray:
        changePanelStyle("#E4E4E4", "#A2A2A2", "#A9A9A9", "#000000");
        break;
    case LightGray:
        changePanelStyle("#EEEEEE", "#E5E5E5", "#D4D0C8", "#6F6F6F");
        break;
    case DarkGray:
        changePanelStyle("#D8D9DE", "#C8C8D0", "#A9ACB5", "#5D5C6C");
        break;
    case Black:
        changePanelStyle("#4D4D4D", "#292929", "#D9D9D9", "#CACAD0");
        break;
    case Brown:
        changePanelStyle("#667481", "#566373", "#C2CCD8", "#E7ECF0");
        break;
    case Silvery:
        changePanelStyle("#E1E4E6", "#CCD3D9", "#B2B6B9", "#000000");
        break;
    default:
        break;
    }
}

void QKeyBoard::changePanelStyle(QString topColor, QString bottomColor,
                                 QString borderColor, QString textColor)
{
    QStringList qss;

    qss.append(QString("QWidget#QKeyBoard{background:qlineargradient(spread:pad,x1:0,y1:0,x2:0,y2:1,stop:0 %1,stop:1 %2);}")
               .arg(topColor).arg(bottomColor));
    qss.append("QPushButton{padding:5px;border-radius:3px;}");
    qss.append(QString("QPushButton:hover{background:qlineargradient(spread:pad,x1:0,y1:0,x2:0,y2:1,stop:0 %1,stop:1 %2);}")
               .arg(topColor).arg(bottomColor));
    qss.append(QString("QPushButton::pressed{padding:4px;background:rgb(0,0,0)}"));
    qss.append(QString("QLabel,QPushButton{color:%1;}").arg(textColor));
    qss.append(QString("QPushButton#btnPre,QPushButton#btnNext,QPushButton#btnClose{padding:5px;}"));
    qss.append(QString("QPushButton{border:1px solid %1;}")
               .arg(borderColor));
    qss.append(QString("QLineEdit{border:1px solid %1;border-radius:5px;padding:2px;background:none;selection-background-color:%2;selection-color:%3;}")
               .arg(borderColor).arg(bottomColor).arg(topColor));

    this->setStyleSheet(qss.join(""));
}

void QKeyBoard::changeInputType(InputType type)
{
    switch (type) {
    case UpperCase:
        ui->btnType->setText(str_board_upper);
        ui->labInfo->setText(str_board_lab_upper);
        letterUpdate(true);
        break;
    case LowerCase:
    default:
        ui->btnType->setText(str_board_lower);
        ui->labInfo->setText(str_board_lab_lower);
        letterUpdate(false);
        break;
    }
}

void QKeyBoard::letterUpdate(bool isUpper)
{
    QList<QPushButton*> buttons = this->findChildren<QPushButton*>();
    foreach (QPushButton *button, buttons) {
        if (button->property("btnLetter").toBool()) {
            if (isUpper)
                button->setText(button->text().toUpper());
            else
                button->setText(button->text().toLower());
        }
    }
}

void QKeyBoard::showPanel()
{
    changeInputType(cur_input_type);
    ui->btnClose->setText(str_board_close);
    this->setVisible(true);
}

void QKeyBoard::hidePanel()
{
    this->setVisible(false);
}

void QKeyBoard::enableKeyBoard()
{
    showPanel();
    hidePanel();
}

void QKeyBoard::slot_onApplicationFocusChanged(QWidget *, QWidget *nowWidget)
{
    if (nowWidget != 0 && !this->isAncestorOf(nowWidget)) {
        if (nowWidget->inherits("QLineEdit")) {
            cur_lineEdit = (QLineEdit*)nowWidget;
            showPanel();
        } else {
            hidePanel();
        }

        // set KeyBoard layout in the bottom of current fouces widget
        QDesktopWidget desktop;
        QRect rect = nowWidget->rect();
        QPoint pos = QPoint(rect.left(), rect.bottom() + 20);
        pos = nowWidget->mapToGlobal(pos);
        this->move((desktop.width() - width()) / 2, pos.y());
    }
}

void QKeyBoard::btn_clicked()
{
    if (cur_lineEdit == 0) {
        return;
    }

    QPushButton *button = (QPushButton*)sender();
    QString objectName = button->objectName();
    if (objectName == "btnDelete") {
        cur_lineEdit->backspace();
    } else if (objectName == "btnClose") {
        // foucs other widget first.
        if (cur_lineEdit) {
            QWidget tempWidget(cur_lineEdit->parentWidget());
            tempWidget.show();
            tempWidget.setFocus();
        }
        hidePanel();
    } else if (objectName == "btnSpace") {
        insertValue(" ");
    } else if (objectName == "btnType") {
        cur_input_type = (cur_input_type == UpperCase) ? LowerCase : UpperCase;
        changeInputType(cur_input_type);
    } else {
        QString value = button->text();
        // letter "&"
        if (objectName == "btnOther7") {
            insertValue(value.right(1));
        } else {
            insertValue(value);
        }
    }
}

void QKeyBoard::insertValue(const QString &value)
{
    if (cur_lineEdit->text().length() < Default_Max_Edit_Length)
        cur_lineEdit->insert(value);
}

void QKeyBoard::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        mousePressed = true;
        mousePoint = event->globalPos() - this->pos();
        event->accept();
    }
}

void QKeyBoard::mouseMoveEvent(QMouseEvent *event)
{
    if (mousePressed && event->buttons() == Qt::LeftButton) {
        this->move(event->globalPos() - mousePoint);
        event->accept();
    }
}

void QKeyBoard::mouseReleaseEvent(QMouseEvent *)
{
    mousePressed = false;
}
