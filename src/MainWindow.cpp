#include "MainWindow.h"
#include "VideoProcessor.h"
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QMetaObject>
#include <QCheckBox>
#include <QDesktopServices> // For opening files/URLs
#include <QUrl>            // For converting file path

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setupUi(); // Create UI elements

    // --- Threading Setup ---
    workerThread = new QThread(this);
    videoProcessorWorker = new VideoProcessor();
    videoProcessorWorker->moveToThread(workerThread);

    // --- Connect Signals and Slots ---
    // Worker -> MainWindow UI Updates
    connect(videoProcessorWorker, &VideoProcessor::frameProcessed, this, &MainWindow::updateVideoDisplay, Qt::QueuedConnection);
    connect(videoProcessorWorker, &VideoProcessor::statusUpdated, this, &MainWindow::updateStatus, Qt::QueuedConnection);
    // --- Connect worker signal providing last saved file path ---
    connect(videoProcessorWorker, &VideoProcessor::recordingFinished, this, &MainWindow::setLastRecordedFile, Qt::QueuedConnection);


    // Thread Cleanup
    connect(workerThread, &QThread::finished, videoProcessorWorker, &QObject::deleteLater);
    connect(workerThread, &QThread::finished, workerThread, &QObject::deleteLater);

    // --- Connect Checkbox Signals to MainWindow Slots ---
    connect(showRestrictedZoneCheckbox, &QCheckBox::toggled, this, &MainWindow::onShowRestrictedZoneToggled);
    connect(showTrajectoryCheckbox, &QCheckBox::toggled, this, &MainWindow::onShowTrajectoryToggled);
    connect(checkSpeedAlertCheckbox, &QCheckBox::toggled, this, &MainWindow::onCheckSpeedAlertToggled);


    // Start the thread
    workerThread->start();

    // --- Initial State Setup for Worker ---
    QMetaObject::invokeMethod(videoProcessorWorker, "setDrawRestrictedZone", Qt::QueuedConnection, Q_ARG(bool, showRestrictedZoneCheckbox->isChecked()));
    QMetaObject::invokeMethod(videoProcessorWorker, "setDrawTrajectory", Qt::QueuedConnection, Q_ARG(bool, showTrajectoryCheckbox->isChecked()));
    QMetaObject::invokeMethod(videoProcessorWorker, "setCheckSpeedAlert", Qt::QueuedConnection, Q_ARG(bool, checkSpeedAlertCheckbox->isChecked()));


    qDebug() << "MainWindow created, worker thread started.";
}

MainWindow::~MainWindow()
{
    qDebug() << "MainWindow destructor called.";
    if (workerThread && workerThread->isRunning()) {
         QMetaObject::invokeMethod(videoProcessorWorker, "stopProcessing", Qt::QueuedConnection);
         workerThread->quit();
        if (!workerThread->wait(1000)) {
             qDebug() << "Warning: Worker thread did not finish gracefully, terminating.";
             workerThread->terminate(); workerThread->wait(); }
    }
    qDebug() << "MainWindow destroyed.";
}

// Helper to create UI
void MainWindow::setupUi() {
    setWindowTitle("Object Tracking System");
    resize(800, 680); // Slightly taller for new button row

    centralWidget = new QWidget(this);
    mainLayout = new QVBoxLayout(centralWidget);

    videoDisplayLabel = new QLabel("Press 'Start Live Stream' or 'Open Video File'", this);
    videoDisplayLabel->setAlignment(Qt::AlignCenter);
    videoDisplayLabel->setStyleSheet("QLabel { background-color : black; color : gray; border: 1px solid gray;}");
    videoDisplayLabel->setMinimumSize(640, 480);
    mainLayout->addWidget(videoDisplayLabel, 1);

    // Button Layout
    buttonLayout = new QHBoxLayout();
    liveStreamButton = new QPushButton("Start Live Stream", this);
    openFileButton = new QPushButton("Open Video File", this);
    stopButton = new QPushButton("Stop Processing", this);
    buttonLayout->addWidget(liveStreamButton);
    buttonLayout->addWidget(openFileButton);
    buttonLayout->addWidget(stopButton);
    mainLayout->addLayout(buttonLayout);

    // Options Layout (Checkboxes)
    optionsLayout = new QHBoxLayout();
    showRestrictedZoneCheckbox = new QCheckBox("Show/Alert Zone", this);
    showTrajectoryCheckbox = new QCheckBox("Show Trajectory", this);
    checkSpeedAlertCheckbox = new QCheckBox("Check Speed Alert", this);
    showRestrictedZoneCheckbox->setChecked(true);
    showTrajectoryCheckbox->setChecked(false); // Trajectory off by default
    checkSpeedAlertCheckbox->setChecked(true);
    optionsLayout->addWidget(showRestrictedZoneCheckbox);
    optionsLayout->addWidget(showTrajectoryCheckbox);
    optionsLayout->addWidget(checkSpeedAlertCheckbox);
    optionsLayout->addStretch(1);
    mainLayout->addLayout(optionsLayout);

    // --- Add Review Button ---
    openRecordingButton = new QPushButton("Open Last Recording", this);
    mainLayout->addWidget(openRecordingButton); // Add below options


    // Status Label
    statusLabel = new QLabel("Status: Idle", this);
    mainLayout->addWidget(statusLabel);

    setCentralWidget(centralWidget);

    // Connect button clicks
    connect(liveStreamButton, &QPushButton::clicked, this, &MainWindow::onLiveStreamClicked);
    connect(openFileButton, &QPushButton::clicked, this, &MainWindow::onOpenFileClicked);
    connect(stopButton, &QPushButton::clicked, this, &MainWindow::onStopClicked);
    // Connect review button click
    connect(openRecordingButton, &QPushButton::clicked, this, &MainWindow::onOpenRecordingClicked);
}

