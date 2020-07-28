#ifndef MIDIMESSAGESELECT_H
#define MIDIMESSAGESELECT_H

#include <QWidget>
#include "midiinmanager.h"

namespace Ui {
class MidiMessageSelect;
}

class MidiMessageSelect : public QWidget
{
    Q_OBJECT

public:
    explicit MidiMessageSelect(MidiInManager* midiManager, QWidget *parent = nullptr);
    ~MidiMessageSelect();

    void setMessage(QByteArray msg);
    QByteArray getMessage() const;


private:
    Ui::MidiMessageSelect *ui;
};

#endif // MIDIMESSAGESELECT_H
