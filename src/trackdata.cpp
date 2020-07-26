#include "trackdata.h"
#include <QFileInfo>
#include <QPainter>
#include <QPen>
#include <QRandomGenerator>
#include "trackdatacache.h"

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
    data->m_midiTrigger = 200;
    data->m_buffer = AudioFileBuffer::fromWavFile(fileName);

    data->m_baseColor = QColor::fromHsl(hue, 0x44, 0xbb);
    hue = (hue + 20) % 255;

    data->renderWaveforms();

    cache.write(data);

    return data;
}


void TrackData::renderWaveforms()
{
    auto buff = m_buffer->floatData();
    int samplesPerX = qMax(1000, buff.size()/360);
    double height = 120;
    double half = height/2.0;
    double dSamplesPerX = double(samplesPerX);
    QPixmap pix(buff.size()/samplesPerX, height);
    QPainter paint(&pix);
    paint.fillRect(pix.rect(), m_baseColor);
    QPen avgWave(QColor::fromHsl(m_baseColor.hue(), 0xcc, 0x77), 1);
    QPen peakWave(QColor::fromHsl(m_baseColor.hue(), 0x99, 0x66), 1);
    for (int x = 0; x*samplesPerX + samplesPerX < buff.size(); x++) {
        double high = 0;
        double low = 0;
        double avgHigh = 0;
        double avgLow = 0;
        for (int i = x*samplesPerX; i < x*samplesPerX + samplesPerX; i++) {
            float sample = buff.at(i);
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

    m_waveform = pix.toImage();

    QPixmap preview(90, 30);
    QPainter prePaint(&preview);
    prePaint.fillRect(preview.rect(), QColor(200, 200, 200));
    prePaint.drawImage(preview.rect(), pix.toImage(), pix.rect());

    m_waveformPreview = preview;
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
}

QString TrackData::autoQueueTrack() const
{
    return m_autoQueueTrack;
}

void TrackData::setAutoQueueTrack(const QString &autoQueueTrack)
{
    m_autoQueueTrack = autoQueueTrack;
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
}

int TrackData::midiTrigger() const
{
    return m_midiTrigger;
}

void TrackData::setMidiTrigger(int midiTrigger)
{
    m_midiTrigger = midiTrigger;
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

QVector<float> TrackData::samples() const
{
    return m_buffer->floatData();
}

int TrackData::sampleCount() const
{
    return m_buffer->floatData().size();
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
        in >> data.m_waveform;
        in >> data.m_waveformPreview;
        data.m_buffer = AudioFileBuffer::fromDataStream(in);
    }
    else {
        in.setStatus(QDataStream::Status::ReadCorruptData);
    }

    return in;
}
