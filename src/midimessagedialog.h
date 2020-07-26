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
    explicit MidiMessageDialog(MidiManager* midiManager, QWidget *parent = nullptr);
    ~MidiMessageDialog();

    void setMessage(QByteArray msg);
    QByteArray getMessage() const;

private:
    Ui::MidiMessageDialog *ui;
    MidiMessageSelect *m_select;
};

#endif // MIDIMESSAGEDIALOG_H
