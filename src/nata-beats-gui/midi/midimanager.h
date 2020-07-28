#ifndef MIDIMANAGER_H
#define MIDIMANAGER_H

#include <QObject>
#include <QComboBox>
#include "RtMidi.h"

class MidiManager : public QObject
{
    Q_OBJECT
public:
    explicit MidiManager(QObject *parent = nullptr);
    ~MidiManager();

    void registerDeviceSelect(QComboBox* cb);
    static void midiInputCallback(double deltatime, std::vector<unsigned char> *message, void *userData);

signals:
    void midiRx(QByteArray);

private:
    RtMidiIn m_in;
};

#endif // MIDIMANAGER_H
