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
#include "midimessagedelegate.h"
#include "midimessagedialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_model(nullptr),
      m_playbackManager(new PlaybackManager()),
      m_rtAudio(nullptr),
      m_midiManager(new MidiManager(this))
{
    ui->setupUi(this);
    this->setWindowTitle("Nata Beats");

    auto trackDelegate = new TrackComboBoxDelegate(this);
    ui->tv_tracks->setItemDelegateForColumn(4, trackDelegate);
    ui->tv_tracks->setItemDelegateForColumn(5, trackDelegate);

    auto midiDelegate = new MidiMessageDelegate(m_midiManager, this);
    ui->tv_tracks->setItemDelegateForColumn(2, midiDelegate);

    ui->playbackLayout->addWidget(new PlaybackDisplay(m_playbackManager, this));

    initAudioDevices();

    loadSettings();

    m_midiManager->registerDeviceSelect(ui->cb_midiInput);
    connect(m_midiManager, SIGNAL(midiRx(QByteArray)), this, SLOT(handleMidi(QByteArray)), Qt::QueuedConnection);

    connect(m_playbackManager,
            SIGNAL(trackStarted(QString)),
            this,
            SLOT(checkAutoQueue(QString)),
            Qt::QueuedConnection);

    connect(m_playbackManager,
            SIGNAL(playbackStopped()),
            this,
            SLOT(on_pb_togglePlay_clicked()),
            Qt::QueuedConnection);
}

MainWindow::~MainWindow()
{
    Settings::write("windowSize", this->size(), "UI");
    Settings::write("windowPos", this->pos(), "UI");
    Settings::write("playMidiControl", m_playMidiControl, "Playback");

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
            qDebug() << "\nError while stopping RtAudio stream:\n" << e.getMessage().c_str() << '\n';
        }
        delete m_rtAudio;
        m_rtAudio = nullptr;
        ui->pb_togglePlay->setText("Play");
    }
    else {
        m_rtAudio = new RtAudio();
        RtAudio::DeviceInfo info = m_rtAudio->getDeviceInfo(m_selectedDevice);

        RtAudio::StreamParameters parameters;
        parameters.deviceId = m_selectedDevice;
        // stereo main and aux
        if (info.outputChannels >= 4) {
            parameters.nChannels = 4;
            m_playbackManager->setOutChannels(4);
        }
        // just stereo main
        else {
            parameters.nChannels = 2;
            m_playbackManager->setOutChannels(2);
        }
        unsigned int sampleRate = 44100;
        unsigned int bufferFrames = 256; // 256 sample frames
        RtAudio::StreamOptions options;
        //options.flags = RTAUDIO_NONINTERLEAVED;

        try {
            m_rtAudio->openStream( &parameters, NULL, RTAUDIO_FLOAT32,
                            sampleRate, &bufferFrames, &playbackCallback, m_playbackManager, &options );
            m_rtAudio->startStream();
            ui->pb_togglePlay->setText("Stop");
        }
        catch ( RtAudioError& e ) {
            qDebug() << "\nError while starting RtAudio stream:\n" << e.getMessage().c_str() << '\n';
            this->on_pb_togglePlay_clicked();
        }
    }
}

void MainWindow::selectAudioDeviceAt(int index)
{
    if (index >=0) {
        if (ui->cb_audioOutput->currentIndex() != index) {
            ui->cb_audioOutput->setCurrentIndex(index);
        }
        m_selectedDevice = ui->cb_audioOutput->itemData(index).toInt();
        ui->pb_togglePlay->setEnabled(true);
        Settings::write("lastOutputDevice", ui->cb_audioOutput->itemText(index), "Playback");
    }
    else {
        m_selectedDevice = -1;
        ui->pb_togglePlay->setEnabled(false);
    }
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

    QVariant playMidi = Settings::read("playMidiControl", "Playback");
    if (!playMidi.isNull()) {
        m_playMidiControl = playMidi.toByteArray();
    }
}

void MainWindow::initAudioDevices()
{
    disconnect(ui->cb_audioOutput, QOverload<int>::of(&QComboBox::currentIndexChanged),
               this, &MainWindow::selectAudioDeviceAt);

    ui->pb_togglePlay->setEnabled(false);
    m_selectedDevice = 0;

    ui->cb_audioOutput->clear();
    RtAudio dac;
    for (uint i = 0; i < dac.getDeviceCount(); i++) {
        auto info = dac.getDeviceInfo(i);
        if (info.probed) {
            ui->cb_audioOutput->addItem(QString(info.name.c_str()), QVariant(i));
        }
    }

    if (ui->cb_audioOutput->count() > 0) {
        int currIdx = 0;
        QVariant lastDevice = Settings::read("lastOutputDevice", "Playback");
        if (!lastDevice.isNull()) {
            int lastIdx = ui->cb_audioOutput->findText(lastDevice.toString());
            if (lastIdx >= 0) {
                currIdx = lastIdx;
            }
        }
        this->selectAudioDeviceAt(currIdx);
    }
    else {
        this->selectAudioDeviceAt(-1);
    }

    connect(ui->cb_audioOutput, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::selectAudioDeviceAt);
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

void MainWindow::playbackErrorCallback(RtAudioError::Type type, const std::string &errorText)
{
    qDebug() << "\nError with RtAudio playback" << errorText.c_str() << '\n';
}

void MainWindow::on_pb_showTracks_clicked()
{
    ui->scroll_trackButtons->setHidden(!ui->scroll_trackButtons->isHidden());
}

void MainWindow::on_pb_configurePlayback_clicked()
{
    MidiMessageDialog * dialog = new MidiMessageDialog(m_midiManager, this);
    dialog->setMessage(m_playMidiControl);
    dialog->setWindowTitle("Playback MIDI Control");
    if (dialog->exec()) {
        m_playMidiControl = dialog->getMessage();
    }
}

void MainWindow::handleMidi(QByteArray message)
{
    ui->lb_midiCheck->setText("0x"+message.toHex());
    if (message.isEmpty()) {
        return;
    }
    if (message == m_playMidiControl) {
        this->on_pb_togglePlay_clicked();
    }
    else {
        for (int row = 0; row < m_model->rowCount(); row++) {
            auto trackData = m_model->getTrackData(row);
            if (trackData.isNull()) {
                continue;
            }
            if (message == trackData->midiTrigger()) {
                this->queueTrack(trackData);
            }
        }
    }
}
