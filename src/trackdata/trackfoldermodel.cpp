#include "trackfoldermodel.h"
#include <QDebug>
#include <QPainter>

TrackFolderModel::TrackFolderModel(QSharedPointer<TrackFolder> trackFolder, QObject *parent)
    : QAbstractTableModel(parent),
      m_trackFolder(trackFolder)
{
    connect(trackFolder.data(), &TrackFolder::updated, this, &TrackFolderModel::fullReset);
}

QVariant TrackFolderModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        if (section == 0) {
            return "Track Name";
        }
        else if (section == 1) {
            return "BPM";
        }
        else if (section == 2) {
            return "MIDI Trigger";
        }
        else if (section == 3) {
            return "";
        }
        else if (section == 4) {
            return "Auto-Queue";
        }
        else if (section == 5) {
            return "Aux";
        }
    }

    return QVariant();
}


int TrackFolderModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_trackFolder->trackNames().size();
}

int TrackFolderModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return 6;
}

QVariant TrackFolderModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    auto tracks = m_trackFolder->trackNames();
    if (index.row() >= tracks.size()) {
        return QVariant();
    }
    auto track = tracks.at(index.row());
    auto trackData = m_trackFolder->trackData(track);

    if (role == Qt::DisplayRole) {
        if (index.column() == 0) {
            return track;
        }
        else if (index.column() == 1) {
            return trackData->bpm();
        }
        else if (index.column() == 2) {
            return trackData->midiTrigger().toHex();
        }
        else if (index.column() == 3) {
            return trackData->waveform();
        }
        else if (index.column() == 4) {
            return trackData->autoQueueTrack();
        }
        else if (index.column() == 5) {
            return trackData->auxTrack();
        }
    }
    else if (role == Qt::EditRole) {
        if (index.column() == 1) {
            return trackData->bpm();
        }
        else if (index.column() == 2) {
            return trackData->midiTrigger();
        }
        else if (index.column() == 4) {
            return trackData->autoQueueTrack();
        }
        else if (index.column() == 5) {
            return trackData->auxTrack();
        }
    }
    else if (role == Qt::DecorationRole) {
        if (index.column() == 3) {
            return trackData->waveformPreview();
        }
    }
    else if (role == Qt::SizeHintRole) {
        if (index.column() == 3) {
            return trackData->waveformPreview().size();
        }
    }
    else if (role == Qt::BackgroundRole) {
        return trackData->baseColor();
    }

    return QVariant();
}

bool TrackFolderModel::setData(const QModelIndex &index, const QVariant &value, int role)
{

    if (!(index.column() == 1
             || index.column() == 2
             || index.column() == 4
             || index.column() == 5 )) {
        return false;
    }

    if (data(index, role) != value) {

        auto tracks = m_trackFolder->trackNames();
        if (index.row() >= tracks.size()) {
            return false;
        }
        auto track = tracks.at(index.row());
        auto trackData = m_trackFolder->trackData(track);

        if (role == Qt::EditRole) {
            if (index.column() == 1) {
                trackData->setBpm(value.toDouble());
                emit dataChanged(index, index, {role, Qt::DisplayRole});
                return true;
            }
            else if (index.column() == 2) {
                trackData->setMidiTrigger(value.toByteArray());
                emit dataChanged(index, index, {role, Qt::DisplayRole});
                return true;
            }
            else if (index.column() == 4) {
                trackData->setAutoQueueTrack(value.toString());
                emit dataChanged(index, index, {role, Qt::DisplayRole});
                return true;
            }
            else if (index.column() == 5) {
                trackData->setAuxTrack(value.toString());
                emit dataChanged(index, index, {role, Qt::DisplayRole});
                return true;
            }
        }
    }
    return false;
}

Qt::ItemFlags TrackFolderModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (index.column() == 1
            || index.column() == 2
            || index.column() == 4
            || index.column() == 5) {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}

QSharedPointer<TrackData> TrackFolderModel::getTrackData(int row)
{
    if (row < 0 || row >= m_trackFolder->trackNames().size()) {
        return QSharedPointer<TrackData>();
    }

    auto track = m_trackFolder->trackNames().at(row);
    return m_trackFolder->trackData(track);
}

QSharedPointer<TrackData> TrackFolderModel::getTrackData(QString fileName)
{
    return m_trackFolder->trackData(fileName);
}

void TrackFolderModel::fullReset()
{
    beginResetModel();
    endResetModel();
}
