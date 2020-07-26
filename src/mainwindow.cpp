#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include "trackfoldermodel.h"
#include <QDebug>
#include <QtMath>
#include "math.h"
#include "settings.h"
#include "playbackdisplay.h"
#include "trackcomboboxdelegate.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_model(nullptr),
      m_playbackManager(new PlaybackManager()),
      m_rtAudio(nullptr)
{
    ui->setupUi(this);
    this->setWindowTitle("Nata Beats");

    auto trackDelegate = new TrackComboBoxDelegate(this);
    ui->tv_tracks->setItemDelegateForColumn(4, trackDelegate);
    ui->tv_tracks->setItemDelegateForColumn(5, trackDelegate);

    ui->playbackLayout->addWidget(new PlaybackDisplay(m_playbackManager, this));

    initAudioDevices();

    loadSettings();

    connect(m_playbackManager,
            SIGNAL(trackStarted(QString)),
            this,
            SLOT(checkAutoQueue(QString)),
            Qt::QueuedConnection);
}

MainWindow::~MainWindow()
{
    Settings::write("windowSize", this->size(), "UI");
    Settings::write("windowPos", this->pos(), "UI");

    if (m_model) {
        m_model->writeDataToCache();
    }

    if (m_rtAudio) {
        m_rtAudio->abortStream();
        delete m_rtAudio;
    }
    delete ui;
    delete m_playbackManager;
}

void MainWindow::on_pb_togglePlay_clicked()
{
    if (m_rtAudio) {
        try {
            m_rtAudio->stopStream();
        }
        catch ( RtAudioError& e ) {
            qDebug() << '\n' << e.getMessage().c_str() << '\n';
        }
        m_rtAudio = nullptr;
    }
    else {
        m_rtAudio = new RtAudio();
        RtAudio::DeviceInfo info = m_rtAudio->getDeviceInfo(m_selectedDevice);

        RtAudio::StreamParameters parameters;
        parameters.deviceId = m_selectedDevice;
        // stereo main and aux
        if (info.outputChannels >= 4) {
            parameters.nChannels = 4;
        }
        // just stereo main
        else {
            parameters.nChannels = 2;
        }
        unsigned int sampleRate = 44100;
        unsigned int bufferFrames = 256; // 256 sample frames
        RtAudio::StreamOptions options;
        //options.flags = RTAUDIO_NONINTERLEAVED;

        try {
            qDebug() << "opening";
            m_rtAudio->openStream( &parameters, NULL, RTAUDIO_FLOAT32,
                            sampleRate, &bufferFrames, &playbackCallback, m_playbackManager, &options );
            m_rtAudio->startStream();
        }
        catch ( RtAudioError& e ) {
            qDebug() << '\n' << e.getMessage().c_str() << '\n';
        }
    }
}

void MainWindow::on_cb_midiInput_currentIndexChanged(int index)
{
}

void MainWindow::on_cb_audioOutput_currentIndexChanged(int index)
{
    m_selectedDevice = index;
    ui->pb_togglePlay->setEnabled(index >= 0);
}

void MainWindow::on_pb_backingTrackSelect_clicked()
{
    QString dirName = QFileDialog::getExistingDirectory(this, "Select Folder with Tracks");
    if (dirName.isEmpty()) {
        return;
    }
    setTrackFolder(dirName);
}

void MainWindow::loadSettings()
{
    QVariant lastDir = Settings::read("lastDir");
    if (!lastDir.isNull()) {
        setTrackFolder(lastDir.toString());
    }

    QVariant wSize = Settings::read("windowSize", "UI");
    QVariant wPos = Settings::read("windowPos", "UI");
    if (!wSize.isNull() && !wPos.isNull()) {
        this->move(wPos.toPoint());
        this->resize(wSize.toSize());
    }
}

void MainWindow::initAudioDevices()
{
    ui->pb_togglePlay->setEnabled(false);
    m_selectedDevice = 0;

    ui->cb_audioOutput->clear();
    RtAudio dac;
    qDebug() << dac.getDeviceCount();
    for (uint i = 0; i < dac.getDeviceCount(); i++) {
        auto info = dac.getDeviceInfo(i);
        ui->cb_audioOutput->addItem(QString(info.name.c_str()));
    }

    if (ui->cb_audioOutput->count() > 0) {
        ui->cb_audioOutput->setCurrentIndex(0);
    }
}


void MainWindow::setTrackFolder(QString dirName)
{
    QDir dir(dirName);

    if (!dir.exists()) {
        return;
    }

    ui->le_backingTrack->setText(dirName);
    Settings::write("lastDir", dirName);

    if (m_model) {
        m_model->writeDataToCache();
    }

    m_model = new TrackFolderModel(dir, this);
    connect(m_model, SIGNAL(initialized()), this, SLOT(adjustToTrackInitialization()));
    adjustToTrackInitialization();

    auto *oldSelectionModel = ui->tv_tracks->selectionModel();
    auto *oldModel = ui->tv_tracks->model();

    ui->tv_tracks->setModel(m_model);

    delete oldSelectionModel;
    delete oldModel;
}

void MainWindow::adjustToTrackInitialization()
{
    for (auto pb : m_trackQueuePbs.values()) {
        delete pb;
    }
    m_trackQueuePbs.clear();

    for (int row = 0; row < m_model->rowCount(); row++) {
        auto trackData = m_model->getTrackData(row);
        if (trackData.isNull()) {
            continue;
        }
        auto pb = new QPushButton(trackData->fileName());
        pb->setCheckable(true);
        pb->setAutoExclusive(true);
        ui->triggerLayout->layout()->addWidget(pb);
        m_trackQueuePbs.insert(trackData, pb);

        connect(pb, &QPushButton::toggled, [this, trackData](bool checked) {
            if (!checked) {
                return;
            }
            this->queueTrack(trackData);
        });
    }
}

void MainWindow::checkAutoQueue(QString trackFileName)
{
    auto track = m_model->getTrackData(trackFileName);
    if (track.isNull()) {
        return;
    }
    if (track->autoQueueTrack().isEmpty()) {
        return;
    }
    auto autoQueue = m_model->getTrackData(track->autoQueueTrack());
    if (autoQueue.isNull()) {
        return;
    }
    this->queueTrack(autoQueue);
}

void MainWindow::queueTrack(QSharedPointer<TrackData> track)
{
    QSharedPointer<TrackData> auxTrack;
    if (!track->auxTrack().isEmpty()) {
        auxTrack = m_model->getTrackData(track->auxTrack());
    }
    this->m_playbackManager->setQueuedTrack(track, auxTrack);
}


int MainWindow::playbackCallback(void *outputBuffer, void *inputBuffer, unsigned int nFrames, double streamTime, RtAudioStreamStatus status, void *userData)
{
    PlaybackManager* playbackManager = static_cast<PlaybackManager*>(userData);
    return playbackManager->writeNextAudioData(outputBuffer, nFrames);
}
