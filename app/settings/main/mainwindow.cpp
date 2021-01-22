#include "mainwindow.h"
#include "retranslatemanager.h"
#include "constant.h"

#include <QApplication>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) : BaseWindow(parent)
{
    initData();
    initLayout();
}

void MainWindow::initData()
{
    mainWindow = this;

    RetranslateManager *manager = RetranslateManager::getInstance(this);
    manager->updateString();
    connect(this, SIGNAL(retranslateUi()), manager, SLOT(updateString()));
}

void MainWindow::initLayout()
{
    QVBoxLayout *mainLayout = new QVBoxLayout;

    m_setttingsWid = new SettingWidgets(this);

    mainLayout->addWidget(m_setttingsWid);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    setLayout(mainLayout);
}

void MainWindow::disableApplication()
{
    qDebug("disable settting application");
    this->setVisible(false);
}

void MainWindow::enableApplication()
{
    qDebug("enable setting application");
    this->setVisible(true);
}

void MainWindow::slot_appQuit()
{
    qApp->closeAllWindows();
}
