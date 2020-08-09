#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include "trackfoldermodel.h"
#include "playbackmanager.h"
#include "midiinmanager.h"
#include "controlconfig.h"

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

    void loadSettings();

    void setTrackFolder(QString dirName);
    void adjustToTrackInitialization();
    void checkAutoQueue(QString trackFileName);
    void queueTrack(QSharedPointer<TrackData> track);

    void initAudio();
    void selectAudioDevice(QString deviceName);
    void selectMainOuts(int idx);
    void selectAuxOuts(int idx);
    void selectBufferSize(int idx);
    void bufferResized(unsigned int size);
    void selectSampleRate(int idx);
    void checkPlayback();
    void playbackStarted();
    void playbackStopped();

    void initMidi();
    void selectMidiPort(QString portName);
    void handleMidi(QByteArray message);

    void reportError(QString errorText, QString title = "Error");

    void on_hs_volume_valueChanged(int value);

    void on_pb_retryDevice_clicked();

private:
    Ui::MainWindow *ui;
    QSharedPointer<TrackFolder> m_trackFolder;
    TrackFolderModel* m_model;
    PlaybackManager* m_playbackManager;
    MidiInManager* m_midiInManager;
    ControlConfig* m_controlConfig;

    QMap<QSharedPointer<TrackData>, QPushButton*> m_trackQueuePbs;
};
#endif // MAINWINDOW_H
