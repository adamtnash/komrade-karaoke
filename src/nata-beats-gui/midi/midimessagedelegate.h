#ifndef MIDIMESSAGEDELEGATE_H
#define MIDIMESSAGEDELEGATE_H

#include <QStyledItemDelegate>
#include "midimanager.h"

class MidiMessageDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    MidiMessageDelegate(MidiManager* midiManager, QObject* parent);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

private:
    MidiManager* m_midiManager;
};

#endif // MIDIMESSAGEDELEGATE_H
