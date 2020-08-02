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
    void start(int fadeInFrames = 100);
    void stop(int fadeOutFrames = 1000);
    void abort();

    bool isRunning();

    int writeNextAudioData(void *outputBuffer, unsigned int nFrames);

    QSharedPointer<TrackData> queuedTrack() const;
    void setQueuedTrack(QSharedPointer<TrackData> queuedTrack, QSharedPointer<TrackData> auxTrack = QSharedPointer<TrackData>());
    void clearActiveTrack();

    QSharedPointer<TrackData> activeTrack() const;
    int activeFrame() const;

    int outChannels() const;
    void setOutChannels(int outChannels);

    bool trackOk(QSharedPointer<TrackData> track);

    QPair<int, int> getMainOuts() const;
    void setMainOuts(const QPair<int, int> &mainOuts);

    QPair<int, int> getAuxOuts() const;
    void setAuxOuts(const QPair<int, int> &auxOuts);

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
    int m_activeFrame;
    int m_outChannels;
    QPair<int, int> m_mainOuts;
    QPair<int, int> m_auxOuts;
    int m_bufferSize;

    QMutex m_mutex;

    RtAudio* m_audio;
    QString m_currentDevice;

    float m_volume;

    int m_fadeOutFrames;
    int m_currFadeOut;
    bool m_fadeOutStarted;

    int m_fadeInFrames;
    int m_currFadeIn;

    bool m_deviceCacheDirty;
    QList<RtAudio::DeviceInfo> m_deviceCache;
};

#endif // PLAYBACKMANAGER_H
