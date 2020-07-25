#ifndef TRACKDATA_H
#define TRACKDATA_H

#include <QPixmap>
#include <QSharedPointer>
#include "audiofilebuffer.h"

class TrackData {
public:
    static QSharedPointer<TrackData> fromFileName(QString fileName);

    QString fileName() const;

    double bpm() const;
    void setBpm(double bpm);

    int midiTrigger() const;
    void setMidiTrigger(int midiTrigger);

    QPixmap waveform() const;

    QPixmap waveformPreview() const;

    QSharedPointer<AudioFileBuffer> buffer() const;

    QVector<float> samples() const;
    int sampleCount() const;

    double samplesPerBeat() const;

    QColor baseColor() const;
    void setBaseColor(const QColor &baseColor);

private:
    TrackData() {}
    void renderWaveforms();

    QString m_fileName;
    double m_bpm;
    int m_midiTrigger;
    QPixmap m_waveform;
    QPixmap m_waveformPreview;
    QSharedPointer<AudioFileBuffer> m_buffer;
    QColor m_baseColor;

    friend class TrackDataCache;
};

#endif // TRACKDATA_H
