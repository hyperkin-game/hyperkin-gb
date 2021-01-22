#include "leftwidgets.h"

#include <QHBoxLayout>
#include <QLabel>

LeftWidgets::LeftWidgets(QWidget *parent) : BaseWidget(parent)
{
    setBackgroundColor(32, 38, 51);

    initLayout();
}

void LeftWidgets::initLayout()
{
    /* add setting tab item here.
       You nend change 'RowCount'、append title list and icon list */
    m_funtionlist = new Funtiontablewidget(this);
    m_funtionlist->setRowCount(7);

    QStringList normaliconlist, selectediconlist, namelist;

    normaliconlist.append(":/image/setting/wifi_normal.png");
    normaliconlist.append(":/image/setting/hotspot.png");
    normaliconlist.append(":/image/setting/bt_normal.png");
    normaliconlist.append(":/image/setting/brightness.png");
    //    normaliconlist.append(":/image/setting/calendar.png");
    normaliconlist.append(":/image/setting/speaker.png");
    normaliconlist.append(":/image/setting/update.png");
    normaliconlist.append(":/image/setting/language.png");

    selectediconlist.append(":/image/setting/wifi_seleted.png");
    selectediconlist.append(":/image/setting/hotspot.png");
    selectediconlist.append(":/image/setting/bt_seleted.png");
    selectediconlist.append(":/image/setting/brightness.png");
    //    selectediconlist.append(":/image/setting/calendar.png");
    selectediconlist.append(":/image/setting/speaker.png");
    selectediconlist.append(":/image/setting/update.png");
    selectediconlist.append(":/image/setting/language.png");

    m_funtionlist->addFunctionItems(normaliconlist, selectediconlist, namelist);

    QVBoxLayout *vmainlyout = new QVBoxLayout;
    vmainlyout->addWidget(m_funtionlist);
    vmainlyout->setMargin(0);
    vmainlyout->setSpacing(0);

    setLayout(vmainlyout);
}

LeftWidgets::~LeftWidgets()
{
}
