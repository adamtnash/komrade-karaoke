#ifndef AUDIOFILEBUFFER_H
#define AUDIOFILEBUFFER_H

#include <QByteArray>
#include <QSharedPointer>
#include <QVector>

class AudioFileBuffer
{
public:
    static QSharedPointer<AudioFileBuffer> fromWavFile(QString fileName);
    ~AudioFileBuffer();

    QVector<float> floatData() const;
    quint32 numChannels() const;
    quint32 bytesPerSample() const;
    quint32 sampleRate() const;

private:
    AudioFileBuffer();
    QByteArray m_data;
    QVector<float> m_floatData;
    quint32 m_numChannels;
    quint32 m_bytesPerSample;
    quint32 m_sampleRate;
};

#endif // AUDIOFILEBUFFER_H
