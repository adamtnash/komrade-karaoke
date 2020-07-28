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

void MidiMessageDialog::setMessage(QByteArray msg)
{
    m_select->setMessage(msg);
}

QByteArray MidiMessageDialog::getMessage() const
{
    return m_select->getMessage();
}