// --- Slot Implementations ---

void MainWindow::onLiveStreamClicked() {
    qDebug() << "Live Stream button clicked!";
    lastRecordedFilePath.clear(); // Clear path when starting new stream
    openRecordingButton->setEnabled(false); // Disable button until recording finishes
    QMetaObject::invokeMethod(videoProcessorWorker, "startProcessing", Qt::QueuedConnection, Q_ARG(int, 0));
}

void MainWindow::onOpenFileClicked() {
    qDebug() << "Open File button clicked!";
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Video File"), QDir::homePath(), tr("Video Files (*.mp4 *.avi *.mov *.mkv)"));
    if (!fileName.isEmpty()) {
        qDebug() << "Selected file:" << fileName;
        lastRecordedFilePath.clear(); // Clear path when starting new stream
        openRecordingButton->setEnabled(false); // Disable button until recording finishes
        QMetaObject::invokeMethod(videoProcessorWorker, "startProcessing", Qt::QueuedConnection, Q_ARG(QString, fileName));
    } else {
         qDebug() << "No file selected."; updateStatus("Status: File open cancelled."); }
}

void MainWindow::onStopClicked() {
    qDebug() << "Stop button clicked!";
    QMetaObject::invokeMethod(videoProcessorWorker, "stopProcessing", Qt::QueuedConnection);
    // Button will be re-enabled when worker signals recording finished
}

// Slots for Checkboxes -> Call worker slots
void MainWindow::onShowRestrictedZoneToggled(bool checked) {
     qDebug() << "Restricted Zone Checkbox Toggled:" << checked;
     QMetaObject::invokeMethod(videoProcessorWorker, "setDrawRestrictedZone", Qt::QueuedConnection, Q_ARG(bool, checked));
}

void MainWindow::onShowTrajectoryToggled(bool checked) {
     qDebug() << "Trajectory Checkbox Toggled:" << checked;
     QMetaObject::invokeMethod(videoProcessorWorker, "setDrawTrajectory", Qt::QueuedConnection, Q_ARG(bool, checked));
}

void MainWindow::onCheckSpeedAlertToggled(bool checked) {
     qDebug() << "Speed Alert Checkbox Toggled:" << checked;
     QMetaObject::invokeMethod(videoProcessorWorker, "setCheckSpeedAlert", Qt::QueuedConnection, Q_ARG(bool, checked));
}

// Slot for Review Button
void MainWindow::onOpenRecordingClicked() {
    qDebug() << "Open Recording button clicked!";
    if (lastRecordedFilePath.isEmpty()) {
        QMessageBox::information(this, "No Recording", "No recording has been completed in this session.");
        return;
    }

    bool success = QDesktopServices::openUrl(QUrl::fromLocalFile(lastRecordedFilePath));
    if (!success) {
         QMessageBox::warning(this, "Error", "Could not open the video file:\n" + lastRecordedFilePath + "\nCheck if a suitable video player is installed.");
    }
}


// Slot to update video display label
void MainWindow::updateVideoDisplay(QPixmap pixmap) { /* ... same as before ... */
    if (!pixmap.isNull()) { videoDisplayLabel->setPixmap(pixmap.scaled(videoDisplayLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation)); }
    else { videoDisplayLabel->setText("Video Stopped / No Frame"); videoDisplayLabel->setStyleSheet("QLabel { background-color : black; color : gray; border: 1px solid gray;}"); } }

// Slot to update status label
void MainWindow::updateStatus(QString status) { statusLabel->setText(status); }

// Slot to receive and store the path of the finished recording
void MainWindow::setLastRecordedFile(QString filePath) {
    lastRecordedFilePath = filePath;
    openRecordingButton->setEnabled(!filePath.isEmpty()); // Enable button if path is valid
    qDebug() << "Last recorded file path set to:" << filePath;
}