#include "VideoProcessor.h"
#include <QDebug>
#include <QImage>
#include <QDir>
#include <QFileInfo> // For getting filename
#include <QDateTime> // For timestamp in filename

// Constructor
VideoProcessor::VideoProcessor(QObject *parent) : QObject(parent)
{
    _isRunning = false;
    _modelLoaded = loadNetwork(); // Try loading network on creation

    timer = new QTimer(this);
    // Connect timer to the processing slot
    connect(timer, &QTimer::timeout, this, &VideoProcessor::processFrame);
}

// Destructor
VideoProcessor::~VideoProcessor()
{
    stopProcessing(); // Ensure resources are released
    qDebug() << "VideoProcessor destroyed.";
}

// --- Slots for UI Control Implementation ---
void VideoProcessor::setDrawRestrictedZone(bool enabled) {
    qDebug() << "Setting draw restricted zone to:" << enabled;
    _drawRestrictedZone = enabled;
}

void VideoProcessor::setDrawTrajectory(bool enabled) {
    qDebug() << "Setting draw trajectory to:" << enabled;
    _drawTrajectory = enabled;
}

void VideoProcessor::setCheckSpeedAlert(bool enabled) {
     qDebug() << "Setting speed alert check to:" << enabled;
    _checkSpeedAlert = enabled;
}
// --- End Slots Implementation ---


// Load Network
bool VideoProcessor::loadNetwork() {
     class_names.clear();
     // Use the names_path member variable declared in the header
     names_path = YOLO_DATA_PATH + "coco.names";
     qDebug() << "DEBUG: Using class names path:" << QString::fromStdString(names_path);
     std::ifstream ifs(names_path);
     if (!ifs.is_open()) {
         emit statusUpdated("Error: Could not load class names file: " + QString::fromStdString(names_path));
         return false;
     }
     std::string line;
     while (std::getline(ifs, line)) { class_names.push_back(line); }
     ifs.close(); // Close the file
     qDebug() << "DEBUG: Loaded" << class_names.size() << "class names.";
     if (class_names.empty()) {
         emit statusUpdated("Error: Class names file is empty: " + QString::fromStdString(names_path));
         return false;
     }


     std::string model_weights = YOLO_DATA_PATH + "yolov4-tiny.weights";
     std::string model_config = YOLO_DATA_PATH + "yolov4-tiny.cfg";
     qDebug() << "DEBUG: Loading network from: " << QString::fromStdString(model_config) << "and" << QString::fromStdString(model_weights);
     try {
         net = cv::dnn::readNetFromDarknet(model_config, model_weights);
         if (net.empty()) {
             emit statusUpdated("Error: Can't load network using provided files.");
             return false;
         }
         net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
         net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
         output_layer_names = net.getUnconnectedOutLayersNames();
         qDebug() << "DEBUG: Network loaded successfully.";
         return true; // Success
     } catch (const cv::Exception& ex) {
         emit statusUpdated("Error: OpenCV exception loading network: " + QString(ex.what()));
         return false;
     }
}

// Start Processing from Camera
void VideoProcessor::startProcessing(int deviceIndex) {
    if (_isRunning) { emit statusUpdated("Status: Processing already running."); return; }
    if (!_modelLoaded) { emit statusUpdated("Error: Network model not loaded."); return; }
    qDebug() << "Attempting to start processing from camera index:" << deviceIndex;

    if (!cap.open(deviceIndex)) { emit statusUpdated("Error: Could not open camera " + QString::number(deviceIndex) + "."); return; }

    // Use member variables declared in VideoProcessor.h
    frame_width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    frame_height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    output_fps = cap.get(cv::CAP_PROP_FPS); // Use member variable
    if (output_fps <= 0 || output_fps > 100) output_fps = 30; // Sanity check FPS
    frame_size = cv::Size(frame_width, frame_height);
    qDebug() << "DEBUG: Frame Size:" << frame_width << "x" << frame_height << ", Output FPS:" << output_fps;

    _currentOutputFilePath = QString::fromStdString(OUTPUT_FILENAME_BASE) + QDateTime::currentDateTime().toString("_yyyyMMdd_hhmmss") + ".avi"; // Store path
    qDebug() << "DEBUG: Attempting to open VideoWriter:" << _currentOutputFilePath;
    video_writer.open(_currentOutputFilePath.toStdString(), OUTPUT_FOURCC, output_fps, frame_size, true);
    QString recordingStatus = video_writer.isOpened() ? "Recording to " + _currentOutputFilePath : "Warning: Recording disabled.";
    emit statusUpdated("Status: Processing Live Stream (Cam " + QString::number(deviceIndex) + "). " + recordingStatus); // Updated Status

    active_tracks.clear(); lost_tracks.clear(); next_track_id = 0; frame_count = 0; _isRunning = true;
    timer->start(1); // Start timer - process frames as fast as possible
}

