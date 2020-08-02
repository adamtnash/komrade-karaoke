#include "playbackmanager.h"
#include <QMutexLocker>
#include <QDebug>
#include "RtAudio.h"

PlaybackManager::PlaybackManager(QObject* parent) :
    QObject(parent),
    m_activeFrame(0),
    m_outChannels(2),
    m_mainOuts({0, 1}),
    m_auxOuts({2, 3}),
    m_bufferSize(256),
    m_audio(new RtAudio()),
    m_volume(1.0),
    m_fadeOutFrames(0),
    m_currFadeOut(0),
    m_fadeOutStarted(false),
    m_fadeInFrames(0),
    m_currFadeIn(0),
    m_deviceCacheDirty(true)
{

}

PlaybackManager::~PlaybackManager()
{
    close();
    delete m_audio;
}

QStringList PlaybackManager::pollDevices()
{
    m_deviceCacheDirty = true;
    return getDevices();
}

QStringList PlaybackManager::getDevices()
{
    if (m_deviceCacheDirty) {
        m_deviceCache.clear();
        for (uint i = 0; i < m_audio->getDeviceCount(); i++) {
            auto info = m_audio->getDeviceInfo(i);
            if (info.probed) {
                m_deviceCache.append(info);
            }
        }
        m_deviceCacheDirty = false;
    }
    QStringList devices;
    for (auto device : m_deviceCache) {
        devices.append(device.name.c_str());
    }
    return devices;
}

QString PlaybackManager::currentDevice() const
{
    return m_currentDevice;
}

int playbackCallback(void *outputBuffer, void *inputBuffer, unsigned int nFrames, double streamTime, RtAudioStreamStatus status, void *userData);
void playbackErrorCallback(RtAudioError::Type type, const std::string &errorText);

bool PlaybackManager::openDevice(const QString &deviceName)
{
    close();
    m_currentDevice = QString();

    int idx = getDevices().indexOf(deviceName);
    if (idx < 0) {
        reportError(QString("Requested audio device is not available: %1").arg(deviceName));
        return false;
    }
    RtAudio::DeviceInfo info = m_deviceCache.at(idx);

    RtAudio::StreamParameters parameters;
    parameters.deviceId = idx;
    m_outChannels = info.outputChannels;
    m_mainOuts = { 0, 1 % m_outChannels};
    if (m_outChannels > 2) {
        m_auxOuts = { 2 % m_outChannels, 3 % m_outChannels};
    }
    else {
        m_auxOuts = { -1, -1};
    }
    parameters.nChannels = m_outChannels;
    unsigned int sampleRate = 44100;
    unsigned int bufferFrames = m_bufferSize;
    RtAudio::StreamOptions options;
    options.flags = RTAUDIO_NONINTERLEAVED;
    options.flags |= RTAUDIO_SCHEDULE_REALTIME;

    try {
        m_audio->openStream( &parameters, NULL, RTAUDIO_FLOAT32,
                        sampleRate, &bufferFrames, &playbackCallback, this, &options );
        m_currentDevice = deviceName;
        emit opened();
        return true;
    }
    catch ( RtAudioError& e ) {
        reportError(QString("Error opening audio stream: %1").arg(e.getMessage().c_str()));
        return false;
    }
}

void PlaybackManager::close()
{
    if (m_audio->isStreamOpen()) {
        abort();
        m_audio->closeStream();
        emit closed();
    }
}

void PlaybackManager::start(int fadeInFrames)
{
    QMutexLocker lock(&m_mutex);
    if (m_audio->isStreamOpen() && !m_audio->isStreamRunning()) {
        m_fadeInFrames = fadeInFrames;
        m_currFadeIn = 0;
        try {
            m_audio->startStream();
            emit started();
        }
        catch ( RtAudioError& e ) {
            reportError(QString("Error starting audio stream: %1").arg(e.getMessage().c_str()));
        }
    }
}

void PlaybackManager::stop(int fadeOutFrames)
{
    QMutexLocker lock(&m_mutex);
    if (m_audio->isStreamOpen() && m_audio->isStreamRunning()) {
        if (!m_fadeOutStarted) {
            m_fadeOutStarted = true;
            m_fadeOutFrames = fadeOutFrames;
            m_currFadeOut = 0;

            if (fadeOutFrames == 0) {
                m_fadeOutStarted = false;
                m_audio->stopStream();
                emit stopped();
            }
        }
    }
}

void PlaybackManager::abort()
{
    if (m_audio->isStreamOpen() && m_audio->isStreamRunning()) {
        m_audio->abortStream();
        emit stopped();
    }
}

bool PlaybackManager::isRunning()
{
    return m_audio->isStreamRunning();
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

    if (!m_queuedTrack.isNull() && m_queuedTrack->autoPlay()) {
        start();
    }

    emit queueChanged();
}

void PlaybackManager::clearActiveTrack()
{
    QMutexLocker lock(&m_mutex);
    m_activeTrack = QSharedPointer<TrackData>();
    m_activeAuxTrack = QSharedPointer<TrackData>();
    m_activeFrame = 0;
    emit queueChanged();
}

