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
    void setQueuedTrack(QSharedPointer<TrackData> queuedTrack, QSharedPointer<TrackData> auxTrack = QSharedPointer<TrackData>());

    QSharedPointer<TrackData> activeTrack() const;
    int activeSample() const;

    int outChannels() const;
    void setOutChannels(int outChannels);

    bool trackOk(QSharedPointer<TrackData> track);

signals:
    void playbackChanged();
    void playbackStopped();
    void trackStarted(QString);

private:
    void checkQueue();

    QSharedPointer<TrackData> m_queuedTrack;
    QSharedPointer<TrackData> m_queuedAuxTrack;

    QSharedPointer<TrackData> m_activeTrack;
    QSharedPointer<TrackData> m_activeAuxTrack;
    int m_activeSample;
    int m_outChannels;
    QMutex m_mutex;
};

#endif // PLAYBACKMANAGER_H
