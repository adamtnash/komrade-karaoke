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
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_model(nullptr),
      m_playbackManager(new PlaybackManager(this)),
      m_midiInManager(new MidiInManager(this))
{
    ui->setupUi(this);
    this->setWindowTitle("Nata Beats");

    auto trackDelegate = new TrackComboBoxDelegate(this);
    ui->tv_tracks->setItemDelegateForColumn(4, trackDelegate);
    ui->tv_tracks->setItemDelegateForColumn(5, trackDelegate);

    auto midiDelegate = new MidiMessageDelegate(m_midiInManager, this);
    ui->tv_tracks->setItemDelegateForColumn(2, midiDelegate);

    ui->playbackLayout->addWidget(new PlaybackDisplay(m_playbackManager, this));

    connect(m_playbackManager, SIGNAL(opened()), this, SLOT(checkPlayback()), Qt::QueuedConnection);
    connect(m_playbackManager, SIGNAL(closed()), this, SLOT(checkPlayback()), Qt::QueuedConnection);
    connect(m_playbackManager, SIGNAL(started()), this, SLOT(playbackStarted()), Qt::QueuedConnection);
    connect(m_playbackManager, SIGNAL(stopped()), this, SLOT(playbackStopped()), Qt::QueuedConnection);
    checkPlayback();

    loadSettings();
    initAudio();
    initMidi();

    connect(m_midiInManager, SIGNAL(midiRx(QByteArray)), this, SLOT(handleMidi(QByteArray)), Qt::QueuedConnection);

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
    Settings::write("playMidiControl", m_playMidiControl, "Playback");
    Settings::write("lastOutputDevice", m_playbackManager->currentDevice(), "Playback");
    Settings::write("lastMidiPort", m_midiInManager->currentPort(), "MIDI");

    if (!m_trackFolder.isNull()) {
        m_trackFolder->writeToCache();
    }

    delete ui;
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

void MainWindow::setTrackFolder(QString dirName)
{
    QDir dir(dirName);

    if (!dir.exists()) {
        return;
    }

    ui->le_backingTrack->setText(dirName);
    Settings::write("lastDir", dirName);

    if (!m_trackFolder.isNull()) {
        m_trackFolder->writeToCache();
    }

    m_trackFolder = TrackFolder::fromPath(dirName);
    if (m_trackFolder.isNull()) {
        reportError(QString("Failed to initialize track folder '%1'").arg(dirName));
        return;
    }
    connect(m_trackFolder.data(), SIGNAL(updated()), this, SLOT(adjustToTrackInitialization()));
    adjustToTrackInitialization();

    m_model = new TrackFolderModel(m_trackFolder, this);

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

    for (auto trackData : m_trackFolder->tracks()) {
        if (trackData.isNull()) {
            continue;
        }
        auto pb = new QPushButton(trackData->fileName());
        ui->triggerLayout->layout()->addWidget(pb);
        m_trackQueuePbs.insert(trackData, pb);

        connect(pb, &QPushButton::clicked, [this, trackData]() {
            this->queueTrack(trackData);
        });
    }
}

void MainWindow::checkAutoQueue(QString trackFileName)
{
    auto track = m_trackFolder->trackData(trackFileName);
    if (track.isNull()) {
        return;
    }
    if (track->autoQueueTrack().isEmpty()) {
        return;
    }
    auto autoQueue = m_trackFolder->trackData(track->autoQueueTrack());
    if (autoQueue.isNull()) {
        return;
    }
    this->queueTrack(autoQueue);
}

void MainWindow::queueTrack(QSharedPointer<TrackData> track)
{
    QSharedPointer<TrackData> auxTrack;
    if (!track->auxTrack().isEmpty()) {
        auxTrack = m_trackFolder->trackData(track->auxTrack());
    }
    this->m_playbackManager->setQueuedTrack(track, auxTrack);
}

void MainWindow::initAudio()
{
    disconnect(ui->cb_audioOutput, &QComboBox::currentTextChanged, this, &MainWindow::selectAudioDevice);
    ui->cb_audioOutput->clear();
    auto devices = m_playbackManager->pollDevices();
    ui->cb_audioOutput->addItems(devices);

    QString device;
    if (devices.size() > 0) {
        device = devices.at(0);
    }

    QVariant lastDevice = Settings::read("lastOutputDevice", "Playback");
    if (!lastDevice.isNull() && devices.contains(lastDevice.toString())) {
        device = lastDevice.toString();
    }

    if (!device.isEmpty()) {
        selectAudioDevice(device);
        ui->cb_audioOutput->setCurrentText(device);
    }

    connect(ui->cb_audioOutput, &QComboBox::currentTextChanged, this, &MainWindow::selectAudioDevice);
}

void MainWindow::selectAudioDevice(QString deviceName)
{
    m_playbackManager->openDevice(deviceName);
}

void MainWindow::checkPlayback()
{
    ui->pb_togglePlay->setText("Play");
    ui->pb_togglePlay->setEnabled(!m_playbackManager->currentDevice().isEmpty());
}

void MainWindow::playbackStarted()
{
    ui->pb_togglePlay->setText("Stop");
}

void MainWindow::playbackStopped()
{
    ui->pb_togglePlay->setText("Play");
}

void MainWindow::initMidi()
{
    disconnect(ui->cb_midiInput, &QComboBox::currentTextChanged, this, &MainWindow::selectMidiPort);
    ui->cb_midiInput->clear();
    auto ports = m_midiInManager->pollPorts();
    ui->cb_midiInput->addItems(ports);

    QString port;
    if (ports.size() > 0) {
        port = ports.at(0);
    }

    QVariant lastPort = Settings::read("lastMidiPort", "MIDI");
    if (!lastPort.isNull() && ports.contains(lastPort.toString())) {
        port = lastPort.toString();
    }

    if (!port.isEmpty()) {
        selectMidiPort(port);
        ui->cb_midiInput->setCurrentText(port);
    }

    connect(ui->cb_midiInput, &QComboBox::currentTextChanged, this, &MainWindow::selectMidiPort);
}

void MainWindow::selectMidiPort(QString portName)
{
    if (!m_midiInManager->openPort(portName)) {
        reportError(QString("Failed to open MIDI port '%1'").arg(portName), "MIDI Error");
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
        auto track = m_trackFolder->trackByMidiTrigger(message);
        if (!track.isNull()) {
            this->queueTrack(track);
        }
    }
}

void MainWindow::reportError(QString errorText, QString title)
{
    QMessageBox::warning(this, title, errorText);
}

void MainWindow::on_pb_togglePlay_clicked()
{
    if (m_playbackManager->isRunning()) {
        m_playbackManager->stop();
    }
    else {
        m_playbackManager->start();
    }
}

void MainWindow::on_pb_showTracks_clicked()
{
    ui->scroll_trackButtons->setHidden(!ui->scroll_trackButtons->isHidden());
}

void MainWindow::on_pb_configurePlayback_clicked()
{
    MidiMessageDialog * dialog = new MidiMessageDialog(m_midiInManager, this);
    dialog->setMessage(m_playMidiControl);
    dialog->setWindowTitle("Playback MIDI Control");
    if (dialog->exec()) {
        m_playMidiControl = dialog->getMessage();
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

static float slideMax = float(100 * 100);
void MainWindow::on_hs_volume_valueChanged(int value)
{
    m_playbackManager->setVolume(float(value * value)/slideMax);
}
