#pragma once

#include "iCameraType.h"

#include <opencv2/opencv.hpp>
#include <queue>
#include <QVideoFrame>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>

class QVideoWidget;
class QCamera;
class QVideoSink;
class QMediaCaptureSession;

class QtCamera : public ICameraType {
    Q_OBJECT

public:
    explicit QtCamera(QObject* parent = nullptr);
    ~QtCamera() override;

    void selectCam(int index) override;
    void setFilter(FilterType filter) override;

private:
    void processFrames() override;
    void onFrameChanged(const QVideoFrame& frame);
    cv::Mat getFilteredFrame(const QVideoFrame &frame);
    void stopCamera();

    QVideoWidget* widget;
    QCamera* camera;
    QVideoSink* sink;
    QMediaCaptureSession* mediaCaptureSession;

    std::queue<QVideoFrame> frameQueue;
    std::mutex queueMutex;
    std::condition_variable frameAvailable;
    std::atomic<bool> stopProcessing;
    std::thread processingThread;
};
