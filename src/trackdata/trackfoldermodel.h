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

    static const int NAME_COL;
    static const int BPM_COL;
    static const int MIDI_COL;
    static const int COLOR_COL;
    static const int AUX_TRACK_COL;
    static const int AUTO_QUEUE_COL;
    static const int AUTO_STOP_COL;
    static const int AUTO_PLAY_COL;
    static const int IS_AUX_COL;
    static const int WAVEFORM_COL;
    static const int QUEUE_GROUP_COL;

    static const int COL_COUNT;

signals:
    void initialized();

private slots:
    void fullReset();

private:
    QSharedPointer<TrackFolder> m_trackFolder;
};

#endif // TRACKFOLDERMODEL_H
