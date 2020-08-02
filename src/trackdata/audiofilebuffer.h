#ifndef AUDIOFILEBUFFER_H
#define AUDIOFILEBUFFER_H

#include <QByteArray>
#include <QSharedPointer>
#include <QVector>

class AudioFileBuffer
{
public:
    static QSharedPointer<AudioFileBuffer> fromDataStream(QDataStream& in);
    static QSharedPointer<AudioFileBuffer> fromWavFile(QString fileName);
    ~AudioFileBuffer();

    QList<QVector<float>> floatData() const;
    int numChannels() const;
    quint32 numFrames() const;
    quint32 bytesPerSample() const;
    quint32 sampleRate() const;

    friend QDataStream& operator<<(QDataStream&, const AudioFileBuffer&);
    friend QDataStream& operator>>(QDataStream&, AudioFileBuffer&);

private:
    AudioFileBuffer();
    AudioFileBuffer(const AudioFileBuffer &other);
    QList<QVector<float>> m_floatData;
    quint32 m_bytesPerSample;
    quint32 m_sampleRate;
};

#endif // AUDIOFILEBUFFER_H
