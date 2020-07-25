#include "playbackmanager.h"
#include <QMutexLocker>

PlaybackManager::PlaybackManager(QObject* parent) :
    QObject(parent),
    m_activeSample(0)
{

}

QSharedPointer<TrackData> PlaybackManager::queuedTrack() const
{
    return m_queuedTrack;
}

void PlaybackManager::setQueuedTrack(const QSharedPointer<TrackData> &queuedTrack)
{
    m_mutex.lock();
    m_queuedTrack = queuedTrack;
    m_mutex.unlock();

    emit playbackChanged();
}

QSharedPointer<TrackData> PlaybackManager::activeTrack() const
{
    return m_activeTrack;
}

int PlaybackManager::activeSample() const
{
    return m_activeSample;
}


void PlaybackManager::checkQueue()
{
    if (!m_queuedTrack.isNull()) {
        m_activeTrack = m_queuedTrack;
        m_activeSample = 0;
        m_queuedTrack = QSharedPointer<TrackData>();
    }
}

int PlaybackManager::writeNextAudioData(void *outputBuffer, unsigned int nFrames)
{
    QMutexLocker lock(&m_mutex);
    float* out = static_cast<float*>(outputBuffer);

    if (m_activeSample == 0) {
        checkQueue();
    }

    if (m_activeTrack.isNull()) {
        return 1;
    }

    if (!m_activeTrack->buffer() || m_activeTrack->buffer()->floatData().size() < 2) {
        return 1;
    }

    for (unsigned int i = 0; i < nFrames; i++) {
        if (m_activeSample + 1 > m_activeTrack->buffer()->floatData().size()) {
            m_activeSample = 0;
            checkQueue();
        }
        out[i*2] = m_activeTrack->buffer()->floatData().at(m_activeSample);
        if (m_activeTrack->buffer()->numChannels() < 2) {
            out[i*2+1] = out[i*2];
        }
        else {
            out[i*2+1] = m_activeTrack->buffer()->floatData().at(m_activeSample+1);
        }
        m_activeSample += m_activeTrack->buffer()->numChannels();
    }

    return 0;
}
