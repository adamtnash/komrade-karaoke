#include "trackcomboboxdelegate.h"
#include <QComboBox>
#include "trackfoldermodel.h"
#include <QStandardItemModel>

TrackComboBoxDelegate::TrackComboBoxDelegate(QObject *parent,
                                             std::function<bool(QSharedPointer<TrackData>)> filter,
                                             bool multiSelect) :
    QStyledItemDelegate(parent),
    m_filter(filter),
    m_multiSelect(multiSelect)
{

}

QWidget *TrackComboBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QComboBox *cb = new QComboBox(parent);
    auto model = (TrackFolderModel*)index.model();

    QStringList trackNames;
    for (int row = 0; row < model->rowCount(); row++) {
        auto trackData = model->getTrackData(row);
        if (m_filter(trackData)) {
            trackNames.append(trackData->fileName());
        }
    }

    if (!m_multiSelect) {
        cb->addItem("< No Track >", QString());
        for (auto trackName : trackNames) {
            cb->addItem(trackName, trackName);
        }
    }
    else {
        auto itemModel = new QStandardItemModel();
        itemModel->setColumnCount(1);
        for (auto trackName : trackNames) {
            QStandardItem *item = new QStandardItem(trackName);
            item->setData(trackName, Qt::UserRole);
            item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            itemModel->appendRow(item);
        }
        cb->setModel(itemModel);
    }


    return cb;
}


void TrackComboBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *cb = qobject_cast<QComboBox *>(editor);

    if (!m_multiSelect) {
        const QString currentText = index.data(Qt::EditRole).toString();
        const int idx = cb->findText(currentText);
        if (idx >= 0) {
           cb->setCurrentIndex(idx);
        }
        else {
           cb->setCurrentIndex(0);
        }
    }
    else {
        QStringList current = index.data(Qt::EditRole).toStringList();
        for (int row = 0; row < cb->count(); row++) {
            QString trackName = cb->itemData(row).toString();
            if (current.contains(trackName)) {
                cb->setItemData(row, Qt::Checked, Qt::CheckStateRole);
            }
            else {
                cb->setItemData(row, Qt::Unchecked, Qt::CheckStateRole);
            }
        }
    }
}


void TrackComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *cb = qobject_cast<QComboBox *>(editor);

    if (!m_multiSelect) {
        model->setData(index, cb->currentData(), Qt::EditRole);
    }
    else {
        QStringList selected;
        for (int row = 0; row < cb->count(); row++) {
            QString trackName = cb->itemData(row, Qt::UserRole + 1).toString();
            if (cb->itemData(row, Qt::CheckStateRole) == Qt::Checked) {
                selected.append(cb->itemData(row).toString());
            }
        }
        model->setData(index, selected, Qt::EditRole);
    }
}
