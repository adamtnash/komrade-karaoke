#include "midimessagedialog.h"
#include "ui_midimessagedialog.h"

MidiMessageDialog::MidiMessageDialog(MidiInManager* midiManager, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MidiMessageDialog),
    m_select(new MidiMessageSelect(midiManager, this))
{
    ui->setupUi(this);

    ui->verticalLayout->insertWidget(0, m_select);
}

MidiMessageDialog::~MidiMessageDialog()
{
    delete ui;
}

QByteArray MidiMessageDialog::getMidiMessage(MidiInManager *midiManager, QByteArray current, bool *accepted, QWidget *parent)
{
    QScopedPointer<MidiMessageDialog> dialog(new MidiMessageDialog(midiManager, parent));
    dialog->setMessage(current);
    dialog->setWindowTitle("Playback MIDI Control");
    if (dialog->exec()) {
        *accepted = true;
        return dialog->getMessage();
    }
    else {
        *accepted = false;
        return QByteArray();
    }
}

void MidiMessageDialog::setMessage(QByteArray msg)
{
    m_select->setMessage(msg);
}

QByteArray MidiMessageDialog::getMessage() const
{
    return m_select->getMessage();
}
