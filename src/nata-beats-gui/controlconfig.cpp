#include "controlconfig.h"
#include "ui_controlconfig.h"
#include "settings.h"
#include "midimessagedialog.h"

ControlConfig::ControlConfig(MidiInManager *midiInManager, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ControlConfig),
    m_midiInManager(midiInManager)
{
    ui->setupUi(this);
    loadSettings();
}

ControlConfig::~ControlConfig()
{
    saveSettings();
    delete ui;
}

void ControlConfig::loadSettings()
{
    QVariant playMidi = Settings::read("playMidiControl", "Playback");
    if (!playMidi.isNull()) {
        setPlayMidiControl(playMidi.toByteArray());
    }
    QVariant clearMidi = Settings::read("clearMidiControl", "Playback");
    if (!clearMidi.isNull()) {
        setClearMidiControl(clearMidi.toByteArray());
    }
    QVariant queueMidi = Settings::read("queueMidiControl", "Playback");
    if (!queueMidi.isNull()) {
        setQueueMidiControl(queueMidi.toByteArray());
    }
    QVariant volumeMidi = Settings::read("volumeControl", "Playback");
    if (!volumeMidi.isNull()) {
        setVolumeControl(volumeMidi.toByteArray());
    }
}

void ControlConfig::saveSettings()
{
    Settings::write("playMidiControl", m_playMidiControl, "Playback");
    Settings::write("clearMidiControl", m_clearMidiControl, "Playback");
    Settings::write("queueMidiControl", m_queueMidiControl, "Playback");
    Settings::write("volumeControl", m_volumeControl, "Playback");
}

void ControlConfig::setQueueMidiControl(const QByteArray &queueMidiControl)
{
    m_queueMidiControl = queueMidiControl;
    ui->le_queue->setText(m_queueMidiControl.toHex());
}

void ControlConfig::setClearMidiControl(const QByteArray &clearMidiControl)
{
    m_clearMidiControl = clearMidiControl;
    ui->le_clear->setText(m_clearMidiControl.toHex());
}

void ControlConfig::setPlayMidiControl(const QByteArray &playMidiControl)
{
    m_playMidiControl = playMidiControl;
    ui->le_play->setText(m_playMidiControl.toHex());
}

void ControlConfig::setVolumeControl(const QByteArray &volumeControl)
{
    m_volumeControl = volumeControl;
    ui->le_volume->setText(m_volumeControl.toHex());
}

QByteArray ControlConfig::playMidiControl() const
{
    return m_playMidiControl;
}

QByteArray ControlConfig::clearMidiControl() const
{
    return m_clearMidiControl;
}

QByteArray ControlConfig::queueMidiControl() const
{
    return m_queueMidiControl;
}

QByteArray ControlConfig::volumeControl() const
{
    return m_volumeControl;
}

void ControlConfig::on_pb_playEdit_clicked()
{
    bool accepted;
    QByteArray result = MidiMessageDialog::getMidiMessage(m_midiInManager, m_playMidiControl, &accepted, this);
    if (accepted) {
        setPlayMidiControl(result);
    }
}

void ControlConfig::on_pb_cycleEdit_clicked()
{
    bool accepted;
    QByteArray result = MidiMessageDialog::getMidiMessage(m_midiInManager, m_queueMidiControl, &accepted, this);
    if (accepted) {
        setQueueMidiControl(result);
    }
}

void ControlConfig::on_pb_clearEdit_clicked()
{
    bool accepted;
    QByteArray result = MidiMessageDialog::getMidiMessage(m_midiInManager, m_clearMidiControl, &accepted, this);
    if (accepted) {
        setClearMidiControl(result);
    }
}

void ControlConfig::on_pb_volumeControl_clicked()
{
    bool accepted;
    QByteArray result = MidiMessageDialog::getMidiMessage(m_midiInManager, m_volumeControl, &accepted, this);
    if (accepted && result.length() > 1) {
        setVolumeControl(result.mid(0, result.length()-1));
    }
}

