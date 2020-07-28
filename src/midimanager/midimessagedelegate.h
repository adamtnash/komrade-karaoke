#ifndef MIDIMESSAGEDELEGATE_H
#define MIDIMESSAGEDELEGATE_H

#include <QStyledItemDelegate>
#include "midiinmanager.h"

class MidiMessageDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    MidiMessageDelegate(MidiInManager* midiManager, QObject* parent);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

private:
    MidiInManager* m_midiManager;
};

#endif // MIDIMESSAGEDELEGATE_H