// Start Processing from File
void VideoProcessor::startProcessing(QString filePath) {
     if (_isRunning) { emit statusUpdated("Status: Processing already running."); return; }
     if (!_modelLoaded) { emit statusUpdated("Error: Network model not loaded."); return; }
     qDebug() << "Attempting to start processing from file:" << filePath;
     if (!cap.open(filePath.toStdString())) { emit statusUpdated("Error: Could not open video file: " + filePath); return; }

     // Use member variables declared in VideoProcessor.h
     frame_width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
     frame_height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
     output_fps = cap.get(cv::CAP_PROP_FPS); // Use member variable
     if (output_fps <= 0 || output_fps > 100) output_fps = 30;
     frame_size = cv::Size(frame_width, frame_height);
     qDebug() << "DEBUG: Frame Size:" << frame_width << "x" << frame_height << ", Output FPS:" << output_fps;

     _currentOutputFilePath = QString::fromStdString(OUTPUT_FILENAME_BASE) + QDateTime::currentDateTime().toString("_yyyyMMdd_hhmmss") + ".avi"; // Store path
     qDebug() << "DEBUG: Attempting to open VideoWriter:" << _currentOutputFilePath;
     video_writer.open(_currentOutputFilePath.toStdString(), OUTPUT_FOURCC, output_fps, frame_size, true);
     QString recordingStatus = video_writer.isOpened() ? "Recording to " + _currentOutputFilePath : "Warning: Recording disabled.";
     emit statusUpdated("Status: Processing file: " + QFileInfo(filePath).fileName() + ". " + recordingStatus); // Updated Status

     active_tracks.clear(); lost_tracks.clear(); next_track_id = 0; frame_count = 0; _isRunning = true;
     timer->start(1); // Start timer
}


// Stop Processing
void VideoProcessor::stopProcessing() {
    qDebug() << "Stopping processing...";
    QString finishedFilePath = ""; // Store path before releasing writer
    if (video_writer.isOpened()) {
        finishedFilePath = _currentOutputFilePath;
        video_writer.release();
        qDebug() << "Video writer released.";
    }
     _isRunning = false; // Set flag before stopping timer
    if (timer->isActive()) {
        timer->stop();
    }
    if (cap.isOpened()) {
        cap.release();
        qDebug() << "Video capture released.";
    }
    active_tracks.clear(); lost_tracks.clear();
    emit statusUpdated("Status: Idle / Stopped");
    emit frameProcessed(QPixmap()); // Emit empty pixmap to clear display
    if (!finishedFilePath.isEmpty()) {
         emit recordingFinished(finishedFilePath); // Emit signal with file path
    }
    _currentOutputFilePath = ""; // Clear stored path
}

