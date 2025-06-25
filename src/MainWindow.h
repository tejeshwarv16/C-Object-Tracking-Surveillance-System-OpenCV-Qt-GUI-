#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QPixmap>
#include <QString>
#include <QCheckBox>

class VideoProcessor; // Forward declaration

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void updateVideoDisplay(QPixmap pixmap);
    void updateStatus(QString status);
    // --- Slot to receive last recorded file path ---
    void setLastRecordedFile(QString filePath);

private slots:
    void onLiveStreamClicked();
    void onOpenFileClicked();
    void onStopClicked();
    void onShowRestrictedZoneToggled(bool checked);
    void onShowTrajectoryToggled(bool checked);
    void onCheckSpeedAlertToggled(bool checked);
    // --- Slot for new button ---
    void onOpenRecordingClicked();


private:
    // UI Elements
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    QHBoxLayout *buttonLayout;
    QHBoxLayout *optionsLayout;
    QLabel *videoDisplayLabel;
    QPushButton *liveStreamButton;
    QPushButton *openFileButton;
    QPushButton *stopButton;
    QLabel *statusLabel;
    QCheckBox *showRestrictedZoneCheckbox;
    QCheckBox *showTrajectoryCheckbox;
    QCheckBox *checkSpeedAlertCheckbox;
    // --- New Button ---
    QPushButton *openRecordingButton;

    // Worker Thread and Object
    QThread* workerThread;
    VideoProcessor* videoProcessorWorker;

    // --- Store last recorded file path ---
    QString lastRecordedFilePath;

    void setupUi();
};

#endif // MAINWINDOW_H