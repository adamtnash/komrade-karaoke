#ifndef TRACKFOLDERMODEL_H
#define TRACKFOLDERMODEL_H

#include <QAbstractTableModel>
#include <QSharedPointer>
#include "trackfolder.h"

class TrackFolderModel : public QAbstractTableModel
{
    Q_OBJECT

public:

    explicit TrackFolderModel(QSharedPointer<TrackFolder> trackFolder, QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QSharedPointer<TrackData> getTrackData(int row);
    QSharedPointer<TrackData> getTrackData(QString fileName);

signals:
    void initialized();

private slots:
    void fullReset();

private:
    QSharedPointer<TrackFolder> m_trackFolder;
};

#endif // TRACKFOLDERMODEL_H