// Main Processing Loop (Called by Timer)
void VideoProcessor::processFrame() {
    if (!_isRunning || !cap.isOpened()) {
        if (_isRunning) stopProcessing(); // Ensure stopped if called unexpectedly
        return;
    }

    long long loop_start_tick = cv::getTickCount();
    cv::Mat frame; // Local frame variable for this processing step
    bool success = false;

    // --- Add try-catch around cap.read ---
    try {
        success = cap.read(frame);
    } catch (const cv::Exception& ex) {
        qDebug() << "OpenCV Exception during cap.read(): " << ex.what();
        emit statusUpdated("Error: Failed to read frame from source.");
        stopProcessing();
        return;
    }
    // --- End try-catch ---

    if (!success || frame.empty()) {
        emit statusUpdated("Status: End of video file or camera error.");
        stopProcessing();
        return;
    }

    // --- Reset, Update Trackers, Manage Lost Tracks ---
    for (auto& pair : active_tracks) { pair.second.updated_this_frame = false; }
    long long tracker_update_start_tick = cv::getTickCount();
    std::vector<int> tracks_to_move_to_lost; long long current_tick = cv::getTickCount();
    for (auto& pair : active_tracks) {
        int id = pair.first; TrackedObject& tobj = pair.second; cv::Rect prev_bbox = tobj.boundingBox;
        bool track_success = false;
        if (tobj.tracker) {
            // --- Add try-catch around tracker->update ---
            try {
                 track_success = tobj.tracker->update(frame, tobj.boundingBox);
            } catch (const cv::Exception& ex) {
                 qDebug() << "OpenCV Exception during tracker->update() for ID " << id << ":" << ex.what();
                 track_success = false; // Treat exception as tracking failure
            }
            // --- End try-catch ---
        }
        // (Velocity/Trajectory calculation logic remains the same)
        if (track_success) { tobj.updated_this_frame = true; tobj.frames_since_seen = 0; cv::Point current_center = getCenter(tobj.boundingBox); tobj.trajectory.push_back(current_center); if (tobj.trajectory.size() > TRAJECTORY_LENGTH) { tobj.trajectory.pop_front(); } if (tobj.trajectory.size() >= 2 && tobj.last_update_tick > 0) { double time_diff_sec = (double)(current_tick - tobj.last_update_tick) / cv::getTickFrequency(); if (time_diff_sec > 1e-3) { cv::Point prev_center = getCenter(prev_bbox); tobj.velocity = cv::norm(current_center - prev_center) / time_diff_sec; } else { tobj.velocity = 0; } } else { tobj.velocity = 0; } tobj.last_update_tick = current_tick; } else { tracks_to_move_to_lost.push_back(id); }
    }
    tracker_update_time_ms = ((double)(cv::getTickCount() - tracker_update_start_tick) / cv::getTickFrequency()) * 1000;
    // (Manage lost tracks logic remains the same)
    for (int id : tracks_to_move_to_lost) { if (active_tracks.count(id)) { TrackedObject lost_obj = active_tracks[id]; lost_obj.frames_since_seen = 1; lost_tracks[id] = lost_obj; active_tracks.erase(id); std::cout << "DEBUG: Moved Track ID " << id << " to lost tracks." << std::endl; } } std::vector<int> tracks_to_permanently_delete; for (auto& pair : lost_tracks) { pair.second.frames_since_seen++; if (pair.second.frames_since_seen > MAX_LOST_FRAMES) { tracks_to_permanently_delete.push_back(pair.first); } } for (int id : tracks_to_permanently_delete) { lost_tracks.erase(id); std::cout << "DEBUG: Permanently deleted Lost Track ID " << id << std::endl;}


    // --- Detection & Association ---
    std::vector<cv::Rect> detected_boxes_current_frame; std::vector<int> detected_classIds_current_frame; std::vector<float> detected_confidences_current_frame;
    long long detection_start_tick = 0;
    if (frame_count % DETECT_INTERVAL == 0 || active_tracks.empty()) {
        detection_start_tick = cv::getTickCount();
        try { // Add try-catch around DNN operations
             // Use member variable blob
             cv::dnn::blobFromImage(frame, blob, 1./255., cv::Size(INPUT_WIDTH, INPUT_HEIGHT), cv::Scalar(), true, false);
             net.setInput(blob); std::vector<cv::Mat> outs; net.forward(outs, output_layer_names);
             detection_time_ms = ((double)(cv::getTickCount() - detection_start_tick) / cv::getTickFrequency()) * 1000;
             processYoloOutput(outs, frame.size(), detected_boxes_current_frame, detected_classIds_current_frame, detected_confidences_current_frame);
         } catch (const cv::Exception& ex) {
             qDebug() << "OpenCV Exception during detection/DNN processing: " << ex.what();
             emit statusUpdated("Error: Detection failed.");
             detection_time_ms = 0; detected_boxes_current_frame.clear();
             detected_classIds_current_frame.clear(); detected_confidences_current_frame.clear();
         }
         // Associate tracks (pass frame needed for tracker init)
         associateAndTrack(frame, detected_boxes_current_frame, detected_classIds_current_frame);
         std::cout << "DEBUG: YOLO Detection took: " << detection_time_ms << " ms. Relevant Detections: " << detected_boxes_current_frame.size() << std::endl;
    }


    // --- 4. Draw Results & Check Alerts ---
    long long drawing_start_tick = cv::getTickCount();
    bool alert_active_this_frame = false;
    cv::Rect restricted_zone(0, 0, frame.cols / 2, frame.rows / 2); // Define zone based on current frame size

    for (auto const& [id, tobj] : active_tracks) {
         if (tobj.updated_this_frame) {
             cv::Scalar box_color = cv::Scalar(0, 255, 0); // Default Green
             std::string alert_text = ""; // Text to add near label

             // Check Restricted Zone Alert (Conditional)
             if (_drawRestrictedZone) {
                  cv::Rect intersection = tobj.boundingBox & restricted_zone;
                  if (intersection.area() > 0) {
                      alert_active_this_frame = true; box_color = cv::Scalar(0, 0, 255); // Red
                      if (frame_count % 10 == 0) { std::cout << "ALERT: ID " << id << " (" << tobj.className << ") in restricted zone!" << std::endl; }
                      alert_text += "[ZONE]";
                  }
             }

             // Check Speed Alert (Conditional)
             if (_checkSpeedAlert && tobj.velocity > SPEED_THRESHOLD_PIXELS_PER_SEC) {
                 alert_active_this_frame = true;
                 if (box_color == cv::Scalar(0, 255, 0)) { box_color = cv::Scalar(0, 165, 255); } // Orange if not already red
                 if (frame_count % 10 == 0) { std::cout << "** SPEED ALERT: ID " << id << " (" << tobj.className << ") V=" << tobj.velocity << " px/s **" << std::endl; }
                 alert_text += "[SPEED]";
             }

             // Draw box, label, velocity
             cv::rectangle(frame, tobj.boundingBox, box_color, 2);
             std::string label = tobj.className + " ID:" + std::to_string(id) + " " + alert_text;
             std::string vel_label = cv::format("V:%.1f px/s", tobj.velocity);
             cv::Point label_origin = cv::Point(tobj.boundingBox.x, tobj.boundingBox.y - 5);
             cv::Point vel_origin = cv::Point(tobj.boundingBox.x, tobj.boundingBox.y + tobj.boundingBox.height + 15);
             cv::putText(frame, label, label_origin, cv::FONT_HERSHEY_SIMPLEX, 0.5, box_color, 1);
             cv::putText(frame, vel_label, vel_origin, cv::FONT_HERSHEY_SIMPLEX, 0.4, box_color, 1);

             // Draw Trajectory (Conditional & Uncommented)
             if (_drawTrajectory && tobj.trajectory.size() > 1) {
                  for(size_t i = 1; i < tobj.trajectory.size(); ++i) {
                      cv::line(frame, tobj.trajectory[i-1], tobj.trajectory[i], cv::Scalar(0, 255, 255), 2); // Yellow trail
                  }
             }
         }
    }

    // Draw Restricted Zone (Conditional)
    if (_drawRestrictedZone) {
        cv::rectangle(frame, restricted_zone, cv::Scalar(0, 0, 255), 2);
        cv::putText(frame, "Restricted", cv::Point(restricted_zone.x + 5, restricted_zone.y + 15), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 1);
    }

    // Draw Timings
    drawing_time_ms = ((double)(cv::getTickCount() - drawing_start_tick) / cv::getTickFrequency()) * 1000;
    std::string time_label = cv::format("Detect: %.1f ms", detection_time_ms); cv::putText(frame, time_label, cv::Point(10, 20), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255), 1);
    std::string tracker_time_label = cv::format("TrackUpd: %.1f ms", tracker_update_time_ms); cv::putText(frame, tracker_time_label, cv::Point(10, 40), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255), 1);
    std::string draw_time_label = cv::format("Draw: %.1f ms", drawing_time_ms); cv::putText(frame, draw_time_label, cv::Point(10, 60), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255), 1);
    long long frame_end_tick = cv::getTickCount(); double frame_processing_time_sec = (double)(frame_end_tick - loop_start_tick) / cv::getTickFrequency();
    if (frame_processing_time_sec > 1e-6) { current_fps = 1.0 / frame_processing_time_sec; }
    std::string fps_label = cv::format("FPS: %.1f", current_fps); cv::putText(frame, fps_label, cv::Point(frame.cols - 100, 20), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255), 2);
    if (alert_active_this_frame && (_drawRestrictedZone || _checkSpeedAlert)) { cv::Point alert_origin(frame.cols / 2 - 60, frame.rows - 20); cv::putText(frame, "ALERT!", alert_origin, cv::FONT_HERSHEY_TRIPLEX, 1.0, cv::Scalar(0, 0, 255), 2); }


    // --- 5. Convert and Emit Frame ---
    QPixmap pixmap = matToPixmap(frame);
    if (!pixmap.isNull()) { emit frameProcessed(pixmap); }

    // --- 6. Write Frame to Video ---
    if (video_writer.isOpened()) {
        // --- Add try-catch around writer.write ---
        try {
             video_writer.write(frame);
        } catch (const cv::Exception& ex) {
             qDebug() << "OpenCV Exception during video_writer.write(): " << ex.what();
             emit statusUpdated("Warning: Error writing video frame.");
             // Consider stopping recording? For now, just report warning.
        }
        // --- End try-catch ---
    }

    frame_count++;
}


