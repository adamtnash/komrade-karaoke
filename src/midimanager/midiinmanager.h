#ifndef MIDIMANAGER_H
#define MIDIMANAGER_H

#include <QObject>
#include <QComboBox>

class RtMidiIn;

class MidiInManager : public QObject
{
    Q_OBJECT
public:
    explicit MidiInManager(QObject *parent = nullptr);
    ~MidiInManager();

    static void midiInputCallback(double deltatime, std::vector<unsigned char> *message, void *userData);

    QString currentPort() const;
    bool openPort(const QString &portName);

    QStringList pollPorts();

public slots:
    void closePort();

signals:
    void midiRx(QByteArray);

private:
    RtMidiIn *m_in;
    QString m_currentPort;
};

#endif // MIDIMANAGER_H
