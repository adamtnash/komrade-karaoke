#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include "trackfoldermodel.h"
#include "playbackmanager.h"
#include "midiinmanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pb_togglePlay_clicked();
    void on_pb_backingTrackSelect_clicked();
    void on_pb_showTracks_clicked();
    void on_pb_configurePlayback_clicked();

    void loadSettings();

    void setTrackFolder(QString dirName);
    void adjustToTrackInitialization();
    void checkAutoQueue(QString trackFileName);
    void queueTrack(QSharedPointer<TrackData> track);

    void initAudio();
    void selectAudioDevice(QString deviceName);
    void checkPlayback();
    void playbackStarted();
    void playbackStopped();

    void initMidi();
    void selectMidiPort(QString portName);
    void handleMidi(QByteArray message);

    void reportError(QString errorText, QString title = "Error");

    void on_hs_volume_valueChanged(int value);

private:
    Ui::MainWindow *ui;
    QSharedPointer<TrackFolder> m_trackFolder;
    TrackFolderModel* m_model;
    PlaybackManager* m_playbackManager;
    MidiInManager* m_midiInManager;

    QMap<QSharedPointer<TrackData>, QPushButton*> m_trackQueuePbs;
    QByteArray m_playMidiControl;
};
#endif // MAINWINDOW_H
