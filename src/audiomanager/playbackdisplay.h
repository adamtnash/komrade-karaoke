#ifndef PLAYBACKDISPLAY_H
#define PLAYBACKDISPLAY_H

#include <QWidget>
#include "playbackmanager.h"
#include <QTimer>

class PlaybackDisplay : public QWidget
{
    Q_OBJECT
public:
    explicit PlaybackDisplay(PlaybackManager* playbackManager, QWidget *parent = nullptr);

    void paintEvent(QPaintEvent*) override;

signals:

private:
    void drawTrack(QPainter *painter, QRect rect, QSharedPointer<TrackData> track);
    void drawActiveTrackOverlay(QPainter *painter, QRect rect);
    void drawNotification(QPainter *painter, QRect rect, QString text);

    PlaybackManager* m_playbackManager;
    QTimer m_animationTimer;
};

#endif // PLAYBACKDISPLAY_H
