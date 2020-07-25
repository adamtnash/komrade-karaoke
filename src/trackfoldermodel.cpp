#include "trackfoldermodel.h"
#include <QDebug>
#include <QPainter>
#include <QRandomGenerator>

TrackFolderModel::TrackFolderModel(QDir trackFolder, QObject *parent)
    : QAbstractTableModel(parent),
      m_trackFolder(trackFolder)
{
    m_trackWatcher = new QFileSystemWatcher({m_trackFolder.path()}, this);
    connect(m_trackWatcher,
            SIGNAL(directoryChanged(QString)),
            this,
            SLOT(initTrackData()));
    initTrackData();
}

void TrackFolderModel::initTrackData()
{
    QRandomGenerator r;
    r.seed(QRandomGenerator::system()->generate());
    beginResetModel();
    m_trackDataMap.clear();
    m_tracks = m_trackFolder.entryList({"*.wav"}, QDir::Files, QDir::Name);
    for (auto track : m_tracks) {
        if (!m_trackDataMap.contains(track)) {
            auto data = TrackData::fromFileName(m_trackFolder.filePath(track));
            if (data.isNull()) {
                continue;
            }
            m_trackDataMap.insert(track, data);
        }
    }
    endResetModel();
    emit initialized();
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
    }

    return QVariant();
}


int TrackFolderModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_tracks.size();
}

int TrackFolderModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return 4;
}

QVariant TrackFolderModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    auto tracks = m_tracks;
    if (index.row() >= tracks.size()) {
        return QVariant();
    }
    auto track = tracks.at(index.row());
    auto trackData = m_trackDataMap.value(track);

    if (role == Qt::DisplayRole) {
        if (index.column() == 0) {
            return track;
        }
        else if (index.column() == 1) {
            return trackData->bpm();
        }
        else if (index.column() == 2) {
            return trackData->midiTrigger();
        }
        else if (index.column() == 3) {
            return trackData->waveform();
        }
    }
    else if (role == Qt::EditRole) {
        if (index.column() == 1) {
            return trackData->bpm();
        }
        else if (index.column() == 2) {
            return trackData->midiTrigger();
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

    if (!(index.column() == 1 || index.column() == 2)) {
        return false;
    }

    if (data(index, role) != value) {

        auto tracks = m_tracks;
        if (index.row() >= tracks.size()) {
            return false;
        }
        auto track = tracks.at(index.row());
        auto trackData = m_trackDataMap.value(track);

        if (role == Qt::EditRole) {
            if (index.column() == 1) {
                trackData->setBpm(value.toDouble());
                emit dataChanged(index, index, {role});
                emit dataChanged(this->index(index.row(), 0), this->index(index.row(), this->columnCount()-1), {Qt::DisplayRole});
                return true;
            }
            else if (index.column() == 2) {
                trackData->setMidiTrigger(value.toInt());
                emit dataChanged(index, index, {role});
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

    if (index.column() == 1 || index.column() == 2) {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}


QSharedPointer<TrackData> TrackFolderModel::getTrackData(int row)
{
    if (row < 0 || row >= m_tracks.size()) {
        return QSharedPointer<TrackData>();
    }

    auto track = m_tracks.at(row);
    return m_trackDataMap.value(track);
}
