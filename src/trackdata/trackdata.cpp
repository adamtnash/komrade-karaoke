#include "trackdata.h"
#include <QFileInfo>
#include <QPainter>
#include <QPen>
#include <QRandomGenerator>
#include "trackdatacache.h"

TrackData::TrackData() :
    m_cacheDirty(true)
{

}

static int hue = 128;
QSharedPointer<TrackData> TrackData::fromFileName(QString fileName)
{
    QFileInfo fileInfo(fileName);
    TrackDataCache cache(fileName);
    auto cachedData = cache.read();

    if (!cachedData.isNull()) {
        return cachedData;
    }

    auto data = QSharedPointer<TrackData>(new TrackData());
    data->m_fileName = fileInfo.fileName();
    data->m_bpm = 120;
    data->m_midiTrigger = QByteArray();
    data->m_buffer = AudioFileBuffer::fromWavFile(fileName);
    data->m_isAux = false;
    data->m_autoPlay = false;
    data->m_autoStop = false;

    data->m_baseColor = QColor::fromHsl(hue, 0x44, 0xbb);
    hue = (hue + 20) % 255;

    data->renderWaveforms();

    cache.write(data);

    return data;
}


void TrackData::renderWaveforms()
{
    auto buff = m_buffer->floatData();
    int totalHeight = 120;
    int height = totalHeight / m_buffer->numChannels();
    int width = 360;

    QPixmap pix(width, totalHeight);
    for (int i = 0; i < m_buffer->numChannels(); i++) {
        QPixmap wave = renderSampleWave(width, height, m_buffer->floatData().at(i), m_baseColor);
        QPainter paint(&pix);
        paint.drawImage(QRect(0, height*i, width, height), wave.toImage(), wave.rect());
        paint.end();
    }

    m_waveform = pix.toImage();

    QPixmap preview(90, 30);
    QPainter prePaint(&preview);
    prePaint.fillRect(preview.rect(), QColor(200, 200, 200));
    prePaint.drawImage(preview.rect(), pix.toImage(), pix.rect());

    m_waveformPreview = preview;

    m_cacheDirty = true;
}

QPixmap TrackData::renderSampleWave(int width, int height, QVector<float> samples, QColor baseColor)
{
    QPixmap pix(width, height);
    int samplesPerX = samples.size()/width;
    double half = height/2.0;
    double dSamplesPerX = double(samplesPerX);

    QPainter paint(&pix);
    paint.fillRect(pix.rect(), baseColor);
    QPen avgWave(QColor::fromHsl(baseColor.hue(), 0xcc, 0x77), 1);
    QPen peakWave(QColor::fromHsl(baseColor.hue(), 0x99, 0x66), 1);
    for (int x = 0; x*samplesPerX + samplesPerX < samples.size(); x++) {
        double high = 0;
        double low = 0;
        double avgHigh = 0;
        double avgLow = 0;
        for (int i = x*samplesPerX; i < x*samplesPerX + samplesPerX; i++) {
            float sample = samples.at(i);
            if (sample > 0) {
                if (sample > high) {
                    high = sample;
                }
                avgHigh += sample;
            }
            else if (sample < 0) {
                if (sample < low) {
                    low = sample;
                }
                avgLow += sample;
            }
        }
        avgHigh /= dSamplesPerX;
        avgLow /= dSamplesPerX;

        paint.setPen(peakWave);
        paint.drawLine(x, half - half*high, x, half - half*low);
        paint.setPen(avgWave);
        paint.drawLine(x, half - half*avgHigh, x, half - half*avgLow);
    }
    paint.end();

    return pix;
}

QStringList TrackData::queueGroup() const
{
    return m_queueGroup;
}

void TrackData::setQueueGroup(const QStringList &queueGroup)
{
    m_queueGroup = queueGroup;
    m_cacheDirty = true;
}

bool TrackData::isAux() const
{
    return m_isAux;
}

void TrackData::setIsAux(bool isAux)
{
    m_isAux = isAux;
    m_cacheDirty = true;
}

bool TrackData::autoPlay() const
{
    return m_autoPlay;
}

void TrackData::setAutoPlay(bool autoPlay)
{
    m_autoPlay = autoPlay;
    m_cacheDirty = true;
}

bool TrackData::autoStop() const
{
    return m_autoStop;
}

void TrackData::setAutoStop(bool autoStop)
{
    m_autoStop = autoStop;
    m_cacheDirty = true;
}

QColor TrackData::baseColor() const
{
    return m_baseColor;
}

void TrackData::setBaseColor(const QColor &baseColor)
{
    m_baseColor = baseColor;
    renderWaveforms();
}

QString TrackData::auxTrack() const
{
    return m_auxTrack;
}

void TrackData::setAuxTrack(const QString &auxTrack)
{
    m_auxTrack = auxTrack;
    m_cacheDirty = true;
}

QString TrackData::autoQueueTrack() const
{
    return m_autoQueueTrack;
}

void TrackData::setAutoQueueTrack(const QString &autoQueueTrack)
{
    m_autoQueueTrack = autoQueueTrack;
    m_cacheDirty = true;
}

QString TrackData::fileName() const
{
    return m_fileName;
}

double TrackData::bpm() const
{
    return m_bpm;
}

void TrackData::setBpm(double bpm)
{
    m_bpm = bpm;
    m_cacheDirty = true;
}

QByteArray TrackData::midiTrigger() const
{
    return m_midiTrigger;
}

void TrackData::setMidiTrigger(QByteArray midiTrigger)
{
    m_midiTrigger = midiTrigger;
    m_cacheDirty = true;
}

QImage TrackData::waveform() const
{
    return m_waveform;
}

QPixmap TrackData::waveformPreview() const
{
    return m_waveformPreview;
}

QSharedPointer<AudioFileBuffer> TrackData::buffer() const
{
    return m_buffer;
}

QVector<float> TrackData::channelSamples(int channel) const
{
    return m_buffer->floatData().at(channel % m_buffer->numChannels());
}

int TrackData::frameCount() const
{
    return m_buffer->numFrames();
}

int TrackData::channelCount() const
{
    return m_buffer->numChannels();
}

double TrackData::samplesPerBeat() const
{
    return double(m_buffer->sampleRate() * 60) / m_bpm;
}

const QString VERSION_1 = "track_data_v1";
QDataStream &operator<<(QDataStream & out, const TrackData &data)
{
    out << VERSION_1;
    out << data.fileName();
    out << data.bpm();
    out << data.midiTrigger();
    out << data.baseColor();
    out << data.auxTrack();
    out << data.autoQueueTrack();
    out << data.autoStop();
    out << data.autoPlay();
    out << data.isAux();
    out << data.queueGroup();
    out << data.waveform();
    out << data.waveformPreview();
    out << *(data.buffer().data());

    return out;
}

QDataStream &operator>>(QDataStream &in, TrackData &data)
{
    QString version;
    in >> version;
    if (version == VERSION_1) {
        in >> data.m_fileName;
        in >> data.m_bpm;
        in >> data.m_midiTrigger;
        in >> data.m_baseColor;
        in >> data.m_auxTrack;
        in >> data.m_autoQueueTrack;
        in >> data.m_autoStop;
        in >> data.m_autoPlay;
        in >> data.m_isAux;
        in >> data.m_queueGroup;
        in >> data.m_waveform;
        in >> data.m_waveformPreview;
        data.m_buffer = AudioFileBuffer::fromDataStream(in);
    }
    else {
        in.setStatus(QDataStream::Status::ReadCorruptData);
    }

    return in;
}
