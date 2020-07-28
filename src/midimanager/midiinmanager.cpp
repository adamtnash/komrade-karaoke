#include "midiinmanager.h"
#include "RtMidi.h"

MidiInManager::MidiInManager(QObject *parent) :
    QObject(parent),
    m_in(new RtMidiIn())
{
    m_in->setCallback( &MidiInManager::midiInputCallback , static_cast<void*>(this));
}

MidiInManager::~MidiInManager()
{
    closePort();
    delete m_in;
}

void MidiInManager::midiInputCallback(double deltatime, std::vector<unsigned char> *message, void *userData)
{
    MidiInManager* manager = static_cast<MidiInManager*>(userData);
    if (manager)
    {
        QByteArray ba(reinterpret_cast<char*>(message->data()), message->size());
        manager->midiRx(ba);
    }
}

QString MidiInManager::currentPort() const
{
    return m_currentPort;
}

bool MidiInManager::openPort(const QString &portName)
{
    m_currentPort = QString();
    if (m_in->isPortOpen()) {
        m_in->closePort();
    }

    auto ports = pollPorts();
    for (int i = 0; i < ports.size(); i++) {
        auto port = ports.at(i);
        if (port == portName) {
            m_in->openPort(i);
            if (m_in->isPortOpen()) {
                m_currentPort = portName;
                return true;
            }
            else {
                return false;
            }
        }
    }

    return false;
}

QStringList MidiInManager::pollPorts()
{
    QStringList ports;
    for (unsigned int i = 0; i < m_in->getPortCount(); i++) {
        ports.append(m_in->getPortName(i).c_str());
    }
    return ports;
}

void MidiInManager::closePort()
{
    m_in->closePort();
}
