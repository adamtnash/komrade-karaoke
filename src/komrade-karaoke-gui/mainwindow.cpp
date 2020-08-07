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
#include "pixmapdelegate.h"
#include "midimessagedialog.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_model(nullptr),
      m_playbackManager(new PlaybackManager()),
      m_midiInManager(new MidiInManager(this)),
      m_controlConfig(new ControlConfig(m_midiInManager, this))
{
    ui->setupUi(this);
    this->setWindowTitle("Komrade Karaoke");

    ui->tab_controls->layout()->addWidget(m_controlConfig);

    auto trackDelegate = new TrackComboBoxDelegate(this, [](QSharedPointer<TrackData> trackData) {
        return !trackData->isAux();
    });
    ui->tv_tracks->setItemDelegateForColumn(TrackFolderModel::AUTO_QUEUE_COL, trackDelegate);

    auto auxTrackDelegate = new TrackComboBoxDelegate(this, [](QSharedPointer<TrackData> trackData) {
            return trackData->isAux();
        });
    ui->tv_tracks->setItemDelegateForColumn(TrackFolderModel::AUX_TRACK_COL, auxTrackDelegate);


    auto queueGroupDelegate = new TrackComboBoxDelegate(this, [](QSharedPointer<TrackData> trackData) {
        return !trackData->isAux();
    }, true);
    ui->tv_tracks->setItemDelegateForColumn(TrackFolderModel::QUEUE_GROUP_COL, queueGroupDelegate);

    auto pixmapDelegate = new PixmapDelegate(this);
    ui->tv_tracks->setItemDelegateForColumn(TrackFolderModel::WAVEFORM_COL, pixmapDelegate);

    auto midiDelegate = new MidiMessageDelegate(m_midiInManager, this);
    ui->tv_tracks->setItemDelegateForColumn(TrackFolderModel::MIDI_COL, midiDelegate);

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
    Settings::write("lastOutputDevice", m_playbackManager->currentDevice(), "Playback");
    Settings::write("lastMainOut", QVariant::fromValue(m_playbackManager->getMainOuts()), "Playback");
    Settings::write("lastAuxOut", QVariant::fromValue(m_playbackManager->getAuxOuts()), "Playback");
    Settings::write("lastMidiPort", m_midiInManager->currentPort(), "MIDI");

    if (!m_trackFolder.isNull()) {
        m_trackFolder->writeToCache();
    }

    m_playbackManager->close();
    delete m_playbackManager;

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

    int pbCount = 0;
    for (auto trackData : m_trackFolder->tracks()) {
        if (trackData.isNull() || trackData->isAux()) {
            continue;
        }
        auto pb = new QPushButton(trackData->fileName());
        pb->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
        ui->triggerGridLayout->addWidget(pb, pbCount/3, pbCount%3);
        m_trackQueuePbs.insert(trackData, pb);
        pbCount++;

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

    disconnect(ui->cb_mainOut, SIGNAL(currentIndexChanged(int)), this, SLOT(selectMainOuts(int)));
    disconnect(ui->cb_auxOut, SIGNAL(currentIndexChanged(int)), this, SLOT(selectAuxOuts(int)));

    ui->cb_mainOut->clear();
    ui->cb_auxOut->clear();

    ui->cb_mainOut->addItem("< No Main >", QVariant::fromValue(ChannelPair(-1, -1)));
    ui->cb_auxOut->addItem("< No Aux >", QVariant::fromValue(ChannelPair(-1, -1)));

    // Add stereo out options
    for (int i = 1; i < m_playbackManager->outChannels(); i += 2) {
        ui->cb_mainOut->addItem(QString("%1 - %2").arg(i).arg(i+1), QVariant::fromValue(ChannelPair(i-1, i)));
        ui->cb_auxOut->addItem(QString("%1 - %2").arg(i).arg(i+1), QVariant::fromValue(ChannelPair(i-1, i)));
    }

    // Add mono out options
    for (int i = 0; i < m_playbackManager->outChannels(); i++) {
        ui->cb_mainOut->addItem(QString("%1 (Mono)").arg(i+1), QVariant::fromValue(ChannelPair(i, i)));
        ui->cb_auxOut->addItem(QString("%1 (Mono)").arg(i+1), QVariant::fromValue(ChannelPair(i, i)));
    }

    int defaultMain = ui->cb_mainOut->findData(QVariant::fromValue(m_playbackManager->getMainOuts()));
    QVariant lastMain = Settings::read("lastMainOut", "Playback");
    if (!lastMain.isNull()) {
        int idx = ui->cb_mainOut->findData(lastMain);
        if (idx >= 0) {
            defaultMain = idx;
        }
    }
    if (defaultMain >= 0) {
        ui->cb_mainOut->setCurrentIndex(defaultMain);
        selectMainOuts(defaultMain);
    }

    int defaultAux = ui->cb_auxOut->findData(QVariant::fromValue(m_playbackManager->getAuxOuts()));
    QVariant lastAux = Settings::read("lastAuxOut", "Playback");
    if (!lastAux.isNull()) {
        int idx = ui->cb_auxOut->findData(lastAux);
        if (idx >= 0) {
            defaultAux = idx;
        }
    }
    if (defaultAux >= 0) {
        ui->cb_auxOut->setCurrentIndex(defaultAux);
        selectAuxOuts(defaultAux);
    }

    connect(ui->cb_mainOut, SIGNAL(currentIndexChanged(int)), this, SLOT(selectMainOuts(int)));
    connect(ui->cb_auxOut, SIGNAL(currentIndexChanged(int)), this, SLOT(selectAuxOuts(int)));
}

void MainWindow::selectMainOuts(int idx)
{
    if (idx < 0 || idx >= ui->cb_mainOut->count()) {
        return;
    }

    m_playbackManager->setMainOuts(ui->cb_mainOut->itemData(idx).value<ChannelPair>());
}

void MainWindow::selectAuxOuts(int idx)
{
    if (idx < 0 || idx >= ui->cb_auxOut->count()) {
        return;
    }

    m_playbackManager->setAuxOuts(ui->cb_auxOut->itemData(idx).value<ChannelPair>());
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
    // only apply playback controls if playback tab is active
    if (ui->tabs->currentWidget() != ui->tab_playback) {
        return;
    }
    if (message.isEmpty()) {
        return;
    }
    if (message == m_controlConfig->playMidiControl()) {
        this->on_pb_togglePlay_clicked();
    }
    else if (message == m_controlConfig->clearMidiControl()) {
        if (m_playbackManager->queuedTrack().isNull() && !m_playbackManager->isRunning()) {
            this->m_playbackManager->clearActiveTrack();
        }
        else {
            queueTrack(QSharedPointer<TrackData>());
        }
    }
    else if (message == m_controlConfig->queueMidiControl()) {
        auto current = this->m_playbackManager->queuedTrack();
        if (current.isNull()) {
            current = this->m_playbackManager->activeTrack();
        }


        QList<QSharedPointer<TrackData>> cycleTracks;
        // Check queue group of active track
        if (!m_playbackManager->activeTrack().isNull()
                && !m_playbackManager->activeTrack()->queueGroup().isEmpty()) {
            for (auto trackName : m_playbackManager->activeTrack()->queueGroup()) {
                auto track = m_trackFolder->trackData(trackName);
                if (!track.isNull()) {
                    cycleTracks.append(track);
                }
            }
        }
        else {
            for (auto track : this->m_trackFolder->tracks()) {
                if (!track->isAux()) {
                    cycleTracks.append(track);
                }
            }
        }

        if (cycleTracks.size() > 0) {
            int currIdx = cycleTracks.indexOf(current);
            if (currIdx < 0) {
                queueTrack(cycleTracks.at(0));
            }
            else {
               queueTrack(cycleTracks.at((currIdx + 1) % cycleTracks.size()));
            }
        }
    }
    else if (!m_controlConfig->volumeControl().isEmpty() && message.startsWith(m_controlConfig->volumeControl())) {
        double value = double(message.back());
        value /= 1.27;
        ui->hs_volume->setValue(int(value));
    }
    else if (!m_trackFolder.isNull()) {
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
