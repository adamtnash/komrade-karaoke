#ifndef PLAYBACKMANAGER_H
#define PLAYBACKMANAGER_H

#include <QObject>
#include "trackdata.h"
#include <QMutex>
#include "RtAudio.h"
#include <QHash>

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
    void start();
    void stop();
    void abort();

    bool isRunning();

    int writeNextAudioData(void *outputBuffer, unsigned int nFrames);

    QSharedPointer<TrackData> queuedTrack() const;
    void setQueuedTrack(QSharedPointer<TrackData> queuedTrack, QSharedPointer<TrackData> auxTrack = QSharedPointer<TrackData>());

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

    bool m_deviceCacheDirty;
    QList<RtAudio::DeviceInfo> m_deviceCache;
};

#endif // PLAYBACKMANAGER_H
