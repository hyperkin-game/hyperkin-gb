#include "focusswitchmanager.h"

#include <QMouseEvent>
#include <QApplication>

FocusSwitchManager* FocusSwitchManager::m_pInstance = NULL;

FocusSwitchManager::FocusSwitchManager() :
    m_currentFocusWidget(0),
    rowCount(0),
    m_currentRow(0),
    m_currentColume(0)
{
}

FocusSwitchManager* FocusSwitchManager::getInstance()
{
    if (m_pInstance == NULL)
        m_pInstance = new FocusSwitchManager();

    return m_pInstance;
}

void FocusSwitchManager::clickCurrentWidget()
{    
    if (m_currentFocusWidget != NULL) {
        QMouseEvent pressEvent(QEvent::MouseButtonPress, QPoint(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(m_currentFocusWidget, &pressEvent);

        QMouseEvent releaseEvent(QEvent::MouseButtonRelease, QPoint(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(m_currentFocusWidget, &releaseEvent);
    }
}

void FocusSwitchManager::focusNextChild()
{
    if (columnCount[m_currentRow] >= m_currentColume + 1) {
        m_currentColume = m_currentColume + 1;
        m_currentFocusWidget = findWidget(m_currentRow, m_currentColume);
        m_currentFocusWidget->setFocus();
    }
}

void FocusSwitchManager::focusPreviousChild()
{
    if (m_currentColume - 1 > 0) {
        m_currentColume = m_currentColume - 1;
        m_currentFocusWidget = findWidget(m_currentRow, m_currentColume);
        m_currentFocusWidget->setFocus();
    }
}

void FocusSwitchManager::focusAboveChild()
{
    if (m_currentRow - 1 > 0) {
        m_currentRow = m_currentRow - 1;
        m_currentColume = 1;
        m_currentFocusWidget = findWidget(m_currentRow, m_currentColume);
        m_currentFocusWidget->setFocus();
    }
}

void FocusSwitchManager::focusBelowChild()
{
    if (rowCount >= m_currentRow + 1) {
        m_currentRow = m_currentRow + 1;
        m_currentColume = 1;
        m_currentFocusWidget = findWidget(m_currentRow, m_currentColume);
        m_currentFocusWidget->setFocus();
    }
}

QWidget* FocusSwitchManager::findWidget(int row, int colume)
{
    QMap<QString, QWidget*>::Iterator it;
    for (it = m_widgets.begin(); it != m_widgets.end(); it++) {
        QString key = it.key();
        if (key.split(",").at(0).toInt() == row && key.split(",").at(1).toInt() == colume)
            return it.value();
    }

    return NULL;
}

void FocusSwitchManager::insertIntoMap(const QString &key, QWidget *value)
{
    int row = key.split(",").at(0).toInt();
    int colume = key.split(",").at(1).toInt();

    rowCount = row > rowCount ? row : rowCount;
    columnCount[row] = colume > columnCount[row] ? colume : columnCount[row];
    m_widgets.insert(key, value);
}