// --- Helper Function Implementations ---

void VideoProcessor::processYoloOutput(const std::vector<cv::Mat>& outs, cv::Size imgSize,
                                      std::vector<cv::Rect>& boxes, std::vector<int>& classIds, std::vector<float>& confidences)
{
    // Temporary storage before NMS
    std::vector<cv::Rect> raw_boxes;
    std::vector<int> raw_classIds;
    std::vector<float> raw_confidences;

    boxes.clear(); classIds.clear(); confidences.clear(); // Clear output vectors

     for (const cv::Mat& output : outs) {
         const float* data = (float*)output.data;
         for (int i = 0; i < output.rows; ++i, data += output.cols) {
             cv::Mat scores = output.row(i).colRange(5, output.cols);
             cv::Point classIdPoint; double confidence;
             cv::minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
             if (confidence > CONFIDENCE_THRESHOLD) {
                 int centerX = (int)(data[0] * imgSize.width); int centerY = (int)(data[1] * imgSize.height);
                 int width = (int)(data[2] * imgSize.width); int height = (int)(data[3] * imgSize.height);
                 int left = centerX - width / 2; int top = centerY - height / 2;
                 // Store corresponding classId and confidence along with the box
                 raw_classIds.push_back(classIdPoint.x);
                 raw_confidences.push_back((float)confidence);
                 raw_boxes.push_back(cv::Rect(left, top, width, height));
             }
         }
     }
     std::vector<int> indices;
     // Run NMS on raw boxes and confidences
     cv::dnn::NMSBoxes(raw_boxes, raw_confidences, CONFIDENCE_THRESHOLD, NMS_THRESHOLD, indices);

     // Filter based on indices from NMS and desired classes
     for (int idx : indices) {
          int classId = raw_classIds[idx]; // Use index from NMS on original unfiltered vectors
          if (classId < class_names.size()) {
               std::string className = class_names[classId];
               if (desired_classes.count(className)) {
                    // Add the filtered box and its corresponding classId/confidence
                    boxes.push_back(raw_boxes[idx]);
                    classIds.push_back(classId);
                    confidences.push_back(raw_confidences[idx]);
               }
          }
     }
}


