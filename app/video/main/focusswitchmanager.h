#ifndef FOCUSSWITCHMANAGER_H
#define FOCUSSWITCHMANAGER_H

#include <QWidget>
#include <QMap>

#define MAX_COLUME_NUM  20

class FocusSwitchManager
{
public:
    static FocusSwitchManager* getInstance();

    void clickCurrentWidget();
    QWidget* findWidget(int row, int colume);
    void insertIntoMap(const QString&, QWidget*);

    void focusPreviousChild();
    void focusNextChild();
    void focusAboveChild();
    void focusBelowChild();

private:
    FocusSwitchManager();
    static FocusSwitchManager* m_pInstance;

    QWidget *m_currentFocusWidget;
    QMap<QString, QWidget*> m_widgets;

    int rowCount;
    int columnCount[MAX_COLUME_NUM] = {0};
    int m_currentRow;
    int m_currentColume;
};

#endif // FOCUSSWITCHMANAGER_H
