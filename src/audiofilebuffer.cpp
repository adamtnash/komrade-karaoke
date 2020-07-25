#include "audiofilebuffer.h"

#include <QFile>

AudioFileBuffer::AudioFileBuffer()
{

}

AudioFileBuffer::~AudioFileBuffer()
{

}

quint32 AudioFileBuffer::sampleRate() const
{
    return m_sampleRate;
}

quint32 AudioFileBuffer::bytesPerSample() const
{
    return m_bytesPerSample;
}

quint32 AudioFileBuffer::numChannels() const
{
    return m_numChannels;
}

QVector<float> AudioFileBuffer::floatData() const
{
    return m_floatData;
}

QSharedPointer<AudioFileBuffer> AudioFileBuffer::fromWavFile(QString fileName)
{
    QFile wavFile(fileName);
    if (!wavFile.open(QFile::ReadOnly)) {
        return QSharedPointer<AudioFileBuffer>();
    }

    auto audio = QSharedPointer<AudioFileBuffer>(new AudioFileBuffer());

    // Check RIFF Header
    QByteArray riff = wavFile.read(4);
    if (riff != "RIFF") {
        return QSharedPointer<AudioFileBuffer>();
    }
    quint32 dataSize = *((quint32*)wavFile.read(4).data());
    if (dataSize + 8 != wavFile.size()) {
        return QSharedPointer<AudioFileBuffer>();
    }
    QByteArray wav = wavFile.read(4);
    if (wav != "WAVE") {
        return QSharedPointer<AudioFileBuffer>();
    }

    // Parse chunks
    while (wavFile.bytesAvailable() >= 8) {
        QByteArray ckId = wavFile.read(4);
        quint32 ckSize = *((quint32*)wavFile.read(4).data());
        if (ckSize > wavFile.bytesAvailable()) {
            break;
        }
        if (ckId == "data") {
            audio->m_data = wavFile.read(ckSize);
            if (ckSize % 2) {
                wavFile.read(1);
            }
        }
        else if (ckId == "fmt ") {
            QByteArray fmt = wavFile.read(ckSize);
            QByteArray fmtCode = fmt.mid(0, 2);
            if (fmtCode != QByteArray::fromHex("0100")) {
                return QSharedPointer<AudioFileBuffer>();
            }
            audio->m_numChannels = *((quint16*)fmt.mid(2, 2).data());
            audio->m_sampleRate = *((quint32*)fmt.mid(4, 4).data());
            audio->m_bytesPerSample = *((quint16*)fmt.mid(12, 2).data()) / audio->m_numChannels;
            if (audio->m_bytesPerSample < 1 || audio->bytesPerSample() > 4) {
                return QSharedPointer<AudioFileBuffer>();
            }
        }
        else {
            wavFile.read(ckSize);
        }
    }

    // Parse PCM data
    if (audio->m_bytesPerSample == 1) {
        qint8* rawData = (qint8*)audio->m_data.data();
        for (int i = 0; i < audio->m_data.size(); i++) {
            audio->m_floatData.push_back(float(rawData[i]) / 128.0f);
        }
    }
    else if (audio->m_bytesPerSample == 2) {
        qint16* rawData = (qint16*)audio->m_data.data();
        for (int i = 0; i < audio->m_data.size()/2; i++) {
            audio->m_floatData.push_back(float(rawData[i]) / 32768.0f);
        }
    }
    else if (audio->m_bytesPerSample == 3) {
        quint8* rawData = (quint8*)audio->m_data.data();
        for (int i = 0; i+2 < audio->m_data.size(); i+=3) {
            qint32 val = rawData[i] + (rawData[i+1] << 8) + (rawData[i+2] << 16);
            val = val - 2*(0x800000 & val);
            audio->m_floatData.push_back(double(val) / 8388608.0);
        }
    }
    else if (audio->m_bytesPerSample == 4) {
        qint32* rawData = (qint32*)audio->m_data.data();
        for (int i = 0; i < audio->m_data.size()/4; i++) {
            audio->m_floatData.push_back(float(double(rawData[i]) / 2147483648.0));
        }
    }


    return audio;
}
