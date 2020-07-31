#ifndef PLAYBACKMANAGER_H
#define PLAYBACKMANAGER_H

#include <QObject>
#include "trackdata.h"
#include <QMutex>
#include "RtAudio.h"
#include <QHash>
#include <QAtomicInt>

class PlaybackManager : public QObject
{
    Q_OBJECT
public:
    PlaybackManager(QObject* parent = nullptr);
    ~PlaybackManager();

    QStringList pollDevices();
    QStringList getDevices();
    QString currentDevice() const;
    bool openDevice(const QString &deviceName);

    void close();
    void start(int fadeInSamples = 100);
    void stop(int fadeOutSamples = 1000);
    void abort();

    bool isRunning();

    int writeNextAudioData(void *outputBuffer, unsigned int nFrames);

    QSharedPointer<TrackData> queuedTrack() const;
    void setQueuedTrack(QSharedPointer<TrackData> queuedTrack, QSharedPointer<TrackData> auxTrack = QSharedPointer<TrackData>());
    void clearActiveTrack();

    QSharedPointer<TrackData> activeTrack() const;
    int activeSample() const;

    int outChannels() const;
    void setOutChannels(int outChannels);

    bool trackOk(QSharedPointer<TrackData> track);

signals:
    void opened();
    void closed();
    void started();
    void stopped();
    void queueChanged();
    void trackStarted(QString);

    void reportError(QString);

public slots:
    void setVolume(float volume);
    int endPlayback();

private:
    void checkQueue();

    QSharedPointer<TrackData> m_queuedTrack;
    QSharedPointer<TrackData> m_queuedAuxTrack;

    QSharedPointer<TrackData> m_activeTrack;
    QSharedPointer<TrackData> m_activeAuxTrack;
    int m_activeSample;
    int m_outChannels;
    QMutex m_mutex;

    RtAudio* m_audio;
    QString m_currentDevice;

    float m_volume;

    QAtomicInt m_fadeOutSamples;
    QAtomicInt m_currFadeOut;
    QAtomicInt m_fadeOutStarted;

    QAtomicInt m_fadeInSamples;
    QAtomicInt m_currFadeIn;

    bool m_deviceCacheDirty;
    QList<RtAudio::DeviceInfo> m_deviceCache;
};

#endif // PLAYBACKMANAGER_H
