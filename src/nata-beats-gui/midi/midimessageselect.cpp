#include "midimessageselect.h"
#include "ui_midimessageselect.h"

MidiMessageSelect::MidiMessageSelect(MidiManager* midiManager, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MidiMessageSelect)
{
    ui->setupUi(this);

    connect(ui->pb_clear, &QPushButton::pressed, [this]() {
        ui->lb_msg->setText(QByteArray());
    });

    auto conn = connect(midiManager, &MidiManager::midiRx, [this](QByteArray message) {
        setMessage(message);
    });

    connect(this, &QObject::destroyed, [conn](){
        disconnect(conn);
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
