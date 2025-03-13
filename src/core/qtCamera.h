#pragma once

#include <QVideoFrame>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <opencv2/opencv.hpp>
#include "iCameraType.h"

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
    void start() override;

private:
    void onFrameChanged(const QVideoFrame& frame);
    void processFrames() override;
    cv::Mat getFilteredFrame(const QVideoFrame &frame);

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
