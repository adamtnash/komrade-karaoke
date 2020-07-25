#ifndef PLAYBACKMANAGER_H
#define PLAYBACKMANAGER_H

#include <QObject>
#include "trackdata.h"
#include <QMutex>

class PlaybackManager : public QObject
{
    Q_OBJECT
public:
    PlaybackManager(QObject* parent = nullptr);

    int writeNextAudioData(void *outputBuffer, unsigned int nFrames);

    QSharedPointer<TrackData> queuedTrack() const;
    void setQueuedTrack(const QSharedPointer<TrackData> &queuedTrack);

    QSharedPointer<TrackData> activeTrack() const;
    int activeSample() const;

signals:
    void playbackChanged();

private:
    void checkQueue();

    QSharedPointer<TrackData> m_queuedTrack;

    QSharedPointer<TrackData> m_activeTrack;
    int m_activeSample;
    QMutex m_mutex;
};

#endif // PLAYBACKMANAGER_H
