#ifndef TRACKFOLDER_H
#define TRACKFOLDER_H

#include <QObject>
#include <QSharedPointer>
#include <QFileSystemWatcher>
#include "trackdata.h"

class TrackFolder : public QObject
{
    Q_OBJECT
public:
    static QSharedPointer<TrackFolder> fromPath(QString folderPath);

    QString folderPath() const;

    QStringList trackNames();
    QList<QSharedPointer<TrackData>> tracks();
    QSharedPointer<TrackData> trackData(QString trackName);
    QSharedPointer<TrackData> trackByMidiTrigger(QByteArray trigger);


    void writeToCache();

signals:
    void updated();

private slots:
    void initialize();

private:
    explicit TrackFolder();

    QString m_folderPath;

    QStringList m_orderedTrackNames;
    QList<QSharedPointer<TrackData>> m_orderedTracks;
    QHash<QString, QSharedPointer<TrackData>> m_trackDataMap;

    QFileSystemWatcher* m_trackWatcher;
};

#endif // TRACKFOLDER_H
