#include "musicplayedlistwidget.h"
#include "musicfunctionuiobject.h"
#include "musicsongsplayedlistwidget.h"
#include "musicsettingmanager.h"
#include "musictinyuiobject.h"
#include "musicuiobject.h"
#include "musicapplication.h"

#include <QLabel>
#include <QBoxLayout>
#include <QPushButton>
#include <QToolButton>

#define MAX_SIZE    3

MusicPlayedListWidget *MusicPlayedListWidget::m_instance = nullptr;

MusicPlayedListWidget::MusicPlayedListWidget(QWidget *parent)
    : MusicToolMenuWidget(parent)
{
    m_instance = this;
    setToolTip(tr("playedList"));
    setStyleSheet(MusicUIObject::MKGBtnPlayedList);

    disconnect(this, SIGNAL(clicked()), this, SLOT(popupMenu()));
    connect(this, SIGNAL(clicked()), SLOT(popupMenu()));

    initWidget();
}

MusicPlayedListWidget::~MusicPlayedListWidget()
{
    delete m_playedListWidget;
    while(!m_labels.isEmpty())
    {
        delete m_labels.takeLast();
    }
}

QString MusicPlayedListWidget::getClassName()
{
    return staticMetaObject.className();
}

MusicPlayedListWidget *MusicPlayedListWidget::instance()
{
    return m_instance;
}

void MusicPlayedListWidget::clear()
{
    m_songLists.clear();
    m_playedListWidget->clear();
    setPlayListCount(0);
}

void MusicPlayedListWidget::resetToolIndex(const QList< std::pair<int, int> > &indexs)
{
    for(int i=indexs.count()-1; i>=0; --i)
    {
        std::pair<int, int> index = indexs[i];
        for(int s=0; s<m_songLists.count(); ++s)
        {
            MusicPlayedSong *song = &m_songLists[s];
            if(song->m_toolIndex == index.first)
            {
                song->m_toolIndex = index.second;
            }
        }
    }
}

QStringList MusicPlayedListWidget::getPlayedList() const
{
    QStringList lists;
    foreach(const MusicPlayedSong &song, m_songLists)
    {
        lists << song.m_song.getMusicPath();
    }
    return lists;
}

void MusicPlayedListWidget::remove(int toolIndex, const MusicSong &song)
{
    MusicObject::MIntSet deletedRow;
    for(int i=0; i<m_songLists.count(); ++i)
    {
        MusicPlayedSong playedSong = m_songLists[i];
        if(playedSong.m_toolIndex == toolIndex && playedSong.m_song == song)
        {
            deletedRow << i;
        }
    }
    MusicObject::MIntList deleteList = deletedRow.toList();
    qSort(deleteList);
    for(int i=deleteList.count()-1; i>=0; --i)
    {
        int index = deleteList[i];
        m_songLists.removeAt(index);
        m_playedListWidget->removeRow(index);
    }

    updateSongsFileName();
}

void MusicPlayedListWidget::remove(int toolIndex, const MusicSongs &songs)
{
    MusicObject::MIntSet deletedRow;
    for(int i=0; i<m_songLists.count(); ++i)
    {
        MusicPlayedSong playedSong = m_songLists[i];
        if(playedSong.m_toolIndex == toolIndex)
        {
            foreach(const MusicSong &song, songs)
            {
                if(playedSong.m_song == song)
                {
                    deletedRow << i;
                }
            }
        }
    }
    MusicObject::MIntList deleteList = deletedRow.toList();
    qSort(deleteList);
    for(int i=deleteList.count() - 1; i>=0; --i)
    {
        int index = deleteList[i];
        m_songLists.removeAt(index);
        m_playedListWidget->removeRow(index);
    }

    updateSongsFileName();
}

void MusicPlayedListWidget::append(int toolIndex, const MusicSong &song)
{
    m_songLists << MusicPlayedSong{toolIndex, song};
    updateSongsFileName();
}

void MusicPlayedListWidget::append(int toolIndex, const MusicSongs &songs)
{
    foreach(const MusicSong &song, songs)
    {
        m_songLists << MusicPlayedSong{toolIndex, song};
    }
    updateSongsFileName();
}

void MusicPlayedListWidget::insert(int toolIndex, const MusicSong &song)
{
    insert(toolIndex, m_playedListWidget->getPlayRowIndex() + 1, song);
}

void MusicPlayedListWidget::insert(int toolIndex, int index, const MusicSong &song)
{
    if(index < 0 || index >= m_songLists.count())
    {
        return;
    }

    m_songLists.insert(index, MusicPlayedSong{toolIndex, song});
    m_playedListWidget->clear();
    updateSongsFileName();
}

void MusicPlayedListWidget::setCurrentIndex(const QString &path)
{
    int index = -1;
    for(int i=0; i<m_songLists.count(); ++i)
    {
        if(m_songLists[i].m_song.getMusicPath() == path)
        {
            index = i;
            break;
        }
    }
    m_playedListWidget->selectRow(index);
}

void MusicPlayedListWidget::resizeWindow()
{
//    int h = M_SETTING_PTR->value(MusicSettingManager::WidgetSize).toSize().height();
//    m_containWidget->setFixedSize(300, 500 + h - WINDOW_HEIGHT_MIN);
}