QSharedPointer<TrackData> PlaybackManager::activeTrack() const
{
    return m_activeTrack;
}

int PlaybackManager::activeFrame() const
{
    return m_activeFrame;
}

void PlaybackManager::checkQueue()
{
    m_activeTrack = QSharedPointer<TrackData>();
    m_activeAuxTrack = QSharedPointer<TrackData>();
    if (!m_queuedTrack.isNull()) {
        m_activeTrack = m_queuedTrack;
        m_activeAuxTrack = m_queuedAuxTrack;
        m_activeFrame = 0;
        m_queuedTrack = QSharedPointer<TrackData>();
        m_queuedAuxTrack = QSharedPointer<TrackData>();

        emit trackStarted(m_activeTrack->fileName());
    }
    emit queueChanged();
}

QPair<int, int> PlaybackManager::getAuxOuts() const
{
    return m_auxOuts;
}

void PlaybackManager::setAuxOuts(const QPair<int, int> &auxOuts)
{
    m_auxOuts = auxOuts;
}

QPair<int, int> PlaybackManager::getMainOuts() const
{
    return m_mainOuts;
}

void PlaybackManager::setMainOuts(const QPair<int, int> &mainOuts)
{
    m_mainOuts = mainOuts;
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
    return !track.isNull() && !track->buffer().isNull() && track->frameCount() >= m_bufferSize;
}

void PlaybackManager::setVolume(float volume)
{
    m_volume = volume;
}

int PlaybackManager::writeNextAudioData(void *outputBuffer, unsigned int nFrames)
{
    QMutexLocker lock(&m_mutex);
    memset(outputBuffer, 0.0f, sizeof(float) * m_outChannels);
    float* out = static_cast<float*>(outputBuffer);

    if (m_activeTrack.isNull()) {
        checkQueue();
    }

    if (!trackOk(m_activeTrack)) {
        return endPlayback();
    }

    bool auxPlayback = m_auxOuts.first >= 0 &&  trackOk(m_activeAuxTrack);

    for (unsigned int i = 0; i < nFrames; i++) {
        // handle track completion
        if (m_activeFrame >= m_activeTrack->frameCount()) {
            m_activeFrame = 0;
            checkQueue();
            if (!trackOk(m_activeTrack)) {
                return endPlayback();
            }
            else if (!m_activeTrack.isNull() && m_activeTrack->autoStop()) {
                return endPlayback();
            }
            auxPlayback = m_auxOuts.first >= 0 && trackOk(m_activeAuxTrack);
        }

        // Actual frame might be affected by fade out
        int frame = m_activeFrame;

        // get volume multiplier
        float volume = m_volume;
        if (m_currFadeIn < m_fadeInFrames) {
            m_currFadeIn++;
            volume *= float(m_currFadeIn * m_currFadeIn) / float(m_fadeInFrames * m_fadeInFrames);
        }
        if (m_fadeOutStarted) {
            frame += m_currFadeOut;
            if (frame >= m_activeTrack->frameCount()) {
                return endPlayback();
            }
            if (m_currFadeOut < m_fadeOutFrames) {
                m_currFadeOut++;
                float reduction = float(m_currFadeOut * m_currFadeOut) / float(m_fadeOutFrames * m_fadeOutFrames);
                volume *= (1.0f - reduction);
            }
            else {
                return endPlayback();
            }
        }

        out[m_mainOuts.first*nFrames + i] = m_activeTrack->channelSamples(0).at(frame) * volume;
        out[m_mainOuts.second*nFrames + i] = m_activeTrack->channelSamples(1).at(frame) * volume;

        // Aux output
        if (auxPlayback) {
            int auxFrame = frame % m_activeAuxTrack->frameCount();
            out[m_auxOuts.first*nFrames + i] = m_activeAuxTrack->channelSamples(0).at(auxFrame) * volume;
            out[m_auxOuts.second*nFrames + i] = m_activeAuxTrack->channelSamples(1).at(auxFrame) * volume;
        }

        if (!m_fadeOutStarted) {
            m_activeFrame++;
        }
    }

    return 0;
}

int PlaybackManager::endPlayback()
{
    m_fadeOutStarted = false;
    m_fadeOutFrames = 0;
    m_currFadeOut = 0;
    emit stopped();
    return 1;
}

int playbackCallback(void *outputBuffer, void *inputBuffer, unsigned int nFrames, double streamTime, RtAudioStreamStatus status, void *userData)
{
    Q_UNUSED(inputBuffer)
    Q_UNUSED(streamTime)
    Q_UNUSED(status)

    PlaybackManager* playbackManager = static_cast<PlaybackManager*>(userData);
    return playbackManager->writeNextAudioData(outputBuffer, nFrames);
}

void playbackErrorCallback(RtAudioError::Type type, const std::string &errorText)
{
    Q_UNUSED(type)
    qDebug() << "\nError with RtAudio playback" << errorText.c_str() << '\n';
}
