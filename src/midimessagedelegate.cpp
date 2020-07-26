#include "midimessagedelegate.h"
#include "midimessageselect.h"


MidiMessageDelegate::MidiMessageDelegate(MidiManager* midiManager, QObject *parent) :
    QStyledItemDelegate(parent),
    m_midiManager(midiManager)
{
}

QWidget *MidiMessageDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return new MidiMessageSelect(m_midiManager, parent);
}


void MidiMessageDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    MidiMessageSelect *select = static_cast<MidiMessageSelect*>(editor);
    select->setMessage(index.data(Qt::EditRole).toByteArray());
}


void MidiMessageDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    MidiMessageSelect *select = static_cast<MidiMessageSelect*>(editor);
    model->setData(index, select->getMessage(), Qt::EditRole);
}
