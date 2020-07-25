#include "playbackdisplay.h"
#include <QPainter>
#include <QFont>

PlaybackDisplay::PlaybackDisplay(PlaybackManager* playbackManager, QWidget *parent) :
    QWidget(parent),
    m_playbackManager(playbackManager)
{
    connect(m_playbackManager, SIGNAL(playbackChanged()), this, SLOT(repaint()));

    connect(&m_animationTimer, SIGNAL(timeout()), this, SLOT(repaint()));
    m_animationTimer.setTimerType(Qt::TimerType::CoarseTimer);
    m_animationTimer.setInterval(250);
    m_animationTimer.start(250);

    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
}

void PlaybackDisplay::drawTrack(QPainter *painter, QRect rect, QSharedPointer<TrackData> track)
{
    painter->save();
    painter->drawImage(rect, track->waveformPreview().toImage(), track->waveformPreview().rect());

    painter->setCompositionMode(QPainter::RasterOp_SourceXorDestination);
    painter->setPen(QColor(0xff, 0xff, 0xff));
    QRect filenameRect(rect);
    filenameRect.translate(10, 5);
    painter->drawText(filenameRect, track->fileName());

//    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
//    painter->setPen(QPen(QColor(0xff, 0xff, 0xff, 0x55), 1));
//    int beatsInTrack = int(double(track->sampleCount()) / track->samplesPerBeat());
//    int xPerBeat = rect.width()/beatsInTrack;
//    for (int x = xPerBeat; x < rect.width(); x += xPerBeat) {
//        painter->drawLine(rect.x() + x, rect.y(), rect.x() + x, rect.bottom());
//    }

    painter->restore();
}

void PlaybackDisplay::drawActiveTrackOverlay(QPainter *painter, QRect rect)
{
    painter->save();

    auto track = m_playbackManager->activeTrack();

    double percent = double(m_playbackManager->activeSample()) / double(track->sampleCount());
    int x = rect.x() + percent*rect.width();
    painter->setCompositionMode(QPainter::RasterOp_SourceXorDestination);
    painter->setPen(QColor(0xff, 0xff, 0xff));
    painter->drawLine(x, 0, x, rect.height());

    painter->restore();
}

void PlaybackDisplay::drawNotification(QPainter *painter, QRect rect, QString text)
{
    painter->save();
    QFont bigText;
    bigText.setBold(true);
    bigText.setPointSize(16);
    painter->setFont(bigText);
    painter->setPen(QColor(0x55, 0x55, 0x55));
    painter->drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter, text);
    painter->restore();
}

void PlaybackDisplay::paintEvent(QPaintEvent*) {
    QPainter painter(this);

    painter.fillRect(0, 0, this->width(), this->height(), QColor(0xdd, 0xdd, 0xdd));

    QRect activeTrackRect(0, 0, this->width()/2, this->height());
    if (!m_playbackManager->activeTrack().isNull()) {
        drawTrack(&painter, activeTrackRect, m_playbackManager->activeTrack());
        drawActiveTrackOverlay(&painter, activeTrackRect);
    }
    else {
        drawNotification(&painter, activeTrackRect, "No Active Track");
    }

    QRect queuedTrackRect(this->width()/2, 0, this->width()/2, this->height());
    if (!m_playbackManager->queuedTrack().isNull()) {
        drawTrack(&painter, queuedTrackRect, m_playbackManager->queuedTrack());
    }
    else if (!m_playbackManager->activeTrack().isNull()) {
        drawTrack(&painter, queuedTrackRect, m_playbackManager->activeTrack());
    }
    else {
        drawNotification(&painter, queuedTrackRect, "No Queued Track");
    }
}
