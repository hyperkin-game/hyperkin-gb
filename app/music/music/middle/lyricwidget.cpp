#include "lyricwidget.h"

#include <QVBoxLayout>
#include <QPainter>

LyricWidget::LyricWidget(QWidget *parent) : AbstractWheelWidget(parent)
{
    initData();
    initLayout();
}

void LyricWidget::initData()
{
    m_lyricUtil = new LyricUtil();
}

void LyricWidget::initLayout()
{
    QVBoxLayout *layout = new QVBoxLayout;

    m_lblTip = new QLabel(this);
    m_lblTip->setAlignment(Qt::AlignCenter);

    layout->addWidget(m_lblTip);
    layout->setMargin(0);
    setLayout(layout);
}

void LyricWidget::setOriginState()
{
    m_lblTip->setText(tr("current no song playing"));
    clearLrc();
}

void LyricWidget::clearLrc()
{
    m_lyricUtil->clear();
    m_maskLength = -1000;
    m_currentRollrect = QRect(0, 0, 0, 0);
    m_realCurrentText = "";

    update();
}

void LyricWidget::currentMediaChanged(const QString &mediaTitle, const QString &currentMedia)
{
    QString filePath = currentMedia;
    QString fileName = currentMedia.mid(currentMedia.lastIndexOf("/") + 1);

    m_currentMedia = currentMedia;
    if (mediaTitle != NULL && mediaTitle != "")
        m_lblTip->setText(tr("Current music: ") + mediaTitle);
    else
        m_lblTip->setText(tr("Current Playing: ") + fileName);

    // load lyric file.
    QString lrcPath = filePath.remove(filePath.lastIndexOf('.'), filePath.size()).append(".lrc");
    if (m_lyricUtil->analyzeLrcContent(lrcPath))
        m_lblTip->setText("");
}

int LyricWidget::itemHeight() const
{
    QFontMetrics fm(this->font());
    return fm.height() * 2;
}

int LyricWidget::itemCount() const
{
    return m_lyricUtil->getCount();
}

void LyricWidget::paintItem(QPainter *painter, int index, QRect &rect)
{
    int ih = itemHeight()*1.2/10;

    if (index == m_currentItem) {
        QFont font = this->font();
        font.setPixelSize(font.pixelSize() +ih);
        painter->setFont(font);

        m_currentRollrect = rect;
        m_currentMaskFont = painter->font();
    }

    if (index == m_currentItem + 1) {
        QFont font = this->font();
        font.setPixelSize(font.pixelSize());
        painter->setFont(font);
    }

    QFontMetrics metrics(painter->font());
    QString text = m_lyricUtil->getLineAt(index);

    painter->drawText((rect.width() - metrics.width(text)) / 2,
                      rect.y() + (rect.height() - metrics.height()) / 2,
                      metrics.width(text),
                      rect.height(),
                      Qt::AlignLeft,
                      text);
}

void LyricWidget::paintItemMask(QPainter *painter)
{
    painter->setFont(m_currentMaskFont);
    painter->setPen(m_lrcHightLight);

    QFontMetrics metrics(m_currentMaskFont);
    painter->drawText((m_currentRollrect.width() - metrics.width(m_realCurrentText)) / 2,
                      m_currentRollrect.y() + (m_currentRollrect.height() - metrics.height()) / 2,
                      m_maskLength,
                      m_currentRollrect.height(),
                      Qt::AlignLeft,
                      m_realCurrentText);
}

void LyricWidget::slot_timerWork()
{
    if (m_itemOffset != 0)
        return;

    int interval = 0;
    float precent = 0;
    QString str=" ";
    // m_lyricUtil->getItemPrecent(m_pos,interval,precent,str);

    QFontMetrics metrics(m_currentMaskFont);
    if (m_itemPrecent==precent) {
        qreal count = interval / 25;
        float lrcMaskMiniStep = metrics.width(str) / count;
        m_maskLength+=lrcMaskMiniStep;
    } else {
        m_maskLength=metrics.width(m_realCurrentText) * precent;
    }

    update();
    m_itemPrecent = precent;
}

void LyricWidget::onCurrentPositionChanged(qint64 positon)
{
    int index = m_lyricUtil->getIndex(positon);
    if (index != m_currentItem && m_itemOffset == 0) {
        m_maskLength = -1000;
        m_currentRollrect = QRect(0, 0, 0, 0);
        m_realCurrentText = m_lyricUtil->getLineAt(index);
        m_itemOffset = -1;
        this->scrollTo(index);
    }

    slot_timerWork();
}