void VideoProcessor::associateAndTrack(cv::Mat& frame, // Pass frame for tracker init
                                      const std::vector<cv::Rect>& detected_boxes,
                                      const std::vector<int>& detected_classIds)
{
    // --- NOTE: This function now assumes detected_boxes and detected_classIds are the FINAL lists after NMS and class filtering ---
    std::vector<bool> detection_matched(detected_boxes.size(), false);
    std::vector<int> reactivated_lost_track_ids;

    // Match detections to ACTIVE tracks
    for (auto& pair : active_tracks) { if (!pair.second.updated_this_frame) continue; int best_match_idx = -1; double best_iou = MIN_IOU_THRESHOLD; for (size_t i = 0; i < detected_boxes.size(); ++i) { if (detection_matched[i]) continue; double iou = calculateIoU(pair.second.boundingBox, detected_boxes[i]); if (iou > best_iou) { best_iou = iou; best_match_idx = i; } } if (best_match_idx != -1) { detection_matched[best_match_idx] = true; } }

    // Match remaining detections to LOST tracks (Re-ID)
    for (size_t i = 0; i < detected_boxes.size(); ++i) { if (detection_matched[i]) continue; int best_lost_match_id = -1; double best_lost_iou = REID_IOU_THRESHOLD; for (auto const& [lost_id, lost_tobj] : lost_tracks) { double iou = calculateIoU(lost_tobj.boundingBox, detected_boxes[i]); if (iou > best_lost_iou) { best_lost_iou = iou; best_lost_match_id = lost_id; } }
         if (best_lost_match_id != -1) { TrackedObject reactivated_track = lost_tracks[best_lost_match_id]; reactivated_track.boundingBox = detected_boxes[i];
             // Use fully qualified name for nested class
             cv::Ptr<cv::legacy::Tracker> legacy_tracker = cv::legacy::TrackerMOSSE::create();
             if(legacy_tracker) { reactivated_track.tracker = cv::makePtr<VideoProcessor::LegacyTrackerWrapper>(legacy_tracker); try { reactivated_track.tracker->init(frame, reactivated_track.boundingBox); reactivated_track.updated_this_frame = true; reactivated_track.frames_since_seen = 0; reactivated_track.trajectory.clear(); reactivated_track.trajectory.push_back(getCenter(reactivated_track.boundingBox)); reactivated_track.last_update_tick = cv::getTickCount(); reactivated_track.velocity = 0; active_tracks[best_lost_match_id] = reactivated_track; reactivated_lost_track_ids.push_back(best_lost_match_id); detection_matched[i] = true; qDebug() << "DEBUG: Re-identified detection" << i << "as Track ID" << best_lost_match_id; }
                   catch (const cv::Exception& ex) { qDebug() << "WARN: Exception during legacy tracker re-init for ID" << best_lost_match_id << ":" << ex.what(); reactivated_track.tracker.release(); }
             } else { qDebug() << "WARN: Failed to create MOSSE tracker instance for Re-ID" << best_lost_match_id; } } }
    for (int id : reactivated_lost_track_ids) { lost_tracks.erase(id); }

    // Create NEW tracks for remaining unmatched detections
    for (size_t i = 0; i < detected_boxes.size(); ++i) { if (!detection_matched[i]) { TrackedObject new_object; new_object.id = next_track_id++; new_object.boundingBox = detected_boxes[i]; new_object.className = class_names[detected_classIds[i]];
          // Use fully qualified name for nested class
          cv::Ptr<cv::legacy::Tracker> legacy_tracker = cv::legacy::TrackerMOSSE::create();
          if (legacy_tracker) { new_object.tracker = cv::makePtr<VideoProcessor::LegacyTrackerWrapper>(legacy_tracker); try { new_object.tracker->init(frame, new_object.boundingBox); new_object.updated_this_frame = true; new_object.trajectory.push_back(getCenter(new_object.boundingBox)); new_object.last_update_tick = cv::getTickCount(); active_tracks[new_object.id] = new_object; qDebug() << "DEBUG: Initialized new Track ID" << new_object.id << "(" << QString::fromStdString(new_object.className) << ")"; }
               catch (const cv::Exception& ex) { qDebug() << "WARN: Exception during legacy tracker init for new track:" << ex.what(); next_track_id--; }
          } else { qDebug() << "WARN: Failed to create MOSSE tracker instance for new detection."; next_track_id--; } } }
}


