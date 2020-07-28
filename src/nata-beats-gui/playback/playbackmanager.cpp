#include "playbackmanager.h"
#include <QMutexLocker>

PlaybackManager::PlaybackManager(QObject* parent) :
    QObject(parent),
    m_activeSample(0),
    m_outChannels(2)
{

}

QSharedPointer<TrackData> PlaybackManager::queuedTrack() const
{
    return m_queuedTrack;
}

void PlaybackManager::setQueuedTrack(QSharedPointer<TrackData> queuedTrack, QSharedPointer<TrackData> auxTrack)
{
    m_mutex.lock();
    m_queuedTrack = queuedTrack;
    m_queuedAuxTrack = auxTrack;
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
    m_activeTrack = QSharedPointer<TrackData>();
    m_activeAuxTrack = QSharedPointer<TrackData>();
    if (!m_queuedTrack.isNull()) {
        m_activeTrack = m_queuedTrack;
        m_activeAuxTrack = m_queuedAuxTrack;
        m_activeSample = 0;
        m_queuedTrack = QSharedPointer<TrackData>();
        m_queuedAuxTrack = QSharedPointer<TrackData>();

        emit trackStarted(m_activeTrack->fileName());
    }
}

int PlaybackManager::outChannels() const
{
    return m_outChannels;
}

void PlaybackManager::setOutChannels(int outChannels)
{
    m_outChannels = outChannels;
}

bool PlaybackManager::trackOk(QSharedPointer<TrackData> track)
{
    return !track.isNull() && !track->buffer().isNull() && track->buffer()->floatData().size() > 512;
}

int PlaybackManager::writeNextAudioData(void *outputBuffer, unsigned int nFrames)
{
    QMutexLocker lock(&m_mutex);
    float* out = static_cast<float*>(outputBuffer);

    if (m_activeTrack.isNull()) {
        checkQueue();
    }

    if (!trackOk(m_activeTrack)) {
        emit playbackStopped();
        return 1;
    }

    bool auxPlayback = m_outChannels == 4 &&  trackOk(m_activeAuxTrack);

    for (unsigned int i = 0; i < nFrames; i++) {
        if (m_activeSample + 1 > m_activeTrack->buffer()->floatData().size()) {
            m_activeSample = 0;
            checkQueue();
            if (!trackOk(m_activeTrack)) {
                emit playbackStopped();
                return 1;
            }
            auxPlayback = m_outChannels == 4 && trackOk(m_activeAuxTrack);
        }

        out[i*m_outChannels] = m_activeTrack->buffer()->floatData().at(m_activeSample);
        if (m_activeTrack->buffer()->numChannels() < 2) {
            out[i*m_outChannels + 1] = out[i*m_outChannels];
        }
        else {
            out[i*m_outChannels + 1] = m_activeTrack->buffer()->floatData().at(m_activeSample+1);
        }

        // Aux output
        if (auxPlayback) {
            int auxSample = m_activeSample / m_activeTrack->buffer()->numChannels();
            auxSample *= m_activeAuxTrack->buffer()->numChannels();
            auxSample %= m_activeAuxTrack->buffer()->floatData().size();
            out[i*m_outChannels + 2] = m_activeAuxTrack->buffer()->floatData().at(auxSample);
            if (m_activeAuxTrack->buffer()->numChannels() < 2) {
                out[i*m_outChannels + 3] = out[i * m_outChannels + 2];
            }
            else {
                out[i*m_outChannels + 3] = m_activeAuxTrack->buffer()->floatData().at(auxSample + 1);
            }
        }

        m_activeSample += m_activeTrack->buffer()->numChannels();
    }

    return 0;
}
