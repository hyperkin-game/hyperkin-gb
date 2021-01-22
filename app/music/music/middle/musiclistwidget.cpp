#include "musiclistwidget.h"
#include "constant.h"

#include <QHBoxLayout>

#ifdef DEVICE_EVB
int header_height = 70;
#else
int header_height = 35;
#endif

PlayListHeader::PlayListHeader(QWidget *parent) : BaseWidget(parent)
{
    setFixedHeight(header_height);
    setCursor(Qt::PointingHandCursor);

    initLayout();
}

void PlayListHeader::initLayout()
{
    QHBoxLayout *layout = new QHBoxLayout;

    m_listCountInfo = new QLabel(tr("song(0)"), this);
    m_listCountInfo->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QLabel *songTypeInfo = new QLabel(tr("type"), this);
    songTypeInfo->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    layout->addWidget(m_listCountInfo);
    layout->addWidget(songTypeInfo);
    layout->setContentsMargins(20, 0, 25, 0);
    layout->setSpacing(0);

    setLayout(layout);
}

void PlayListHeader::updateSongCountText(int songCount)
{
    m_listCountInfo->setText(tr("music(%1)").arg(QString::number(songCount)));
}

MusicListWidget::MusicListWidget(QWidget *parent) : BaseWidget(parent)
  , m_playlist(new MediaList(this))
{
    initLayout();
    initConnection();
}

void MusicListWidget::initLayout()
{
    QVBoxLayout *layout = new QVBoxLayout;

    m_header = new PlayListHeader(this);
    m_table = new MusicListTable(this);

    layout->addWidget(m_header);
    layout->addWidget(m_table);
    layout->setMargin(0);
    layout->setSpacing(0);

    setLayout(layout);
}

void MusicListWidget::initConnection()
{
    connect(m_table, SIGNAL(cellClicked(int,int)), this, SIGNAL(tableClick(int,int)));
    connect(m_table, SIGNAL(longPressedEvent(int)), this, SIGNAL(tableLongPressed(int)));
}

void MusicListWidget::setPlayingMediaContent(QString filaPath)
{
    QList<QString> pathList = m_playlist->getPathList();
    int index = -1;
    for (int i = 0; i < pathList.size(); i++) {
        if (pathList.at(i) == filaPath) {
            index = i;
            break;
        }
    }

    if (index != -1) {
        m_table->playingItemChanged(index);
        m_playlist->setCurrentIndex(index);
    }
}

void MusicListWidget::setOriginState()
{
    m_table->setOriginState();
}

void MusicListWidget::deleteItem(int row)
{
    m_table->removeTableItem(row);
    m_playlist->removeItem(row);
    m_header->updateSongCountText(m_table->rowCount());

}

void MusicListWidget::insertIntoTable(const QFileInfo &fileInfo)
{
    QString fileName = fileInfo.baseName();
    QString fileSuffix = fileInfo.suffix();
    QString filePath = fileInfo.absoluteFilePath();

    m_table->insertIntoTable(fileName, fileSuffix);
    m_playlist->addPlayList(filePath);
    m_header->updateSongCountText(m_table->rowCount());
}

void MusicListWidget::updateLocalList(QFileInfoList fileInfoList)
{
    // remove table and set song count first
    m_table->clearTable();;
    m_playlist->clearList();
    m_header->updateSongCountText(m_table->rowCount());

    for (int i = 0; i < fileInfoList.size(); i++) {
        QFileInfo fileInfo = fileInfoList.at(i);
        if (!m_playlist->getPathList().contains(fileInfo.absoluteFilePath()))
            insertIntoTable(fileInfo);
    }
}
