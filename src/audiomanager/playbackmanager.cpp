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
    m_sampleRate(44100),
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
    return getDeviceNames();
}

QList<RtAudio::DeviceInfo> PlaybackManager::getDevices()
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
    return m_deviceCache;
}

QStringList PlaybackManager::getDeviceNames()
{
    getDevices();
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

bool PlaybackManager::openDevice(QString deviceName)
{
    QString lastDevice = m_currentDevice;
    close();
    m_outChannels = 0;

    int idx = getDeviceNames().indexOf(deviceName);
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

    if (lastDevice != deviceName) {
        m_sampleRate = info.preferredSampleRate;
        m_bufferSize = 256;
    }

    parameters.nChannels = m_outChannels;
    unsigned int sampleRate = m_sampleRate;
    unsigned int plannedBufferSize = m_bufferSize;
    RtAudio::StreamOptions options;
    options.flags = RTAUDIO_NONINTERLEAVED;
    options.flags |= RTAUDIO_SCHEDULE_REALTIME;

    try {
        m_audio->openStream( &parameters, NULL, RTAUDIO_FLOAT32,
                        sampleRate, &m_bufferSize, &playbackCallback, this, &options );
        m_currentDevice = deviceName;
        emit opened();
        if (plannedBufferSize != m_bufferSize) {
            emit bufferResized(m_bufferSize);
        }
        return true;
    }
    catch ( RtAudioError& e ) {
        reportError(QString("Error opening audio stream: %1").arg(e.getMessage().c_str()));
        return false;
    }
}

void PlaybackManager::close()
{
    m_currentDevice = QString();
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

unsigned int PlaybackManager::getSampleRate() const
{
    return m_sampleRate;
}

bool PlaybackManager::setSampleRate(unsigned int sampleRate)
{
    if (m_sampleRate != sampleRate) {
        m_mutex.lock();
        m_sampleRate = sampleRate;
        m_mutex.unlock();
        return openDevice(m_currentDevice);
    }
    return true;
}

QVector<unsigned int> PlaybackManager::getAvailableBufferSizes() const
{
    return {16, 32, 64, 128, 256, 512, 1024, 2048, 4096};
}

QVector<unsigned int> PlaybackManager::getAvailableSampleRates(QString deviceName) const
{
    if (deviceName.isEmpty()) {
        deviceName = m_currentDevice;
    }
    if (deviceName.isEmpty()) {
        return {};
    }
    for (auto device: m_deviceCache) {
        if (deviceName.toStdString() == device.name) {
            return QVector<unsigned int>::fromStdVector(device.sampleRates);
        }
    }
    return {};
}

unsigned int PlaybackManager::getBufferSize() const
{
    return m_bufferSize;
}

bool PlaybackManager::setBufferSize(unsigned int bufferSize)
{
    if (m_bufferSize != bufferSize) {
        m_mutex.lock();
        m_bufferSize = bufferSize;
        m_mutex.unlock();
        return openDevice(m_currentDevice);
    }
    return true;
}

ChannelPair PlaybackManager::getAuxOuts() const
{
    return m_auxOuts;
}

void PlaybackManager::setAuxOuts(const ChannelPair &auxOuts)
{
    QMutexLocker lock(&m_mutex);
    m_auxOuts = auxOuts;
}

ChannelPair PlaybackManager::getMainOuts() const
{
    return m_mainOuts;
}

void PlaybackManager::setMainOuts(const ChannelPair &mainOuts)
{
    QMutexLocker lock(&m_mutex);
    m_mainOuts = mainOuts;
}

int PlaybackManager::outChannels() const
{
    return m_outChannels;
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
    float* out = static_cast<float*>(outputBuffer);
    for (size_t i = 0; i < nFrames * m_outChannels; i++) {
        out[i] = 0.0f;
    }

    if (m_activeTrack.isNull()) {
        checkQueue();
    }

    if (!trackOk(m_activeTrack)) {
        return endPlayback();
    }

    bool mainPlayback = m_mainOuts.first >= 0;
    bool auxPlayback = m_auxOuts.first >= 0 &&  trackOk(m_activeAuxTrack);

    float mainFactor = 1.0f;
    float auxFactor = 1.0f;
    if (m_mainOuts.first == m_mainOuts.second) {
        mainFactor *= 0.5f;
    }
    if (m_auxOuts.first == m_auxOuts.second) {
        auxFactor *= 0.5f;
    }
    if (m_mainOuts.first == m_auxOuts.first) {
        mainFactor *= 0.5f;
        auxFactor *= 0.5f;
    }

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


        if (mainPlayback) {
            out[m_mainOuts.first*nFrames + i] += m_activeTrack->channelSamples(0).at(frame) * volume * mainFactor;
            out[m_mainOuts.second*nFrames + i] += m_activeTrack->channelSamples(1).at(frame) * volume * mainFactor;
        }

        // Aux output
        if (auxPlayback) {
            int auxFrame = frame % m_activeAuxTrack->frameCount();
            out[m_auxOuts.first*nFrames + i] += m_activeAuxTrack->channelSamples(0).at(auxFrame) * volume * auxFactor;
            out[m_auxOuts.second*nFrames + i] += m_activeAuxTrack->channelSamples(1).at(auxFrame) * volume * auxFactor;
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
