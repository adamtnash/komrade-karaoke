#include "trackcomboboxdelegate.h"
#include <QComboBox>
#include "trackfoldermodel.h"

TrackComboBoxDelegate::TrackComboBoxDelegate(QObject *parent,
                                             std::function<bool(QSharedPointer<TrackData>)> filter) :
    QStyledItemDelegate(parent),
    m_filter(filter)
{

}

QWidget *TrackComboBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QComboBox *cb = new QComboBox(parent);
    auto model = (TrackFolderModel*)index.model();
    cb->addItem("< No Track >", QString());
    for (int row = 0; row < model->rowCount(); row++) {
        auto trackData = model->getTrackData(row);
        if (m_filter(trackData)) {
            cb->addItem(trackData->fileName(), trackData->fileName());
        }
    }
    return cb;
}


void TrackComboBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *cb = qobject_cast<QComboBox *>(editor);
    const QString currentText = index.data(Qt::EditRole).toString();
    const int idx = cb->findText(currentText);
    if (idx >= 0) {
       cb->setCurrentIndex(idx);
    }
    else {
       cb->setCurrentIndex(0);
    }
}


void TrackComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *cb = qobject_cast<QComboBox *>(editor);
    model->setData(index, cb->currentData(), Qt::EditRole);
}
