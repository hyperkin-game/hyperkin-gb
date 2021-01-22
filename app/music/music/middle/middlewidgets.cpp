#include "middlewidgets.h"

#include <QHBoxLayout>

MiddleWidgets::MiddleWidgets(QWidget *parent) : BaseWidget(parent)
{
    setTextColorBlack();

    initLayout();
}

void MiddleWidgets::initLayout()
{
    QHBoxLayout *hyout = new QHBoxLayout;

    m_listWid = new MusicListWidget(this);
    m_lyricWid = new LyricWidget(this);

    QFrame *splitter = new QFrame(this);
    splitter->setFixedWidth(1);
    splitter->setStyleSheet("QFrame{border:1px solid rgb(100,100,100,255);}");
    splitter->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    hyout->addWidget(m_listWid, 1);
    hyout->addWidget(splitter);
    hyout->addWidget(m_lyricWid, 2);
    hyout->setSpacing(0);
    hyout->setMargin(0);

    setLayout(hyout);
}

MiddleWidgets::~MiddleWidgets()
{
}
