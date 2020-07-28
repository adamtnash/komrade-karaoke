#include "trackfolder.h"
#include <QDir>
#include "trackdatacache.h"

TrackFolder::TrackFolder()
{

}

QSharedPointer<TrackFolder> TrackFolder::fromPath(QString folderPath)
{
    QDir dir(folderPath);
    if (!dir.exists()) {
        return QSharedPointer<TrackFolder>();
    }
    QSharedPointer<TrackFolder> folder(new TrackFolder());
    folder->m_folderPath = folderPath;
    folder->m_trackWatcher = new QFileSystemWatcher({folderPath}, folder.data());
    folder->initialize();
    return folder;
}

void TrackFolder::initialize()
{
    // Disconnect watcher so that cache writes don't trigger another init
    disconnect(m_trackWatcher,
            SIGNAL(directoryChanged(QString)),
            this,
            SLOT(initialize()));

    m_trackDataMap.clear();
    QDir dir(m_folderPath);
    m_orderedTrackNames = dir.entryList({"*.wav"}, QDir::Files, QDir::Name);

    for (auto track : m_orderedTrackNames) {
        if (!m_trackDataMap.contains(track)) {
            auto data = TrackData::fromFileName(dir.filePath(track));
            if (data.isNull()) {
                continue;
            }
            m_trackDataMap.insert(track, data);
            m_orderedTracks.append(data);
        }
    }

    connect(m_trackWatcher,
            SIGNAL(directoryChanged(QString)),
            this,
            SLOT(initialize()));

    emit updated();
}

QString TrackFolder::folderPath() const
{
    return m_folderPath;
}

QStringList TrackFolder::trackNames()
{
    return m_orderedTrackNames;
}

QList<QSharedPointer<TrackData>> TrackFolder::tracks()
{
    return m_orderedTracks;
}

QSharedPointer<TrackData> TrackFolder::trackData(QString trackName)
{
    return m_trackDataMap.value(trackName);
}

QSharedPointer<TrackData> TrackFolder::trackByMidiTrigger(QByteArray trigger)
{
    for (auto track : m_orderedTracks) {
        if (track->midiTrigger() == trigger) {
            return track;
        }
    }
    return QSharedPointer<TrackData>();
}

void TrackFolder::writeToCache()
{
    QDir dir(m_folderPath);
    if (!dir.exists()) {
        return;
    }
    for (auto track : m_orderedTracks) {
        TrackDataCache cache(dir.filePath(track->fileName()));
        cache.write(track);
    }
}