QPixmap VideoProcessor::matToPixmap(const cv::Mat& mat) {
     if (mat.empty()) { return QPixmap(); }
     try {
          if (mat.channels() == 3) { // Color BGR
              cv::Mat rgbFrame;
              cv::cvtColor(mat, rgbFrame, cv::COLOR_BGR2RGB);
              // Use copy to ensure data lifetime if rgbFrame goes out of scope
              return QPixmap::fromImage(QImage(rgbFrame.data, rgbFrame.cols, rgbFrame.rows, static_cast<int>(rgbFrame.step), QImage::Format_RGB888).copy());
          } else if (mat.channels() == 1) { // Grayscale
              return QPixmap::fromImage(QImage(mat.data, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_Grayscale8).copy());
          } else {
              qDebug() << "Unsupported cv::Mat channel count for QPixmap conversion:" << mat.channels();
              return QPixmap();
          }
     } catch (const cv::Exception& ex) {
         qDebug() << "Error converting Mat to QPixmap:" << ex.what();
         return QPixmap();
     }
 }

cv::Point VideoProcessor::getCenter(const cv::Rect& rect) { return cv::Point(rect.x + rect.width / 2, rect.y + rect.height / 2); }
double VideoProcessor::calculateIoU(const cv::Rect& box1, const cv::Rect& box2) { cv::Rect intersection = box1 & box2; double intersectionArea = intersection.area(); if (intersectionArea <= 0) return 0.0; double unionArea = box1.area() + box2.area() - intersectionArea; if (unionArea <= 0) return 0.0; return intersectionArea / unionArea; }
