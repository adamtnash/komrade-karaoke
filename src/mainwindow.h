#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include "RtAudio.h"
#include "RtMidi.h"
#include "trackfoldermodel.h"
#include "playbackmanager.h"
#include "midimanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static int playbackCallback(void *outputBuffer, void *inputBuffer, unsigned int nFrames, double streamTime, RtAudioStreamStatus status, void *userData);
    static void playbackErrorCallback(RtAudioError::Type type, const std::string &errorText);

private slots:
    void on_pb_togglePlay_clicked();
    void selectAudioDeviceAt(int index);
    void on_pb_backingTrackSelect_clicked();

    void loadSettings();
    void initAudioDevices();
    void setTrackFolder(QString dirName);
    void adjustToTrackInitialization();

    void checkAutoQueue(QString trackFileName);
    void queueTrack(QSharedPointer<TrackData> track);

    void on_pb_showTracks_clicked();

    void on_pb_configurePlayback_clicked();

    void handleMidi(QByteArray message);

private:
    Ui::MainWindow *ui;
    TrackFolderModel* m_model;
    PlaybackManager* m_playbackManager;
    QMap<QSharedPointer<TrackData>, QPushButton*> m_trackQueuePbs;

    QByteArray m_playMidiControl;

    int m_selectedDevice = -1;

    RtAudio* m_rtAudio;

    MidiManager* m_midiManager;
};
#endif // MAINWINDOW_H
