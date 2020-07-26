#include "midimanager.h"
#include "settings.h"

MidiManager::MidiManager(QObject *parent) : QObject(parent)
{
    m_in.setCallback( &MidiManager::midiInputCallback , static_cast<void*>(this));
}

MidiManager::~MidiManager()
{
    m_in.closePort();
}

void MidiManager::registerDeviceSelect(QComboBox *cb)
{
    for (unsigned int i = 0; i < m_in.getPortCount(); i++) {
        cb->addItem(m_in.getPortName(i).c_str(), QVariant(i));
    }

    auto selectMidi = [this, cb](int idx) {
        cb->setCurrentIndex(idx);
        m_in.closePort();
        m_in.openPort(cb->itemData(idx).toInt());

        Settings::write("lastMidiIn", cb->itemText(idx), "MIDI");
     };

    connect(cb, &QComboBox::currentIndexChanged, selectMidi);

    if (cb->count() > 0) {
        int currIdx = 0;
        QVariant lastDevice = Settings::read("lastMidiIn", "MIDI");
        if (!lastDevice.isNull()) {
            int lastIdx = cb->findText(lastDevice.toString());
            if (lastIdx >= 0) {
                currIdx = lastIdx;
            }
        }
        selectMidi(currIdx);
    }
}

void MidiManager::midiInputCallback(double deltatime, std::vector<unsigned char> *message, void *userData)
{
    MidiManager* manager = static_cast<MidiManager*>(userData);
    if (manager)
    {
        QByteArray ba(reinterpret_cast<char*>(message->data()), message->size());
        manager->midiRx(ba);
    }
}
