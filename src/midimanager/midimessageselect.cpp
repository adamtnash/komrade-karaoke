#include "midimessageselect.h"
#include "ui_midimessageselect.h"

MidiMessageSelect::MidiMessageSelect(MidiInManager* midiManager, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MidiMessageSelect)
{
    ui->setupUi(this);

    connect(ui->pb_clear, &QPushButton::pressed, [this]() {
        ui->lb_msg->setText(QByteArray());
    });

    auto conn = connect(midiManager, &MidiInManager::midiRx, this, [this](QByteArray message) {
        setMessage(message);
    });
}

MidiMessageSelect::~MidiMessageSelect()
{
    delete ui;
}

void MidiMessageSelect::setMessage(QByteArray msg)
{
    ui->lb_msg->setText(msg.toHex());
}

QByteArray MidiMessageSelect::getMessage() const
{
    return QByteArray::fromHex(ui->lb_msg->text().toUtf8());
}
