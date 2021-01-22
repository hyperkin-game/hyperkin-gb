#include "listwidget.h"
#include "constant.h"

#include <QFileDialog>
#include <QFile>

ListWidgets::ListWidgets(QWidget *parent) : BaseWidget(parent)
{
    setBackgroundColor(43, 45, 51);

    initData();
    initLayout();
    initConnection();
}

void ListWidgets::initData()
{
    m_playList = new MediaList(this);
}

void ListWidgets::initLayout()
{
    QVBoxLayout *vmianlyout = new QVBoxLayout;

    /*
     * the list widegts includes with header and list.
     * and the 'list' include net list and local list.
     */
    m_listHeader = new ListHeader(this);

    m_stackedWid = new QStackedWidget(this);
    m_localTable = new LocalListTable(m_stackedWid);
    m_netTable = new BaseTableWidget(m_stackedWid);

    m_stackedWid->addWidget(m_localTable);
    m_stackedWid->addWidget(m_netTable);

    vmianlyout->addWidget(m_listHeader);
    vmianlyout->addWidget(m_stackedWid);
    vmianlyout->setContentsMargins(10, 0, 10, 0);

    setLayout(vmianlyout);
}

void ListWidgets::initConnection()
{
    connect(m_listHeader, SIGNAL(buttonLocalClick()), this, SLOT(slot_switchToLocalList()));
    connect(m_listHeader, SIGNAL(buttonNetClick()), this, SLOT(slot_switchToNetList()));
    connect(m_localTable, SIGNAL(cellClicked(int,int)), this, SIGNAL(sig_localTableItemClick(int,int)));
    connect(m_localTable, SIGNAL(longPressedEvent(int)), this, SIGNAL(tableLongPressed(int)));
}

void ListWidgets::setOriginState()
{
    m_localTable->setOriginState();
}

void ListWidgets::deleteItem(int row)
{
    QFile file(m_playList->getUrlAt(row).path());
    if (file.exists())
        file.remove();

    m_localTable->removeTableItem(row);
    m_playList->removeItem(row);
}

void ListWidgets::updateResUi(QFileInfoList fileList)
{
    m_localTable->clearTable();
    m_playList->clearList();

    for (int i = 0; i < fileList.size(); i++) {
        QFileInfo fileInfo = fileList.at(i);
        if (!m_playList->getUrlList().contains(QUrl::fromLocalFile(fileInfo.absoluteFilePath()))) {
            m_localTable->insertIntoTable(fileInfo.fileName(), "");
            m_playList->addPlayList(fileInfo.absoluteFilePath());
        }
    }
}

void ListWidgets::updatePlayingItemStyle(QMediaContent content)
{
    QList<QUrl> urlList = m_playList->getUrlList();
    int index = -1;
    for (int i = 0; i < urlList.size(); i++) {
        if (urlList.at(i) == content.canonicalUrl()) {
            index = i;
            break;
        }
    }

    if (index != -1)
        m_localTable->playingItemChanged(index);
}

void ListWidgets::addVideo()
{
    QFileDialog *dialog = new QFileDialog(mainWindow, "Selete File");
    if (dialog->exec() == QFileDialog::Accepted) {
        QStringList files = dialog->selectedFiles();
        if (files.isEmpty())
            return;
        for (int i = 0; i < files.count(); i++) {
            QFileInfo info(files[i]);
            if (!m_playList->getUrlList().contains(QUrl::fromLocalFile(files.value(i)))
                    && info.exists())
                m_localTable->insertIntoTable(info.fileName(), "");
        }
    }
}

ListWidgets::~ListWidgets()
{
}