void MusicPlayedListWidget::popupMenu()
{
    QPoint pos = mapToGlobal(QPoint(0, 0));
    pos.setY(pos.y() - m_containWidget->height() - 10);
    pos.setX(pos.x() - (m_containWidget->width() - width() - 3));

    m_playedListWidget->selectPlayedRow();
    m_menu->exec(pos);
}

void MusicPlayedListWidget::setDeleteItemAt()
{
    setPlayListCount(m_songLists.count());
}

void MusicPlayedListWidget::setDeleteItemAll()
{
    if(m_songLists.isEmpty())
    {
        return;
    }

    int count = m_playedListWidget->rowCount();
    for(int i=0; i<count; ++i)
    {
        m_playedListWidget->removeRow(0);
    }
    m_songLists.clear();
    setPlayListCount(0);
    MusicApplication::instance()->musicPlayIndex(-1);
}

void MusicPlayedListWidget::initWidget()
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);

    layout->addStretch(2);
    for(int i=0; i<MAX_SIZE; ++i)
    {
        QLabel *label = new QLabel(this);
        label->setFixedWidth(9);
        label->setPixmap(QPixmap(":/tiny/lb_number0"));
        layout->addWidget(label);
        m_labels << label;
    }
    layout->addStretch(1);

    setLayout(layout);

    m_containWidget->setFixedSize(320, 400);
    QHBoxLayout *containLayout = new QHBoxLayout(m_containWidget);
    containLayout->setContentsMargins(0, 0, 0, 0);
    containLayout->setSpacing(0);
    containLayout->addWidget( createContainerWidget() );
    m_containWidget->setLayout(containLayout);
}

QWidget *MusicPlayedListWidget::createContainerWidget()
{
    QWidget *containWidget = new QWidget(this);
    containWidget->setObjectName("containWidget");
    containWidget->setStyleSheet(QString("#%1{%2}").arg(containWidget->objectName())
                                 .arg(MusicUIObject::MBackgroundStyle17));
    QVBoxLayout *layout = new QVBoxLayout(containWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QWidget *topWidget = new QWidget(this);
    topWidget->setFixedHeight(35);
    topWidget->setStyleSheet(MusicUIObject::MBackgroundStyle20);

    QHBoxLayout *topWidgetLayout = new QHBoxLayout(topWidget);
    topWidgetLayout->setSpacing(15);
    QLabel *label = new QLabel(tr("playedList"), topWidget);
    label->setStyleSheet(MusicUIObject::MColorStyle02 + MusicUIObject::MFontStyle01 +
                         MusicUIObject::MFontStyle03);

    QPushButton *shareButton = new QPushButton(this);
    shareButton->setFixedSize(16, 16);
    shareButton->setToolTip(tr("shareList"));
    shareButton->setCursor(QCursor(Qt::PointingHandCursor));
    shareButton->setStyleSheet(MusicUIObject::MKGTinyBtnShare);

    QPushButton *deleteButton = new QPushButton(this);
    deleteButton->setFixedSize(16, 16);
    deleteButton->setToolTip(tr("clearList"));
    deleteButton->setCursor(QCursor(Qt::PointingHandCursor));
    deleteButton->setStyleSheet(MusicUIObject::MKGTinyBtnDelete);
    connect(deleteButton, SIGNAL(clicked()), SLOT(setDeleteItemAll()));

    QToolButton *closeButton = new QToolButton(this);
    closeButton->setFixedSize(16, 16);
    closeButton->setToolTip(tr("closeList"));
    closeButton->setCursor(QCursor(Qt::PointingHandCursor));
    closeButton->setStyleSheet(MusicUIObject::MKGTinyBtnClose);
    connect(closeButton, SIGNAL(clicked()), m_menu, SLOT(close()));

    topWidgetLayout->addWidget(label);
    topWidgetLayout->addStretch(1);
    topWidgetLayout->addWidget(shareButton);
    topWidgetLayout->addWidget(deleteButton);
    topWidgetLayout->addWidget(closeButton);
    topWidget->setLayout(topWidgetLayout);

    m_playedListWidget = new MusicSongsPlayedListWidget(this);
    m_playedListWidget->setSongsFileName(&m_songLists);
    connect(m_playedListWidget, SIGNAL(updateCountLabel()), SLOT(setDeleteItemAt()));

    layout->addWidget(topWidget);
    layout->addWidget(m_playedListWidget);

    containWidget->setLayout(layout);
    return containWidget;
}

void MusicPlayedListWidget::updateSongsFileName()
{
    setPlayListCount(m_songLists.count());
    m_playedListWidget->updateSongsFileName(m_songLists);
}

void MusicPlayedListWidget::setPlayListCount(int count)
{
    for(int i=MAX_SIZE-1; i>=0; --i)
    {
        m_labels[i]->setPixmap(QPixmap(QString(":/tiny/lb_number%1").arg(count%10)));
        count = count/10;
    }

    if(count > 1000)
    {
        for(int i=MAX_SIZE-1; i>=0; --i)
        {
            m_labels[i]->setPixmap(QPixmap(QString(":/tiny/lb_number%1").arg(9)));
        }
    }
}
