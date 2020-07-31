#ifndef TRACKCOMBOBOXDELEGATE_H
#define TRACKCOMBOBOXDELEGATE_H

#include <QStyledItemDelegate>
#include "trackdata.h"

class TrackComboBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    TrackComboBoxDelegate(QObject* parent,
                          std::function<bool(QSharedPointer<TrackData>)> filter = [](QSharedPointer<TrackData>){return true;},
                          bool multiSelect = false);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

private:
    std::function<bool(QSharedPointer<TrackData>)> m_filter;
    bool m_multiSelect;
};

#endif // TRACKCOMBOBOXDELEGATE_H
