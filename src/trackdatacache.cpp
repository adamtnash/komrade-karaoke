#include "trackdatacache.h"
#include "trackdata.h"

TrackDataCache::TrackDataCache(QString sourceFileName) :
    m_sourceFileName(sourceFileName)
{

}

void TrackDataCache::write(QSharedPointer<TrackData> trackData)
{
    QString fileName = cacheFileName();
    QFileInfo fileInfo(fileName);
    QDir dir = fileInfo.dir();

    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QFile file(fileName);
    file.open(QIODevice::WriteOnly | QIODevice::Truncate);
    QDataStream out(&file);
    out << trackData->waveform() << trackData->waveformPreview();
}

const TrackData* TrackDataCache::read()
{
    QString fileName = cacheFileName();
    QFileInfo fileInfo(fileName);
    QDir dir = fileInfo.dir();

    if (!dir.exists()) {
        return nullptr;
    }

    QFile file(fileName);
    if (!file.exists()) {
        return nullptr;
    }

    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);
    in >> m_data.m_waveform >> m_data.m_waveformPreview;

    return &m_data;
}

QString TrackDataCache::cacheFileName()
{
    QFileInfo fileInfo(m_sourceFileName);
    return fileInfo.canonicalPath()+"/_nb_cache_/"+fileInfo.baseName()+".dat";
}
