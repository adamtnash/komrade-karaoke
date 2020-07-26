#ifndef TRACKDATACACHE_H
#define TRACKDATACACHE_H

#include <QString>
#include <QSharedPointer>
#include <QDir>
#include "trackdata.h"

class TrackDataCache
{
public:
    TrackDataCache(QString sourceFileName);

    void write(QSharedPointer<TrackData> trackData);
    QSharedPointer<TrackData> read();

private:
    QString cacheFileName();
    QString m_sourceFileName;
    TrackData m_data;
};

#endif // TRACKDATACACHE_H
