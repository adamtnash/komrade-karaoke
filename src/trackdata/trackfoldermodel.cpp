#include "trackfoldermodel.h"
#include <QDebug>
#include <QPainter>

const int TrackFolderModel::NAME_COL = 0;
const int TrackFolderModel::BPM_COL = -1;
const int TrackFolderModel::MIDI_COL = 1;
const int TrackFolderModel::COLOR_COL = -1;
const int TrackFolderModel::AUX_TRACK_COL = 3;
const int TrackFolderModel::AUTO_QUEUE_COL = 4;
const int TrackFolderModel::AUTO_STOP_COL = 7;
const int TrackFolderModel::AUTO_PLAY_COL = 6;
const int TrackFolderModel::IS_AUX_COL = 2;
const int TrackFolderModel::WAVEFORM_COL = 8;
const int TrackFolderModel::QUEUE_GROUP_COL = 5;

const int TrackFolderModel::COL_COUNT = 9;

const QSet<int> EDITABLE_COLS = {
    TrackFolderModel::BPM_COL,
    TrackFolderModel::MIDI_COL,
    TrackFolderModel::AUX_TRACK_COL,
    TrackFolderModel::AUTO_QUEUE_COL,
    TrackFolderModel::AUTO_STOP_COL,
    TrackFolderModel::AUTO_PLAY_COL,
    TrackFolderModel::QUEUE_GROUP_COL,
    TrackFolderModel::IS_AUX_COL
};

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
        if (section == NAME_COL) {
            return "Track Name";
        }
        else if (section == BPM_COL) {
            return "BPM";
        }
        else if (section == MIDI_COL) {
            return "MIDI Trigger";
        }
        else if (section == AUTO_QUEUE_COL) {
            return "Auto-Queue";
        }
        else if (section == AUX_TRACK_COL) {
            return "Aux Track";
        }
        else if (section == IS_AUX_COL) {
            return "Aux";
        }
        else if (section == AUTO_STOP_COL) {
            return "Auto Stop";
        }
        else if (section == AUTO_PLAY_COL) {
            return "Auto Play";
        }
        else if (section == QUEUE_GROUP_COL) {
            return "Queue Group";
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

    return COL_COUNT;
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

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        if (index.column() == NAME_COL) {
            return track;
        }
        else if (index.column() == BPM_COL) {
            return trackData->bpm();
        }
        else if (index.column() == MIDI_COL) {
            return trackData->midiTrigger().toHex();
        }
        else if (index.column() == WAVEFORM_COL) {
            return trackData->waveform();
        }
        else if (index.column() == AUTO_QUEUE_COL) {
            return trackData->autoQueueTrack();
        }
        else if (index.column() == AUX_TRACK_COL) {
            return trackData->auxTrack();
        }
        else if (index.column() == IS_AUX_COL) {
            return trackData->isAux();
        }
        else if (index.column() == AUTO_PLAY_COL) {
            return trackData->autoPlay();
        }
        else if (index.column() == AUTO_STOP_COL) {
            return trackData->autoStop();
        }
        else if (index.column() == QUEUE_GROUP_COL) {
            if (role == Qt::DisplayRole) {
                if (trackData->queueGroup().isEmpty()) {
                    return QVariant();
                }
                else {
                    return QString("%1 tracks (%2)").arg(trackData->queueGroup().size()).arg(trackData->queueGroup().join(","));
                }
            }
            else {
                return trackData->queueGroup();
            }
        }
    }
    else if (role == Qt::DecorationRole) {
        if (index.column() == WAVEFORM_COL) {
            return trackData->waveformPreview();
        }
    }
    else if (role == Qt::SizeHintRole) {
        if (index.column() == WAVEFORM_COL) {
            return trackData->waveformPreview().size();
        }
    }
    else if (role == Qt::BackgroundRole) {
        if (index.column() == AUTO_QUEUE_COL) {
            auto queueTrackData = m_trackFolder->trackData(trackData->autoQueueTrack());
            if (!queueTrackData.isNull()) {
                return queueTrackData->baseColor().lighter(120);
            }
        }
        else if (index.column() == AUX_TRACK_COL) {
            auto auxTrackData = m_trackFolder->trackData(trackData->auxTrack());
            if (!auxTrackData.isNull()) {
                return auxTrackData->baseColor().lighter(120);
            }
        }
        return trackData->baseColor().lighter(120);
    }

    return QVariant();
}

bool TrackFolderModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!EDITABLE_COLS.contains(index.column())) {
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
            if (index.column() == BPM_COL) {
                trackData->setBpm(value.toDouble());
                emit dataChanged(index, index, {role, Qt::DisplayRole});
                return true;
            }
            else if (index.column() == MIDI_COL) {
                trackData->setMidiTrigger(value.toByteArray());
                emit dataChanged(index, index, {role, Qt::DisplayRole});
                return true;
            }
            else if (index.column() == AUTO_QUEUE_COL) {
                trackData->setAutoQueueTrack(value.toString());
                emit dataChanged(index, index, {role, Qt::DisplayRole});
                return true;
            }
            else if (index.column() == AUX_TRACK_COL) {
                trackData->setAuxTrack(value.toString());
                emit dataChanged(index, index, {role, Qt::DisplayRole});
                return true;
            }
            else if (index.column() == IS_AUX_COL) {
                trackData->setIsAux(value.toBool());
                emit dataChanged(index, index, {role, Qt::DisplayRole});
                return true;
            }
            else if (index.column() == AUTO_PLAY_COL) {
                trackData->setAutoPlay(value.toBool());
                emit dataChanged(index, index, {role, Qt::DisplayRole});
                return true;
            }
            else if (index.column() == AUTO_STOP_COL) {
                trackData->setAutoStop(value.toBool());
                emit dataChanged(index, index, {role, Qt::DisplayRole});
                return true;
            }
            else if (index.column() == QUEUE_GROUP_COL) {
                trackData->setQueueGroup(value.toStringList());
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

    if (EDITABLE_COLS.contains(index.column())) {
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
