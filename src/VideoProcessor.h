#ifndef VIDEOPROCESSOR_H
#define VIDEOPROCESSOR_H

#include <QObject>
#include <QPixmap>
#include <QImage>
#include <QString>
#include <QTimer>
#include <QDateTime> // For unique filenames

// Include OpenCV headers needed for processing
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/tracking/tracking_legacy.hpp> // For MOSSE wrapper

#include <vector>
#include <string> // Needed for std::string
#include <map>
#include <fstream>
#include <set>
#include <deque>
#include <iostream> // For cerr/cout

// Forward declare wrapper class defined in the CPP file
class LegacyTrackerWrapper;

// Define TrackedObject struct (same as before)
struct TrackedObject {
    cv::Ptr<cv::Tracker> tracker;
    cv::Rect boundingBox;
    int id;
    std::string className;
    bool updated_this_frame = false;
    std::deque<cv::Point> trajectory;
    double velocity = 0.0;
    long long last_update_tick = 0;
    int frames_since_seen = 0;
};


class VideoProcessor : public QObject
{
    Q_OBJECT // Necessary for signals and slots

public:
    explicit VideoProcessor(QObject *parent = nullptr);
    ~VideoProcessor();

signals: // Signals emitted by this worker
    void frameProcessed(QPixmap pixmap); // Emits the frame to display
    void statusUpdated(QString status);  // Emits status messages
    void recordingFinished(QString filePath); // Signal for review button

public slots: // Slots called by MainWindow (or timer)
    void startProcessing(int deviceIndex);  // Start from camera
    void startProcessing(QString filePath); // Start from file
    void stopProcessing();
    void processFrame(); // Slot called by timer to process one frame
    // --- Slots for UI Controls ---
    void setDrawRestrictedZone(bool enabled);
    void setDrawTrajectory(bool enabled);
    void setCheckSpeedAlert(bool enabled);


private:
    // Configuration
    const float CONFIDENCE_THRESHOLD = 0.4; const float NMS_THRESHOLD = 0.4;
    const int INPUT_WIDTH = 320; const int INPUT_HEIGHT = 320; const int DETECT_INTERVAL = 30;
    const double MIN_IOU_THRESHOLD = 0.1; const double REID_IOU_THRESHOLD = 0.2;
    const int MAX_LOST_FRAMES = 60; const std::set<std::string> desired_classes = {"person", "bicycle", "car", "motorbike", "bus", "truck"};
    const int TRAJECTORY_LENGTH = 20; const std::string OUTPUT_FILENAME_BASE = "../output_video";
    const int OUTPUT_FOURCC = cv::VideoWriter::fourcc('M','J','P','G');
    const std::string YOLO_DATA_PATH = "../data/";
    const double SPEED_THRESHOLD_PIXELS_PER_SEC = 150.0;

    // OpenCV Objects
    cv::VideoCapture cap;
    cv::dnn::Net net;
    std::vector<std::string> class_names;
    std::vector<cv::String> output_layer_names;
    cv::VideoWriter video_writer;
    cv::Size frame_size;
    int frame_width = 0;
    int frame_height = 0;
    double output_fps = 30.0;
    cv::Mat blob;
    // **** ADDED MISSING MEMBER DECLARATION ****
    std::string names_path; // Path for coco.names file
    // **** END ADDED DECLARATION ****

    // Tracking State
    std::map<int, TrackedObject> active_tracks;
    std::map<int, TrackedObject> lost_tracks;
    int next_track_id = 0;
    int frame_count = 0;
    double detection_time_ms = 0.0;
    double tracker_update_time_ms = 0.0;
    double drawing_time_ms = 0.0;
    double current_fps = 0.0;

    // State Flags
    bool _isRunning = false;
    bool _modelLoaded = false;
    bool _drawRestrictedZone = true;
    bool _drawTrajectory = false;
    bool _checkSpeedAlert = true;
    QString _currentOutputFilePath = ""; // Store current output filename


    // Timer for processing loop
    QTimer *timer;

    // Private helper functions
    bool loadNetwork();
    void processYoloOutput(const std::vector<cv::Mat>& outs, cv::Size imgSize, std::vector<cv::Rect>& boxes, std::vector<int>& classIds, std::vector<float>& confidences);
    void associateAndTrack(cv::Mat& frame, const std::vector<cv::Rect>& detected_boxes, const std::vector<int>& detected_classIds);
    cv::Point getCenter(const cv::Rect& rect);
    double calculateIoU(const cv::Rect& box1, const cv::Rect& box2);
    QPixmap matToPixmap(const cv::Mat& mat);

    // Nested Wrapper Class Definition
    class LegacyTrackerWrapper : public cv::Tracker {
      public:
        LegacyTrackerWrapper(const cv::Ptr<cv::legacy::Tracker>& lt) : legacy_tracker_(lt) { CV_Assert(lt); }
        void init(cv::InputArray i, const cv::Rect& b) CV_OVERRIDE {
             cv::Rect2d bd = b; if (!legacy_tracker_->init(i, bd)) { std::cerr << "WARN: Legacy tracker init() returned false." << std::endl; } }
        bool update(cv::InputArray i, cv::Rect& b) CV_OVERRIDE {
             cv::Rect2d bd = b; bool s = legacy_tracker_->update(i, bd);
             if (s) { b = cv::Rect(cvRound(bd.x), cvRound(bd.y), cvRound(bd.width), cvRound(bd.height)); } return s; }
      private:
        cv::Ptr<cv::legacy::Tracker> legacy_tracker_;
    };

};

#endif // VIDEOPROCESSOR_H