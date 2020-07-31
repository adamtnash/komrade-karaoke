#ifndef MIDIMESSAGEDIALOG_H
#define MIDIMESSAGEDIALOG_H

#include <QDialog>
#include "midimessageselect.h"

namespace Ui {
class MidiMessageDialog;
}

class MidiMessageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MidiMessageDialog(MidiInManager* midiManager, QWidget *parent = nullptr);
    ~MidiMessageDialog();

    static QByteArray getMidiMessage(MidiInManager* midiManager, QByteArray current, bool *accepted, QWidget * parent = nullptr);

    void setMessage(QByteArray msg);
    QByteArray getMessage() const;

private:
    Ui::MidiMessageDialog *ui;
    MidiMessageSelect *m_select;
};

#endif // MIDIMESSAGEDIALOG_H
