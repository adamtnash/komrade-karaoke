#include "trackdatacache.h"
#include "trackdata.h"

TrackDataCache::TrackDataCache(QString sourceFileName) :
    m_sourceFileName(sourceFileName)
{

}

const QString CACHE_V1 = "Nata Beats Cache v1";

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
    out << CACHE_V1;
    out << *(trackData.data());
}

QSharedPointer<TrackData> TrackDataCache::read()
{
    QString fileName = cacheFileName();
    QFileInfo fileInfo(fileName);
    QDir dir = fileInfo.dir();

    if (!dir.exists()) {
        return QSharedPointer<TrackData>();
    }

    QFile file(fileName);
    if (!file.exists()) {
        return QSharedPointer<TrackData>();
    }

    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);

    QString version;
    in >> version;
    if (version == CACHE_V1) {
        TrackData temp;
        in >> temp;
        if (in.status() != QDataStream::Ok) {
            return QSharedPointer<TrackData>();
        }
        return QSharedPointer<TrackData>(new TrackData(temp));
    }
    else {
        return QSharedPointer<TrackData>();
    }

}

QString TrackDataCache::cacheFileName()
{
    QFileInfo fileInfo(m_sourceFileName);
    return fileInfo.canonicalPath()+"/_nb_cache_/"+fileInfo.baseName()+".dat";
}
