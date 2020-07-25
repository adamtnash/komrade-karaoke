#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include "RtAudio.h"
#include "trackfoldermodel.h"
#include "playbackmanager.h"

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

private slots:
    void on_pb_togglePlay_clicked();
    void on_cb_midiInput_currentIndexChanged(int index);
    void on_cb_audioOutput_currentIndexChanged(int index);
    void on_pb_backingTrackSelect_clicked();

    void loadSettings();
    void initAudioDevices();
    void setTrackFolder(QString dirName);
    void adjustToTrackInitialization();

private:
    Ui::MainWindow *ui;
    TrackFolderModel* m_model;
    PlaybackManager* m_playbackManager;
    QMap<QSharedPointer<TrackData>, QPushButton*> m_trackQueuePbs;

    int m_selectedDevice = -1;

    RtAudio* m_rtAudio;
};
#endif // MAINWINDOW_H