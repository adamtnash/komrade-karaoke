#ifndef TRACKCOMBOBOXDELEGATE_H
#define TRACKCOMBOBOXDELEGATE_H

#include <QStyledItemDelegate>

class TrackComboBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    TrackComboBoxDelegate(QObject* parent);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
};

#endif // TRACKCOMBOBOXDELEGATE_H
